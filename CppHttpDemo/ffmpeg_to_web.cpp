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
	LOG(INFO) << " ffmpeg_to_web::mainThread 主线程开始";

	std::thread([&](ffmpeg_to_web *pointer)
	{
		pointer->mainThread();

	}, this).detach();
}

void ffmpeg_to_web::stop()
{
	LOG(INFO) << " ffmpeg_to_web::stop 调用，主线程停止";

	while (m_bIsRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void ffmpeg_to_web::ffmpegInit()
{
	LOG(INFO) << " ffmpeg_to_web::ffmpegInit 初始化";

	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_ERROR);
}

int ffmpeg_to_web::openInputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openInputStream 打开输入流，查找最佳流";

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
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_open_input失败，错误：" << errStr;
		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info 打开输入流成功，查找最佳流";

	ret = avformat_find_stream_info(context, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info失败，错误：" << errStr;
		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info成功，已找到最佳流，等待打开输出流...";

	av_dump_format(context, 0, m_strFileName.c_str(), 0);

	return ret;
}

int ffmpeg_to_web::openOutputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openOutputStream 打开输出流，分配初始化";

	int ret = 0;
	ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", m_strUrl.c_str());
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_alloc_output_context2分配初始化失败，错误：" << errStr;
		return ret;
	}
	
	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2初始化，avformat_alloc_output_context2分配初始化成功";

	ret = avio_open2(&outputContext->pb, m_strUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2初始化失败，错误：" << errStr;

		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2初始化成功，avcodec_copy_context开始进行解码器复制...";

	for (int i = 0; i < context->nb_streams; i++)
	{
		AVStream* stream = avformat_new_stream(outputContext, nullptr);
		ret = avcodec_copy_context(stream->codec, context->streams[i]->codec);
		if (ret < 0)
		{
			char errStr[256];
			av_strerror(ret, errStr, 256);

			LOG(INFO) << " ffmpeg_to_web::openOutputStream avcodec_copy_context 解码器复制失败，错误：" << errStr;

			return ret;
		}
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream，解码器复制完成，avformat_write_header开始写输出头...";

	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_write_header 写入输出头失败，错误：" << errStr;

		return ret;
	}

	av_dump_format(outputContext, 0, m_strUrl.c_str(), 0);

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_write_header成功，正在准备发送流数据到服务器...";

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
		LOG(INFO) << " ffmpeg_to_web::openInputStream failed, 直到找到最佳输入流";

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
		LOG(INFO) << " ffmpeg_to_web::openInputStream failed, 直到找到最佳输入流";

		if (openOutputStream() < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));

			LOG(INFO) << " ffmpeg_to_web::openOutputStream failed, 线程退出";

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
					LOG(DEBUG) << " ffmpeg_to_web::writePacket 成功，已发送：" << packetCount << "帧";
				}
				else
				{
					LOG(DEBUG) << " ffmpeg_to_web::writePacket 失败，第" << packetCount << "帧";
				}
			}
			else
			{
				LOG(DEBUG) << " ffmpeg_to_web::readPacketFromSource 读取帧失败，第" << packetCount << "帧";
				return;
			}
		}
	}
}
