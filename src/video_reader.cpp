#include "video_reader.hpp"
#include <inttypes.h>

bool video_reader_open(VideoReaderState* state, const char* filename)
{
	//*unpack members of state
	auto& width				 = state->width;
	auto& height			 = state->height;
	auto& time_base			 = state->time_base;
	auto& av_format_ctx		 = state->av_format_ctx;
	auto& av_codec_ctx		 = state->av_codec_ctx;
	auto& av_frame			 = state->av_frame;
	auto& av_packet			 = state->av_packet;
	auto& video_stream_index = state->video_stream_index;
	auto& sws_scaler_ctx	 = state->sws_scaler_ctx;

	//*open the file using libavformat
	av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx)
	{
		fprintf(stderr, "could not allocate AVFormatContext\n");
		return false;
	}

	if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0)
	{
		printf("attempt to open: %s\n", filename);
		printf("couldn't open video file\n");
		return false;
	}

	//*find the first valid video stream in the file
	video_stream_index = -1;
	AVCodecParameters* av_codec_params;
	const AVCodec* av_codec;

	for (int i = 0; i < av_format_ctx->nb_streams; ++i)
	{
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec		= avcodec_find_decoder(av_codec_params->codec_id);
		if (!av_codec)
		{
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream_index = i;
			width			   = av_codec_params->width;
			height			   = av_codec_params->height;
			time_base		   = av_format_ctx->streams[i]->time_base;
			break;
		}
	}

	if (video_stream_index == -1)
	{
		printf("couldn't find video stream\n");
		return false;
	}

	//*set up a codec context for the decoder
	av_codec_ctx = avcodec_alloc_context3(av_codec);
	if (!av_codec_ctx)
	{
		printf("couldn't allocate AVCodecContext\n");
		return false;
	}
	if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0)
	{
		printf("couldn't initialize AVCodecContext\n");
		return false;
	}
	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0)
	{
		printf("couldn't open codec\n");
		return false;
	}

	av_frame = av_frame_alloc();
	if (!av_frame)
	{
		printf("couldn't allocate AVFrame\n");
		return false;
	}
	av_packet = av_packet_alloc();
	if (!av_packet)
	{
		printf("couldn't allocate AVPacket\n");
		return false;
	}
	//*set up sws scaler
	return true;
}

bool video_reader_read_frame(VideoReaderState* state, uint8_t* frame_buffer)
{
	// Unpack members of state
	auto& width				 = state->width;
	auto& height			 = state->height;
	auto& av_format_ctx		 = state->av_format_ctx;
	auto& av_codec_ctx		 = state->av_codec_ctx;
	auto& video_stream_index = state->video_stream_index;
	auto& av_frame			 = state->av_frame;
	auto& av_packet			 = state->av_packet;
	auto& sws_scaler_ctx	 = state->sws_scaler_ctx;

	int response;
	while (av_read_frame(av_format_ctx, av_packet) >= 0)
	{
		if (av_packet->stream_index != video_stream_index)
		{
			av_packet_unref(av_packet);
			continue;
		}

		response = avcodec_send_packet(av_codec_ctx, av_packet);
		if (response < 0)
		{
			printf("Failed to send packet for decoding: %s\n", av_err2str(response));
			av_packet_unref(av_packet);
			return false;
		}

		// Receive frames until all packets are processed
		while (response >= 0)
		{
			response = avcodec_receive_frame(av_codec_ctx, av_frame);
			if (response == AVERROR(EAGAIN))
			{
				// Decoder needs more input data
				break;
			}
			else if (response == AVERROR_EOF)
			{
				// End of stream
				av_packet_unref(av_packet);
				return false;
			}
			else if (response < 0)
			{
				printf("Error during decoding: %s\n", av_err2str(response));
				av_packet_unref(av_packet);
				return false;
			}

			// Process the decoded frame
			if (!sws_scaler_ctx)
			{
				sws_scaler_ctx = sws_getContext(width, height, av_codec_ctx->sw_pix_fmt, width, height, AV_PIX_FMT_RGB0, SWS_FAST_BILINEAR, NULL, NULL, NULL);
				if (!sws_scaler_ctx)
				{
					printf("Couldn't initialize SwsContext\n");
					av_packet_unref(av_packet);
					return false;
				}
			}

			uint8_t* dest[4]	 = {frame_buffer, NULL, NULL, NULL};
			int dest_linesize[4] = {width * 4, 0, 0, 0};
			sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, height, dest, dest_linesize);

			// After processing, unreference the packet and continue
			av_packet_unref(av_packet);
			return true;
		}
	}

	// If no frames were processed
	return false;
}

void video_reader_close(VideoReaderState* state)
{
	sws_freeContext(state->sws_scaler_ctx);
	avformat_close_input(&state->av_format_ctx);
	avformat_free_context(state->av_format_ctx);
	av_frame_free(&state->av_frame);
	av_packet_free(&state->av_packet);
	avcodec_free_context(&state->av_codec_ctx);
}