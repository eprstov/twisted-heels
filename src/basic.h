#pragma once

extern "C"
{

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>

}

#include <deque>
#include <stdexcept>

template< typename... Args >
std::runtime_error FormatError( const char* format, Args... args )
{
	char buffer[1024] = {0};
	snprintf( buffer, sizeof(buffer), format, args... );

	return std::runtime_error(buffer);
}

struct Dimensions
{
	Dimensions() = default;

	template< typename T > requires std::is_integral_v<T>
	Dimensions( T w, T h )  : width(w), height(h)
	{
	}

	Dimensions( const AVCodecParameters* codec ) : width( codec->width ), height( codec->height )
	{
	}

	friend bool operator==( const Dimensions& left, const Dimensions& right )
	{
		return left.width == right.width && left.height == right.height;
	}

	std::size_t width, height;
};

// simplified container
template< typename C, typename T > concept Container = requires( C cont, const C ccont )
{
	std::is_same_v< decltype( ccont.begin() ), decltype( ccont.begin() ) >;
	std::is_same_v< decltype( *ccont.begin() ), T >;
	ccont.begin()++ != ccont.end();
	cont.clear();
};

enum Bitrate : std::size_t {};

Bitrate inline operator "" _Kbps( unsigned long long val )
{
	return Bitrate( val * 1000 );
}

Bitrate inline operator "" _Mbps( unsigned long long val )
{
	return Bitrate( val * 1000 * 1000 );
}
