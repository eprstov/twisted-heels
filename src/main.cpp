#include "basic.h"
#include "builder.h"
#include "input.h"
#include "processor.h"

#include <filesystem>
#include <iostream>
#include <unistd.h>

struct Profile
{
	Dimensions dimensions;
	Bitrate bitrate;
	std::size_t gop;
};

void operator|=( AVCodecParameters* codec, const Profile& profile )
{
	codec->width = profile.dimensions.width;
	codec->height = profile.dimensions.height;
	codec->bit_rate = profile.bitrate;
}

struct {} usage;
std::ostream& operator<<( std::ostream& stream, const decltype(usage)& )
{
	return stream << "usage: -a <audio source path> -v <video source path> -o <output dir path>";
}

const std::vector<Profile> profiles =
{
	{ { 420, 240 }, 500_Kbps, 60 },
	{ { 840, 480 }, 1_Mbps, 60 },
	{ { 1280, 720 }, 2_Mbps, 60 },
};

int main( int argc, char **argv )
{
	std::filesystem::path apath, vpath, opath;
	for( int c; (c = getopt( argc, argv, "ha:v:o:" )) != -1; )
	{
		switch (c)
		{
		case 'h':
			std::cout << usage << std::endl;
			return 0;
		case 'a':
			apath = optarg;
			break;

		case 'v':
			vpath = optarg;
			break;

		case 'o':
			opath = optarg;
			break;

		default:
			std::cerr << "unknown option " << char(c) << std::endl;
			std::cerr << usage << std::endl;

			return -1;
		}
	}

	if( apath.empty() || vpath.empty() || opath.empty() )
	{
		std::cerr << usage << std::endl;
		return -1;
	}

	try
	{
		Input vinput( vpath, AVMEDIA_TYPE_VIDEO );
		Input ainput( apath, AVMEDIA_TYPE_AUDIO );

		Builder builder(opath);

		auto audio = builder.Add( ainput.stream()->codecpar );
		audio->time_base = ainput.stream()->time_base;

		std::vector<Processor> procs;
		procs.reserve( profiles.size() );

		for( const auto& profile : profiles )
		{
			auto* istream = vinput.stream();
			auto* ostream = builder.Add( istream->codecpar );
			ostream->time_base = istream->time_base;

			avcodec_parameters_copy( ostream->codecpar, istream->codecpar );
			ostream->codecpar |= profile;
			ostream->avg_frame_rate = istream->avg_frame_rate;

			auto& proc = procs.emplace_back( istream, ostream, Processor::Config{ profile.gop } );
			proc.Adjust( ostream->codecpar );
		}

		builder.Start();

		while( AVPacket* packet = vinput.Read() )
		{
			for( auto i = 0; i < procs.size(); ++i )
			{
				auto& proc = procs[i];
				builder( proc(packet) );
			}
		}

		for( auto& proc : procs )
		{
			builder( proc() );
		}

		while( AVPacket* packet = ainput.Read() )
		{
			packet->stream_index = audio->index;
			av_packet_rescale_ts( packet, ainput.stream()->time_base, audio->time_base );
			packet->pos = -1;

			builder(packet);
		}

		builder.Finish();
	}
	catch( const std::exception& ex )
	{
		std::cout << "error: " << ex.what() << std::endl;
	}

	return 0;
}
