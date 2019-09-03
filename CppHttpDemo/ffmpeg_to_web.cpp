#include "ffmpeg_to_web.h"



ffmpeg_to_web::ffmpeg_to_web()
{
	m_bIsRunning = true;
}


ffmpeg_to_web::~ffmpeg_to_web()
{

}

void ffmpeg_to_web::start()
{
	LOG(INFO) << " ffmpeg_to_web::mainThread ���߳̿�ʼ";

	std::thread([&](ffmpeg_to_web *pointer)
	{
		pointer->mainThread();

	}, this).detach();
}

void ffmpeg_to_web::stop()
{
	LOG(INFO) << " ffmpeg_to_web::stop ���ã����߳�ֹͣ";

	while (m_bIsRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void ffmpeg_to_web::ffmpegInit()
{
	LOG(INFO) << " ffmpeg_to_web::ffmpegInit ��ʼ��";

	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_ERROR);
}

int ffmpeg_to_web::openInputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openInputStream �������������������";

	context = avformat_alloc_context();
	AVDictionary* dicts = NULL;
	AVInputFormat* ifmt = av_find_input_format("h264");
	AVCodec *pCodec;
	AVDictionary* format_opts = NULL;

	int ret = 0;
	av_dict_set(&dicts, "protocol_whitelist", "file,udp", 0);
	av_dict_set(&dicts, "timeout", "10000000", 0);
	av_dict_set(&dicts, "rtbufsize", "655360", 0);
	av_dict_set(&dicts, "bufsize", "655360", 0);

	ret = avformat_open_input(&context, m_strFileName.c_str(), nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_open_inputʧ�ܣ�����" << errStr;
		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info ���������ɹ������������";

	ret = avformat_find_stream_info(context, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_infoʧ�ܣ�����" << errStr;
		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info�ɹ������ҵ���������ȴ��������...";

	av_dump_format(context, 0, m_strFileName.c_str(), 0);

	return ret;
}

int ffmpeg_to_web::openOutputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openOutputStream ��������������ʼ��";

	int ret = 0;
	ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", m_strUrl.c_str());
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_alloc_output_context2�����ʼ��ʧ�ܣ�����" << errStr;
		return ret;
	}
	
	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2��ʼ����avformat_alloc_output_context2�����ʼ���ɹ�";

	ret = avio_open2(&outputContext->pb, m_strUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2��ʼ��ʧ�ܣ�����" << errStr;

		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2��ʼ���ɹ���avcodec_copy_context��ʼ���н���������...";

	for (int i = 0; i < context->nb_streams; i++)
	{
		AVStream* stream = avformat_new_stream(outputContext, nullptr);
		ret = avcodec_copy_context(stream->codec, context->streams[i]->codec);
		if (ret < 0)
		{
			char errStr[256];
			av_strerror(ret, errStr, 256);

			LOG(INFO) << " ffmpeg_to_web::openOutputStream avcodec_copy_context ����������ʧ�ܣ�����" << errStr;

			return ret;
		}
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream��������������ɣ�avformat_write_header��ʼд���ͷ...";

	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_write_header д�����ͷʧ�ܣ�����" << errStr;

		return ret;
	}

	av_dump_format(outputContext, 0, m_strUrl.c_str(), 0);

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_write_header�ɹ�������׼�����������ݵ�������...";

	return ret;
}

std::shared_ptr<AVPacket> ffmpeg_to_web::readPacketFromSource()
{
	std::shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket *p) { av_free_packet(p); av_freep(&p); });
	av_init_packet(packet.get());
	int ret = av_read_frame(context, packet.get());
	if (ret >= 0)
	{
		return packet;
	}
	else
	{
		return nullptr;
	}
}

int ffmpeg_to_web::writePacket(std::shared_ptr<AVPacket> packet)
{
	auto inputStream = context->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
	return av_interleaved_write_frame(outputContext, packet.get());
}

void ffmpeg_to_web::mainThread()
{

	LOG(INFO) << " ffmpeg_to_web::mainThread Start";

	ffmpegInit();

	if (openInputStream() < 0)
	{
		LOG(INFO) << " ffmpeg_to_web::openInputStream failed, ֱ���ҵ����������";

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		while (true)
		{
			if (openInputStream() > 0)
			{
				break;
			}
		}
	}
	else
	{
		LOG(INFO) << " ffmpeg_to_web::openInputStream failed, ֱ���ҵ����������";

		if (openOutputStream() < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));

			LOG(INFO) << " ffmpeg_to_web::openOutputStream failed, �߳��˳�";

			return;
		}


		while (m_bIsRunning)
		{
			int packetCount = 0;
			std::shared_ptr<AVPacket> packet = nullptr;
			packet = readPacketFromSource();
			if (packet)
			{
				packetCount++;
				if (writePacket(packet) >= 0)
				{
					LOG(DEBUG) << " ffmpeg_to_web::writePacket �ɹ����ѷ��ͣ�" << packetCount << "֡";
				}
				else
				{
					LOG(DEBUG) << " ffmpeg_to_web::writePacket ʧ�ܣ���" << packetCount << "֡";
				}
			}
			else
			{
				LOG(DEBUG) << " ffmpeg_to_web::readPacketFromSource ��ȡ֡ʧ�ܣ���" << packetCount << "֡";
				return;
			}
		}
	}
}