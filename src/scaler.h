#pragma once

#include "basic.h"

extern "C"
{

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

}

class Scaler
{
public:
	Scaler( const AVCodecParameters* from, const AVCodecParameters* to );
	~Scaler();

	AVFrame* operator()( AVFrame* input );

private:
	Dimensions dimensions;
	SwsContext* context = nullptr;
	AVFrame* output = nullptr;
};
