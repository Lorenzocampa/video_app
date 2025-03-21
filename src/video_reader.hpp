#ifndef video_reader_hpp
#define video_reader_hpp
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <inttypes.h>
}

struct VideoReaderState
{
	VideoReaderState()	= default;
	~VideoReaderState() = default;

	//*public things for other parts of the program to read from
	int width, height;
	AVRational time_base;

	//*private internal state
	AVFormatContext* av_format_ctx;
	AVCodecContext* av_codec_ctx;
	int video_stream_index;
	AVPacket* av_packet;
	AVFrame* av_frame;
	SwsContext* sws_scaler_ctx;
};

bool video_reader_open(VideoReaderState* state, const char* filename);
bool video_reader_read_frame(VideoReaderState* state, uint8_t* frame_buffer);
void video_reader_close(VideoReaderState* state);

#endif // VIDEO_READER_HPP
