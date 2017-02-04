/*
 * This follows the tutorial on http://dranger.com/ffmpeg/tutorial01.html but using a most up to date ffmpeg libraries.
 *
 * This was made based on ffmpeg 3.2.2 and tested with macosx 10.12.3 (gcc stable 6.3.0)
 *
 * https://www.ffmpeg.org/doxygen/3.2/index.html
 *
 * Container (Format) - a wrapper, providing sync, metadata and muxing for the streams.
 * Stream - a continuous stream (audio and video) of data over time, the data itself are the frames, each stream is encoded by a different codec.
 * Codec - defines how data are COded and DECoded.
 * Packet - are the data decoded as raw frames (for this simple explanation), one frame for video and multiple for audio.
 * Frame - a decoded raw frame
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);
void logg(const char *str);

int main(int argc, const char *argv[])
{
  logg("initializing");
  // registering all the formats (containers), codecs and protocols.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__core.html#ga917265caec45ef5a0646356ed1a507e3
  av_register_all();

  // holds container header info
  // https://www.ffmpeg.org/doxygen/3.2/structAVFormatContext.html
  AVFormatContext *pFormatCtx = NULL;
  int i;
  // https://www.ffmpeg.org/doxygen/3.2/structAVCodecContext.html
  AVCodecContext *pCodecCtxOrig = NULL;
  AVCodecContext *pCodecCtx = NULL;
  int videoStream = -1;
  // https://www.ffmpeg.org/doxygen/3.2/structAVCodec.html
  AVCodec *pCodec = NULL;
  AVFrame *pFrame = NULL;
  AVFrame *pFrameRGB = NULL;
  uint8_t *buffer = NULL;
  int numBytes;
  struct SwsContext *sws_ctx = NULL;
  int frameFinished;
  AVPacket packet;

  logg("opening the input, loading format (container) header");
  // Open an input stream and read the header. The codecs are not opened.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
  if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
  {
    logg("ERROR could not open the file");
    return -1;
  }

  logg("finding stream info from format");
  // Read packets of a media file to get stream information.
  // this function populates pFormatCtx->streams (of size equals to pFormatCtx->nb_streams)
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
  {
    logg("ERROR could not get the stream info");
    return -1;
  }

  // Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavf__misc.html#gae2645941f2dc779c307eb6314fd39f10
  // av_dump_format(pFormatCtx, 0, argv[1], 0);

  // Find the first video stream
  for (i = 0; i < pFormatCtx->nb_streams; i++)
  {
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      logg("found video stream");
      videoStream = i;
      break;
    }
  }

  if (videoStream==-1)
  {
    logg("ERROR could not find the video stream");
    return -1;
  }

  // get a pointer to the codec context for the video stream
  pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

  logg("finding the proper decoder");
  // Find a registered decoder with a matching codec ID.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
  pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);

  if (pCodec==NULL)
  {
    logg("ERROR unsupported codec!");
    return -1;
  }

  // Allocate an AVCodecContext and set its fields to default values.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavc__core.html#gae80afec6f26df6607eaacf39b561c315
  pCodecCtx = avcodec_alloc_context3(pCodec);

  // Copy the settings of the source AVCodecContext into the destination AVCodecContext.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavc__core.html#gae381631ba4fb14f4124575d9ceacb87eO
  // Deprectated
  if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0)
  {
    logg("ERROR could not copy the codec context");
    return -1;
  }

  // Initialize the AVCodecContext to use the given AVCodec.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
  {
    logg("ERROR could not open the codec context");
    return -1;
  }

  // Allocate an AVFrame and set its fields to default values.
  // https://www.ffmpeg.org/doxygen/3.2/group__lavu__frame.html#gac700017c5270c79c1e1befdeeb008b2f
  pFrame = av_frame_alloc();
  pFrameRGB = av_frame_alloc();

  if (pFrameRGB == NULL || pFrame == NULL)
  {
    logg("ERROR could not allocated memory for the frames");
    return -1;
  }

  logg("getting number of needed byte for a full frame");
  // Return the size in bytes of the amount of data required to store an image with the given parameters
  // https://www.ffmpeg.org/doxygen/3.2/group__lavu__picture.html#ga24a67963c3ae0054a2a4bab35930e694
  numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

  logg("allocating memory for the frame");
  // allocates memory
  // https://www.ffmpeg.org/doxygen/3.2/tableprint__vlc_8h.html#ae97db1f58b6b1515ed57a83bea3dd572
  buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

  logg("setup frame");
  avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

  // initialize SWS context for software scaling
  sws_ctx = sws_getContext(pCodecCtx->width,
      pCodecCtx->height,
      pCodecCtx->pix_fmt,
      pCodecCtx->width,
      pCodecCtx->height,
      AV_PIX_FMT_RGB24,
      SWS_BILINEAR,
      NULL,
      NULL,
      NULL
      );

  i=0;

  logg("reading frame");
  while(av_read_frame(pFormatCtx, &packet) >= 0)
  {
    if (packet.stream_index == videoStream)
    {
      logg("decoding frame");
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
    }

    if (frameFinished)
    {
      logg("converting color to RGB 24b");
      //convert from yuv420 to 24b rgb
      sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
          pFrame->linesize, 0, pCodecCtx->height,
          pFrameRGB->data, pFrameRGB->linesize);
      if(++i<=1)
      {
        SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
        logg("releasing frame");
        av_free_packet(&packet);
        break;
      }
    }

    logg("releasing frame");
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  logg("releasing all the resources");
  // Free the RGB image
  av_free(buffer);
  av_free(pFrameRGB);

  // Free the YUV frame
  av_free(pFrame);

  // Close the codecs
  avcodec_close(pCodecCtx);
  avcodec_close(pCodecCtxOrig);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  logg("saving the frame");

  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;

  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);

  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

  // Close file
  fclose(pFile);
}

void logg(const char *str)
{
  printf("LOG: %s\n", str);
}
