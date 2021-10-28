#pragma once

#include "basic.h"

extern "C"
{

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include <filesystem>
#include <type_traits>

class Builder
{
public:
	Builder( std::filesystem::path );
	~Builder();

	AVStream* Add( const AVCodecParameters* );

	void operator()( AVPacket* );

	template< Container< AVPacket* > C >
	void operator()( const C& packets )
	{
		for( auto packet : packets )
		{
			operator()(packet);
		}
	}

	void Start();
	void Finish();

private:
	bool started = false;

	std::filesystem::path path;

	AVFormatContext* format = nullptr;
	AVDictionary* opts = nullptr;
};
