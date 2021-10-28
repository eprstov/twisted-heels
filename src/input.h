#pragma once

extern "C"
{

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include <filesystem>

class Input
{
public:
	Input( const std::filesystem::path& path, AVMediaType );
	~Input();

	AVPacket* Read();

	const AVStream* stream() const { return stream_; }

private:
	AVFormatContext* format = nullptr;

	AVStream* stream_ = nullptr;
	AVPacket* packet = nullptr;
};
