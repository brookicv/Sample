#pragma once

#include <iostream>

extern "C" {

#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>

}

enum MEDIA_TYPE
{
	NONE,
	VIDEO, // Only video infomation
	AUDIO, // Only audio infomation
	ALL // Video and audio infomation
};

enum  ERROR_TYPE
{
	OPEN,             // Open input failed
	STREAM_INFO,      // Couldn't find stream info
	AUDIO_STREAM,     // Couldn't find audio stream index
	VIDEO_STREAM,     // Couldn't find video stream index
	AUDIO_CODEC,      // Couldn't find audio codec
	VIDEO_CODEC,      // Couldn't find video codec
	AUDIO_CODEC_OPEN, // Open audio codec context failed
	VIDEO_CODEC_OPEN, // Open video codec context failed
	MISS_TYPE,        // Miss the media type
	OK                // Succes
};

class CMediaInfo
{

public:
	CMediaInfo();
	CMediaInfo(MEDIA_TYPE media);
	~CMediaInfo();

public:
	ERROR_TYPE open(const char *filename);
	void close();
	void error_message(ERROR_TYPE error);

public:
	MEDIA_TYPE type;
	AVFormatContext *pFormatContext;

	AVCodecContext *pVideo_codec_context;
	AVCodecContext *pAudio_codec_context;

	int video_stream_index;
	int audio_stream_index;
};

