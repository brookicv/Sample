#include "MediaInfo.h"


CMediaInfo::CMediaInfo()
{
	type = MEDIA_TYPE::NONE;

	pFormatContext = nullptr;

	pVideo_codec_context = nullptr;
	pAudio_codec_context = nullptr;

	video_stream_index = -1;
	audio_stream_index = -1;
}

CMediaInfo::CMediaInfo(MEDIA_TYPE media)
	:type(media)
{
	pFormatContext = nullptr;

	pVideo_codec_context = nullptr;
	pAudio_codec_context = nullptr;

	video_stream_index = -1;
	audio_stream_index = -1;
}


CMediaInfo::~CMediaInfo()
{
	if (pFormatContext)
	{
		avformat_close_input(&pFormatContext);
		pFormatContext = nullptr;
	}

	if (pAudio_codec_context)
	{
		avcodec_close(pAudio_codec_context);
		pAudio_codec_context = nullptr;
	}

	if (pVideo_codec_context)
	{
		avcodec_close(pVideo_codec_context);
		pVideo_codec_context = nullptr;
	}
}

ERROR_TYPE CMediaInfo::open(const char *filename)
{
	if (type == MEDIA_TYPE::NONE)
		return ERROR_TYPE::MISS_TYPE;

	av_register_all();

	if (avformat_open_input(&pFormatContext, filename, nullptr, nullptr) < 0)
		return ERROR_TYPE::OPEN;

	if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
		return ERROR_TYPE::STREAM_INFO;

#ifdef _DEBUG
	av_dump_format(pFormatContext, 0, filename, 0);
#endif

	// Find stream index
	for (size_t i = 0; i < pFormatContext->nb_streams; i++)
	{
		if ((type == MEDIA_TYPE::ALL || type == MEDIA_TYPE::AUDIO) &&
			pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
			audio_stream_index < 0)

			audio_stream_index = i;

		if ((type == MEDIA_TYPE::ALL || type == MEDIA_TYPE::VIDEO) &&
			pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
			video_stream_index < 0)

			video_stream_index = i;
	}

	// Video and audio
	if (type == MEDIA_TYPE::ALL)
	{
		if (audio_stream_index < 0)
			return ERROR_TYPE::AUDIO_STREAM;
		if (video_stream_index < 0)
			return ERROR_TYPE::VIDEO_STREAM;

		// Open audio codec context
		AVCodec *pAudio_codec = avcodec_find_decoder(pFormatContext->streams[audio_stream_index]->codecpar->codec_id);
		if (!pAudio_codec)
			return ERROR_TYPE::AUDIO_CODEC;

		pAudio_codec_context = avcodec_alloc_context3(pAudio_codec);
		//avcodec_copy_context(pAudio_codec_context, pFormatContext->streams[audio_stream_index]->codec);

		avcodec_parameters_to_context(pAudio_codec_context, pFormatContext->streams[audio_stream_index]->codecpar);
		if (avcodec_open2(pAudio_codec_context, pAudio_codec, nullptr) < 0)
			return ERROR_TYPE::AUDIO_CODEC_OPEN;

		// Open video codec context
		AVCodec *pVideo_codec = avcodec_find_decoder(pFormatContext->streams[video_stream_index]->codecpar->codec_id);
		if (!pVideo_codec)
			return ERROR_TYPE::VIDEO_CODEC;

		pVideo_codec_context = avcodec_alloc_context3(pVideo_codec);
		avcodec_parameters_to_context(pVideo_codec_context, pFormatContext->streams[video_stream_index]->codecpar);

		if (avcodec_open2(pVideo_codec_context, pVideo_codec, nullptr) < 0)
			return ERROR_TYPE::VIDEO_CODEC_OPEN;

		return ERROR_TYPE::OK;
	}

	// Only video
	if (type == MEDIA_TYPE::VIDEO)
	{
		if (video_stream_index < 0)
			return ERROR_TYPE::VIDEO_STREAM;

		// Open video codec context
		AVCodec *pVideo_codec = avcodec_find_decoder(pFormatContext->streams[video_stream_index]->codecpar->codec_id);
		if (!pVideo_codec)
			return ERROR_TYPE::VIDEO_CODEC;

		pVideo_codec_context = avcodec_alloc_context3(pVideo_codec);
		avcodec_parameters_to_context(pVideo_codec_context, pFormatContext->streams[video_stream_index]->codecpar);

		if (avcodec_open2(pVideo_codec_context, pVideo_codec, nullptr) < 0)
			return ERROR_TYPE::VIDEO_CODEC_OPEN;

		return ERROR_TYPE::OK;
	}

	// Only audio
	if (type == MEDIA_TYPE::AUDIO)
	{
		if (audio_stream_index < 0)
			return ERROR_TYPE::AUDIO_STREAM;

		// Open audio codec context
		AVCodec *pAudio_codec = avcodec_find_decoder(pFormatContext->streams[audio_stream_index]->codecpar->codec_id);
		if (!pAudio_codec)
			return ERROR_TYPE::AUDIO_CODEC;

		pAudio_codec_context = avcodec_alloc_context3(pAudio_codec);
		avcodec_parameters_to_context(pAudio_codec_context, pFormatContext->streams[audio_stream_index]->codecpar);

		if (avcodec_open2(pAudio_codec_context, pAudio_codec, nullptr) < 0)
			return ERROR_TYPE::AUDIO_CODEC_OPEN;

		return ERROR_TYPE::OK;
	}

	return ERROR_TYPE::OK;
}

void CMediaInfo::close()
{
	if (pFormatContext)
	{
		avformat_close_input(&pFormatContext);
		pFormatContext = nullptr;
	}

	if (pAudio_codec_context)
	{
		avcodec_free_context(&pAudio_codec_context);
		pAudio_codec_context = nullptr;
	}

	if (pVideo_codec_context)
	{
		avcodec_free_context(&pVideo_codec_context);
		pVideo_codec_context = nullptr;
	}

	type = MEDIA_TYPE::NONE;
	video_stream_index = -1;
	audio_stream_index = -1;
}

void CMediaInfo::error_message(ERROR_TYPE error)
{
	switch (error)
	{
	case ERROR_TYPE::OPEN:
		std::cerr << "Could not open the media file" << std::endl;
		break;

	case ERROR_TYPE::STREAM_INFO:
		std::cerr << "Could not find the stream info." << std::endl;
		break;

	case ERROR_TYPE::AUDIO_STREAM:
		std::cerr << "Could not find the index of audio stream." << std::endl;
		break;

	case ERROR_TYPE::VIDEO_STREAM:
		std::cerr << "Could not find the index of video stream." << std::endl;
		break;

	case ERROR_TYPE::VIDEO_CODEC:
		std::cerr << "Could not find the video CODEC." << std::endl;
		break;

	case ERROR_TYPE::AUDIO_CODEC:
		std::cerr << "Could not find the audio CODEC." << std::endl;
		break;

	case ERROR_TYPE::AUDIO_CODEC_OPEN:
		std::cerr << "Could not open the audio CODEC context." << std::endl;
		break;

	case ERROR_TYPE::VIDEO_CODEC_OPEN:
		std::cerr << "Could not open the video CODEC context." << std::endl;
		break;

	case ERROR_TYPE::MISS_TYPE:
		std::cerr << "Miss the media type." << std::endl;
		break;
	}
}
