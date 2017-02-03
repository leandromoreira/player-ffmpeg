/*
 * This follows the tutorial on http://dranger.com/ffmpeg/tutorial01.html but using a most up to date ffmpeg libraries.
 *
 * This was made based on ffmpeg 3.2.2 and tested with macosx 10.12.3 (gcc stable 6.3.0)
 *
 * https://www.ffmpeg.org/doxygen/3.2/index.html
 *
 * Container - a wrapper, providing sync, metadata and muxing for the streams.
 * Stream - a continuous stream (audio and video) of data over time, the data itself are the frames, each stream is encoded by a different codec.
 * Codec - defines how data are COded and DECoded.
 * Packet - are the data decoded as raw frames (for this simple explanation), one frame for video and multiple for audio.
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

int main(int argc, const char *argv[])
{
  // registering all the formats (containers), codecs and protocols.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__core.html#ga917265caec45ef5a0646356ed1a507e3
  av_register_all();

  // holds container header info
  // https://www.ffmpeg.org/doxygen/3.2/structAVFormatContext.html
  AVFormatContext *pFormatCtx = NULL;

  // Open an input stream and read the header. The codecs are not opened.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
  if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
  {
    return -1;
  }

  // Read packets of a media file to get stream information.
  // this function populates pFormatCtx->streams (of size equals to pFormatCtx->nb_streams)
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
  {
    return -1;
  }

  // show debug info about the format onto standard error fd
  av_dump_format(pFormatCtx, 0, argv[1], 0);

  return 0;
}
