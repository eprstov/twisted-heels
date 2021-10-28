#include "scaler.h"

#include <assert.h>

Scaler::Scaler( const AVCodecParameters* from, const AVCodecParameters* to )
{
	context = sws_getContext( from->width, from->height, AVPixelFormat( from->format ), to->width, to->height, AVPixelFormat( to->format ), 0, nullptr, nullptr, nullptr );
	assert(context);

	output = av_frame_alloc();
	assert(output);
}

AVFrame* Scaler::operator()( AVFrame* input )
{
	int res;
	av_frame_unref(output);

	res = av_frame_copy_props( output, input );
	assert( res >= 0 );

	res = sws_scale_frame( context, output, input );
	assert( res >= 0 );

	return output;
}

Scaler::~Scaler()
{
	av_frame_free( &output );
	sws_freeContext(context);
}
