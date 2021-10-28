FROM debian:latest
RUN apt-get update \
	&& apt-get install -y cmake make g++ pkg-config libx264-dev \
	&& apt-get install -y git yasm \
	&& git clone --depth 1 https://github.com/FFmpeg/FFmpeg.git /tmp/ffmpeg \
	&& cd /tmp/ffmpeg && ./configure --enable-libx264 --enable-gpl && make install \
	&& cd / && rm -rf /tmp/ffmpeg && apt-get remove -y git yasm \
	&& rm -rf /var/lib/apt/lists/*
