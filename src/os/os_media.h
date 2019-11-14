/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

typedef struct OSMedia_s
{
	AVOutputFormat* fmt;
	AVIOContext* avioContext;
	AVFormatContext* container;

	AVCodec* codec_audio;
	AVCodec* codec_video;
	AVCodec* codec_subtitles;

	AVCodecContext* codec_context_video;
	AVFrame* frame_video;

	Vec2i origSize;

	OSMediaCallback_read* funcRead;
	OSMediaCallback_seek* funcSeek;
	void* func_data;
} OSMedia;

BOOL OSMedia_isAudio(const OSMedia* self)
{
	return self->fmt->audio_codec != AV_CODEC_ID_NONE;
}

BOOL OSMedia_isVideo(const OSMedia* self)
{
	return self->fmt->video_codec != AV_CODEC_ID_NONE;
}

BOOL OSMedia_isSubtitles(const OSMedia* self)
{
	return self->fmt->subtitle_codec != AV_CODEC_ID_NONE;
}

static int _OSMedia_readFunction(void* opaque, uint8_t* buf, int buf_size)
{
	OSMedia* self = opaque;
	return self->funcRead(self->func_data, buf, buf_size);
}

static int64_t _OSMedia_seekFunction(void* opaque, int64_t offset, int whence)
{
	if (whence == AVSEEK_SIZE)
		offset = -1;
	//return -1; // I don't know "size of my handle in bytes"

	OSMedia* self = opaque;
	return self->funcSeek(self->func_data, offset);
}

void OSMedia_delete(OSMedia* self)
{
	//Image4_free(&self->origImg);

	if (self->frame_video)	av_frame_free(&self->frame_video);

	if (self->codec_context_video)	avcodec_free_context(&self->codec_context_video);

	if (self->container)	avformat_free_context(self->container);
	if (self->avioContext)	av_freep(&self->avioContext);	//avio_context_free(&self->avioContext);

	memset(self, 0, sizeof(OSMedia));
	free(self);
}

//note: fileName is the name, not path
OSMedia* OSMedia_new(OSMediaCallback_read* funcRead, OSMediaCallback_seek* funcSeek, void* func_data, const char* ext)
{
	AVOutputFormat* fmt = av_guess_format(0, ext, 0);
	if (!fmt)
		return 0;

	//...
	if (Std_cmpCHAR(ext, ".jpg") || Std_cmpCHAR(ext, ".jpeg"))
		fmt->video_codec = AV_CODEC_ID_MJPEG;
	else
		if (Std_cmpCHAR(ext, ".png"))
			fmt->video_codec = AV_CODEC_ID_PNG;
		else
			if (Std_cmpCHAR(ext, ".bmp"))
				fmt->video_codec = AV_CODEC_ID_BMP;

	OSMedia* self = calloc(1, sizeof(OSMedia));
	self->fmt = fmt;
	self->funcRead = funcRead;
	self->funcSeek = funcSeek;
	self->func_data = func_data;
	//...

	const int ioBufferSize = 32 * 1024;
	unsigned char* ioBuffer = (unsigned char*)av_malloc(ioBufferSize + AV_INPUT_BUFFER_PADDING_SIZE);
	self->avioContext = avio_alloc_context(ioBuffer, ioBufferSize, 0, self, &_OSMedia_readFunction, NULL, &_OSMedia_seekFunction);

	self->container = avformat_alloc_context();
	self->container->pb = self->avioContext;
	avformat_open_input(&self->container, "dummyFileName", NULL, NULL);

	//av_seek_frame(container, ...);	//seek ...

	/*AVCodecContext* c = st->codec;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_AUDIO;

	//put sample parameters
	c->bit_rate = 64000;
	c->sample_rate = 44100;
	c->channels = 2;*/

	/*
		AVCodecContext *pCodecCtx = st->audio_codec;
		AVCodecContext* pCodecCtx = pFormatCtx->streams[0]->codec;
		pCodecCtx->width = size.x;
		pCodecCtx->height = size.y;
		pCodecCtx->pix_fmt = PIX_FMT_YUV420P;*/

	if (OSMedia_isVideo(self))
	{
		self->codec_video = avcodec_find_decoder(fmt->video_codec);
		if (!self->codec_video)
		{
			OSMedia_delete(self);
			return 0;
		}

		self->codec_context_video = avcodec_alloc_context3(self->codec_video);
		//self->codec_context_video->pix_fmt = AV_PIX_FMT_RGB32;

		if (avcodec_open2(self->codec_context_video, self->codec_video, NULL) < 0)
		{
			OSMedia_delete(self);
			return 0;
		}

		self->frame_video = av_frame_alloc();
		if (!self->frame_video)
		{
			OSMedia_delete(self);
			return 0;
		}

		//c->get_buffer2 = lavc_GetFrame;	//callback
		//self->codec_context_video->width = size.x;	//možná rozlišení musí být power(2) nebo sudé? ...
		//self->codec_context_video->height = size.y;
		//self->codec_context_video->pix_fmt = AV_PIX_FMT_RGB32;	//jsou i různé jiné pořadí ...

		//self->codec_context_video->pix_fmt = AV_PIX_FMT_RGB32;
		avformat_find_stream_info(self->container, NULL);
		int videoStream = -1;
		int i;
		for (i = 0; i < self->container->nb_streams; i++)
			if (self->container->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0)
				videoStream = i;
		if (videoStream >= 0)
			self->origSize = Vec2i_init2(self->container->streams[videoStream]->codecpar->width, self->container->streams[videoStream]->codecpar->height);

		//self->codec_context_video->pix_fmt = AV_PIX_FMT_RGB32;

		if (videoStream < 0)
		{
			OSMedia_delete(self);
			return 0;
		}
	}

	return self;
}

const Vec2i* OSMedia_getOrigSize(const OSMedia* self)
{
	return &self->origSize;
}

void OSMedia_loadVideo(OSMedia* self, Image4* out)
{
	if (!self->codec_context_video)
		return;

	AVPacket pkt;
	av_init_packet(&pkt);

	if (!av_read_frame(self->container, &pkt))
	{
		if (!avcodec_send_packet(self->codec_context_video, &pkt))
		{
			if (!avcodec_receive_frame(self->codec_context_video, self->frame_video))
			{
				if (Vec2i_is(out->size))
				{
					AVFrame* frame = self->frame_video;

					struct SwsContext* context = sws_getContext(frame->width, frame->height, self->codec_context_video->pix_fmt, out->size.x, out->size.y, AV_PIX_FMT_RGB32, 0, 0, 0, 0);
					if (context)
					{
						/*uint8_t* data[2] = { Y, Y + Stride * Height };
						int linesize[2] = { Stride, Stride };
						uint8_t* outData[1] = { out }; // RGB have one plane
						int outLinesize[1] = { frameWidth * 4 }; // RGB32 Stride
						sws_scale(context, data, linesize, 0, Height, outData, outLinesize);*/

						uint8_t* outData[1] = { (UCHAR*)out->data }; // RGB have one plane
						int outLinesize[1] = { out->size.x * 4 }; // RGB32 Stride

						sws_scale(context, (const UCHAR* const*)frame->data, frame->linesize, 0, frame->height, outData, outLinesize);

						sws_freeContext(context);
					}
					//This it for audio
					//int data_size = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, self->codec_context_video->sample_fmt, 1);
					//data_size *= 2;
					//memcpy(dst, decoded_frame->data[0], data_size);
				}
			}
		}
	}
}
