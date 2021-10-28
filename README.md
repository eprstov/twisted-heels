# twisted heels
a primitive hls mixer

### coding profiles
- 420x240, 500 Kbps
- 840x480, 1 Mbps
- 1280x720, 2 Mbps

### how to build
1. create a docker image using the Dockerfile provided
2. in the docker container go to the source directory
3. build the sources with `cmake -B.make && make -C.make`
4. the executable now is in `.make/mixer`

### how to run
```
usage: -a <audio source path> -v <video source path> -o <output dir path>
```
- `-a` the path to a container to extract an audio track from
- `-v` the path to a container to extract a video track from
- `-o` the output directory

### known issues
hls.js struggles switching between video tracks while playing the output
