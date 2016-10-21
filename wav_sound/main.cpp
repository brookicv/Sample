
#include <iostream>
#include <fstream>
#include "Wav.h"
#include <cstdio>

#include "MediaInfo.h"

using namespace std;

int to_pcm()
{
	char* filename = "E:\\Wildlife.wmv";

	uint8_t *buffer = new uint8_t[192000];

	CMediaInfo media(MEDIA_TYPE::AUDIO);
	media.open(filename);

	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();

	auto duration = media.pFormatContext->duration + (media.pFormatContext->duration <= INT64_MAX - 5000 ? 5000 : 0);

	ofstream ofs("wildlife.wav", ofstream::binary);

	AVSampleFormat dst_format = AV_SAMPLE_FMT_S16;
	uint8_t dst_channels = 2;
	auto dst_layout = av_get_default_channel_layout(dst_channels);

	auto audio_ctx = media.pAudio_codec_context;
	if (audio_ctx->channel_layout <= 0)
		audio_ctx->channel_layout = av_get_default_channel_layout(audio_ctx->channels);
	SwrContext *swr_ctx = swr_alloc();

	swr_alloc_set_opts(swr_ctx, dst_layout, dst_format, audio_ctx->sample_rate,
		audio_ctx->channel_layout, audio_ctx->sample_fmt, audio_ctx->sample_rate, 0, nullptr);
	if (!swr_ctx || swr_init(swr_ctx))
		return -1;

	int pcm_data_size = 0;
	while (av_read_frame(media.pFormatContext, packet) >= 0)
	{
		if (packet->stream_index == media.audio_stream_index)
		{
			auto ret = avcodec_send_packet(media.pAudio_codec_context, packet);

			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
				return -1;

			ret = avcodec_receive_frame(media.pAudio_codec_context, frame);
			if (ret < 0 && ret != AVERROR_EOF)
				return -1;

			auto nb = swr_convert(swr_ctx, &buffer, 192000, (const uint8_t **)frame->data, frame->nb_samples);

			auto length = nb * dst_channels * av_get_bytes_per_sample(dst_format);
			ofs.write((char*)buffer, length);

			pcm_data_size += length;
		}
		av_packet_unref(packet);
	}

	Wave_header header(dst_channels, audio_ctx->sample_rate, av_get_bytes_per_sample(dst_format) * 8);
	header.data->cb_size = ((pcm_data_size + 1) / 2) * 2;
	header.riff->cb_size = 4 + 4 + header.fmt->cb_size + 4 + 4 + header.data->cb_size + 4;

	ofs.seekp(0, ios::beg);
	CWaveFile::write_header(ofs, header);

	ofs.close();
	delete[] buffer;
	swr_free(&swr_ctx);
	av_packet_free(&packet);
	av_frame_free(&frame);

	media.close();

	return 0;
}

int main()
{
	av_register_all();

	AVFormatContext *pFormat_context = nullptr;
	AVOutputFormat *pOutput_format   = nullptr;

	AVCodecContext *pCodec_context   = nullptr;
	AVCodec *pCodec                  = nullptr;
	AVStream *audio_st               = nullptr;
	
	pFormat_context = avformat_alloc_context();
	char* out_file = "life.aac";
	pOutput_format = av_guess_format(nullptr, out_file, nullptr);

	pFormat_context->oformat = pOutput_format;

	avio_open(&pFormat_context->pb, out_file, AVIO_FLAG_READ_WRITE);

	audio_st = avformat_new_stream(pFormat_context, nullptr);

	pCodec_context = audio_st->codec;

	AVCodecParameters *codec_para = avcodec_parameters_alloc();
	

	pCodec_context->codec_id = pOutput_format->audio_codec;
	pCodec_context->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodec_context->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodec_context->sample_rate = 44100;
	pCodec_context->channel_layout = AV_CH_LAYOUT_STEREO;
	pCodec_context->channels = av_get_channel_layout_nb_channels(pCodec_context->channel_layout);
	pCodec_context->bit_rate = 64000;

	av_dump_format(pFormat_context, 0, out_file, 1);

	cout << "end." << endl;
	getchar();
	return 0;
}


