#include "basic.h"
#include "input.h"

#include <assert.h>
#include <string>

namespace
{

AVStream* FindStream( AVFormatContext* format, AVMediaType type )
{
	for ( int i = 0; i < format->nb_streams; ++i )
	{
		AVStream* istream = format->streams[i];

		AVCodecParameters* codec = istream->codecpar;
		if( codec->codec_type == type )
		{
			return istream;
		}
	}

	return nullptr;
}

}

Input::Input( const std::filesystem::path& path, AVMediaType type )
{
	assert( type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO );

	if( avformat_open_input( &format, path.c_str(), nullptr, nullptr ) < 0 )
	{
		throw FormatError( "could not open input file '%s'", path.c_str() );
	}

	auto res = avformat_find_stream_info( format, nullptr );
	assert( res >= 0 );

	stream_ = FindStream( format, type );
	if( !stream_ )
	{
		throw FormatError( "input '%s' does not include any %s streams", path.c_str(), type == AVMEDIA_TYPE_VIDEO ? "video" : "audio" );
	}

	packet = av_packet_alloc();
}

Input::~Input()
{
	avformat_close_input( &format );
	av_packet_free( &packet );
}

AVPacket* Input::Read()
{
	int result = 0;

	do
	{
		av_packet_unref(packet);
		result = av_read_frame( format, packet );
	}
	while( result >= 0 && packet->stream_index != stream_->index );

	return result >= 0 ? packet : nullptr;
}
