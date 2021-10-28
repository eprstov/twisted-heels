#include "processor.h"

#include <assert.h>

void Processor::CreateDecoder()
{
	const auto params = istream->codecpar;
	const AVCodec* codec = avcodec_find_decoder( params->codec_id );
	assert(codec);

	decoder = avcodec_alloc_context3(codec);
	assert(decoder);

	auto res = avcodec_parameters_to_context( decoder, params );
	assert( res >= 0 );
	decoder->time_base = istream->time_base;

	decoder->framerate = istream->r_frame_rate;
	res = avcodec_open2( decoder, codec, nullptr );
	assert( res >= 0 );
}

void Processor::CreateEncoder()
{
	const auto params = ostream->codecpar;
	const AVCodec* codec = avcodec_find_encoder( params->codec_id );
	assert(codec);

	encoder = avcodec_alloc_context3(codec);
	assert(encoder);

	auto res = avcodec_parameters_to_context( encoder, params );
	assert( res >= 0 );

	encoder->sample_aspect_ratio = decoder->sample_aspect_ratio;
	encoder->pix_fmt = codec->pix_fmts ? codec->pix_fmts[0] : decoder->pix_fmt;
	encoder->time_base = decoder->time_base;
	encoder->gop_size = config.gop;
	encoder->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	encoder->framerate = decoder->framerate;

	res = avcodec_open2( encoder, codec, nullptr );
	assert( res >= 0 );
}

Processor::Processor( const AVStream* is, const AVStream* os, Config&& cfg ) : istream(is), ostream(os), packets( av_packet_alloc, av_packet_free, av_packet_unref ), scaler( istream->codecpar, ostream->codecpar ), config( std::move(cfg) )
{
	CreateDecoder();
	CreateEncoder();

	ipacket = av_packet_alloc();
	iframe = av_frame_alloc();
}

Processor::~Processor()
{
	packets.Release(results);

	av_frame_free( &iframe );
	av_packet_free( &ipacket );

	avcodec_close(decoder);
	avcodec_close(encoder);
}

void Processor::Adjust( AVCodecParameters* params ) const
{
	[[maybe_unused]] auto res = avcodec_parameters_from_context( params, encoder );
	assert( res >= 0 );
}

const std::vector< AVPacket* >& Processor::operator()( const AVPacket* ipkt )
{
	packets.Release(results);

	av_packet_ref( ipacket, ipkt );
	av_packet_rescale_ts( ipacket, istream->time_base, decoder->time_base );

	auto res = avcodec_send_packet( decoder, ipacket );
	assert( res >= 0 );

	while( res >= 0 )
	{
		res = avcodec_receive_frame( decoder, iframe );
		assert( res >= 0 || res == AVERROR_EOF || res == AVERROR(EAGAIN) );

		if( res >= 0 )
		{
			auto oframe = scaler(iframe);
			oframe->pts = iframe->best_effort_timestamp; // prevents non-monotonic timestamps
			oframe->pict_type = AV_PICTURE_TYPE_NONE;

			res = avcodec_send_frame( encoder, oframe );
			assert( res >= 0 );

			Receive();
		}
	}


	return results;
}

std::vector< AVPacket* > Processor::operator()()
{
	auto res = avcodec_send_frame( encoder, nullptr );
	assert( res >= 0 );

	packets.Release(results);
	Receive();

	return results;
}

void Processor::Receive()
{
	for( int res = 0; res >= 0; )
	{
		auto* opacket = packets.Aquire();
		res = avcodec_receive_packet( encoder, opacket );
		assert( res >= 0 || res == AVERROR(EAGAIN) || res == AVERROR_EOF );

		if( res >= 0 )
		{
			opacket->stream_index = ostream->index;
			opacket->pos = -1;
			av_packet_rescale_ts( opacket, encoder->time_base, ostream->time_base );

			results.push_back(opacket);
		}
		else
			packets.Release(opacket);
	}
}
