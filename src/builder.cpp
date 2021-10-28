#include "builder.h"

#include <assert.h>
#include <sstream>

namespace
{

std::string MakeVariantMap( std::size_t nvars )
{
	std::stringstream result;
	result << "a:0,agroup:aud";

	for( int i = 0; i < nvars; ++i )
	{
		result << " v:" << i << ",agroup:aud";
	}

	return result.str();
}

}

Builder::Builder( std::filesystem::path p ) : path( std::move(p) )
{
	avformat_alloc_output_context2( &format, nullptr, "hls", (path / "media-%v/stream.m3u8").c_str() );
	assert(format);
}

Builder::~Builder()
{
	avformat_free_context(format);
}

AVStream* Builder::Add( const AVCodecParameters* params )
{
	assert( !started );

	auto stream = avformat_new_stream( format, nullptr );
	assert(stream);

	avcodec_parameters_copy( stream->codecpar, params );

	return stream;
}

void Builder::operator()( AVPacket* packet )
{
	auto res = av_write_frame( format, packet );
	assert( res >= 0 );
}

void Builder::Start()
{
	auto nvars = std::count_if( format->streams, format->streams + format->nb_streams, [] (auto stream) { return stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO; } );

	auto var_stream_map = MakeVariantMap(nvars);
	AVDictionary* opts = nullptr;
	av_dict_set( &opts, "hls_time", "1", 0 );
	av_dict_set( &opts, "hls_playlist_type", "vod", 0 );
	av_dict_set( &opts, "hls_flags", "independent_segments", 0 );
	av_dict_set( &opts, "hls_segment_type", "mpegts", 0 );
	av_dict_set( &opts, "hls_segment_filename", (path / "media-%v/data-%02d.ts").c_str(), 0 );
	av_dict_set( &opts, "var_stream_map", var_stream_map.c_str(), 0 );
	av_dict_set( &opts, "master_pl_name", "master.m3u8", 0 );

	auto res = avformat_write_header( format, &opts );
	assert( res >= 0 );

	av_dict_free( &opts );
}

void Builder::Finish()
{
	av_write_trailer(format);
}
