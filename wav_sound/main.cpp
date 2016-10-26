
#include <iostream>
#include <fstream>
#include "Wav.h"
#include <cstdio>

#include "MediaInfo.h"
#include "..\SoundTouch\SoundTouch.h"

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

	////////////////////////////////////////////////////////////////////
	// 设置SoundTouch，配置变调变速参数
	soundtouch::SoundTouch s_touch;
	s_touch.setSampleRate(audio_ctx->sample_rate); // 设置采样率
	s_touch.setChannels(audio_ctx->channels); // 设置通道数

	////////////////////////////////////////////
	// 设置 rate或者pitch的改变参数
	//s_touch.setRate(0.5); // 设置速度为0.5，原始的为1.0
	//s_touch.setRateChange(-50.0);
	s_touch.setPitch(0.1);
	s_touch.setSetting(SETTING_USE_QUICKSEEK, 1); 
	s_touch.setSetting(SETTING_USE_AA_FILTER, 1);

	soundtouch::SAMPLETYPE touch_buffer[96000]; 

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

			//////////////////////////////////////////////////////////////
			// 传入sample，并接收处理后的sample

			// 将解码后的buffer(uint8*)转换为soundtouch::SAMPLETYPE，也就是singed int 16
			auto len = nb * dst_channels * av_get_bytes_per_sample(dst_format);
			for (auto i = 0; i < len; i++)
			{
				touch_buffer[i] = (buffer[i * 2] | (buffer[i * 2 + 1] << 8));	
			}

			// 传入Sample
			s_touch.putSamples(touch_buffer, nb);
			do
			{
				// 接收处理后的sample
				nb = s_touch.receiveSamples(touch_buffer, 96000);

				auto length = nb * dst_channels * av_get_bytes_per_sample(dst_format);
				ofs.write((char*)touch_buffer, length);

				pcm_data_size += length;
			} while (nb != 0);
		}
		av_packet_unref(packet);
	}

	///////////////////////////////////////////////
	// 接收管道内余下的处理后数据
	s_touch.flush();
	int nSamples;
	do
	{
		nSamples = s_touch.receiveSamples(touch_buffer, 96000);

		auto length = nSamples * dst_channels * av_get_bytes_per_sample(dst_format);
		ofs.write((char*)touch_buffer, length);

		pcm_data_size += length;
	} while (nSamples != 0);

	// 写Wav文件头
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
	to_pcm();

	cout << "end." << endl;
	getchar();
	return 0;
}


