#pragma once

#include "basic.h"
#include "pool.h"
#include "scaler.h"

extern "C"
{

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include <deque>
#include <vector>

class Processor
{
public:
	struct Config
	{
		std::size_t gop;
	};

	Processor( const AVStream* input, const AVStream* output, Config&& );
	~Processor();

	void Adjust( AVCodecParameters* ) const;

	const std::vector< AVPacket* >& operator()( const AVPacket* );
	std::vector< AVPacket* > operator()(); // flush

private:
	void CreateDecoder();
	void CreateEncoder();

	void Receive();

	AVCodecContext* encoder = nullptr;
	AVCodecContext* decoder = nullptr;

	const AVStream* istream = nullptr;
	const AVStream* ostream = nullptr;

	AVPacket* ipacket = nullptr;
	AVFrame* iframe = nullptr;

	std::vector< AVPacket* > results;
	Pool<AVPacket> packets;

	Scaler scaler;
	Config config;
};
