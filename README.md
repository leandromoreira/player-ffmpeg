# Intro

This follows the tutorial on http://dranger.com/ffmpeg/tutorial01.html but using a most up to date ffmpeg libraries. (ie: ffmpeg/swscale become libswscale/swscale)

This was tested with `MacOSx 10.12.3`, `ffmpeg 3.2.2` and `GCC stable 6.3.0` but I think you can run it on linux and even windows (with a few caveats).

## FFmpeg API documentation for this version

https://www.ffmpeg.org/doxygen/3.2/index.html

## Basic terminoogy

Container - a wrapper, providing sync, metadata and muxing for the streams.
Stream - a continuous stream (audio and video) of data over time, the data itself are the frames, each stream is encoded by a different codec.
Codec - defines how data are COded and DECoded.
Packet - are the data decoded as raw frames (for this simple explanation), one frame for video and multiple for audio.
