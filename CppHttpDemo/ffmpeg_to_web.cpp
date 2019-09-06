#include "ffmpeg_to_web.h"



ffmpeg_to_web::ffmpeg_to_web()
{
	m_bIsRunning = true;
	m_bSaveJPEG = false;
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

	//av_dump_format(context, 0, m_strFileName.c_str(), 0);

	return ret;
}

int ffmpeg_to_web::openOutputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openOutputStream 打开输出流，分配初始化";

	int ret = 0;
	ret = avformat_alloc_output_context2(&outputContext, nullptr, m_fileExt.c_str(), m_strUrl.c_str());
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
		if (context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
		}
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

	//av_dump_format(outputContext, 0, m_strUrl.c_str(), 0);

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
		//判断是否需要保存为本地文件
		if (m_bSaveJPEG)
		{
			AVFrame *pFrame;
			int got_picture;
			pFrame = av_frame_alloc();
			AVCodecContext *pCodecCtx;
			AVCodec *pCodec;
			pCodecCtx = context->streams[videoStream]->codec;
			pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
			if (pCodec == NULL) {
				LOG(INFO) << "avcode find decoder failed!\n";
				exit(1);
			}

			//打开解码器
			if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
				LOG(INFO) << "avcode open failed!\n";
				exit(1);
			}
			avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet.get());
			if (got_picture)
			{
				writeJPEG(pFrame, pCodecCtx->width, pCodecCtx->height);
			}
			av_freep(pFrame);
			//avcodec_close(pCodecCtx);
		}
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

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

		int packetCount = 0;
		while (m_bIsRunning)
		{
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

	ffmpegClose();
}

void ffmpeg_to_web::writeJPEG(AVFrame * pFrame, int width, int height)
{
	std::string fileName = xmlConfig::strWorkPath + "\\picture";
	if (0 != _access(fileName.c_str(), 0))
		if (0 != _mkdir(fileName.c_str()))
		{
			LOG(INFO) << "创建文件夹失败，文件夹路径：" << fileName; 				  // 返回 0 表示创建成功，-1 表示失败
			return;
		}
	fileName = fileName + "\\" + xmlConfig::getCurrentTime() + ".jpg";

	// 分配AVFormatContext对象
	AVFormatContext* pFormatCtx = avformat_alloc_context();

	// 设置输出文件格式
	pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
	// 创建并初始化一个和该url相关的AVIOContext
	if (avio_open(&pFormatCtx->pb, fileName.c_str(), AVIO_FLAG_READ_WRITE) < 0) 
	{
		LOG(INFO) << "打开图片路径失败，图片路径" << fileName;
	}

	// 构建一个新stream
	AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
	if (pAVStream == NULL) 
	{
		LOG(INFO) << "创建图片输出流失败";
		return;
	}

	// 设置该stream的信息
	AVCodecContext* pCodecCtx = pAVStream->codec;

	pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	// Begin Output some information
	av_dump_format(pFormatCtx, 0, fileName.c_str(), 1);
	// End Output some information

	// 查找解码器
	AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		LOG(INFO) << "查找JPEG图片解码器失败，图片路径"<< fileName;
	}
	// 设置pCodecCtx的解码器为pCodec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		LOG(INFO) << "打开JPEG图片解码器失败，图片路径" << fileName;
	}

	//Write Header
	avformat_write_header(pFormatCtx, NULL);

	int y_size = pCodecCtx->width * pCodecCtx->height;

	//Encode
	// 给AVPacket分配足够大的空间
	AVPacket pkt;
	av_new_packet(&pkt, y_size * 3);

	// 
	int got_picture = 0;
	int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
	if (ret < 0) {
		LOG(INFO) << "编码JPEG图片失败，图片路径" << fileName;
	}
	if (got_picture == 1) {
		//pkt.stream_index = pAVStream->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}

	av_free_packet(&pkt);

	//Write Trailer
	av_write_trailer(pFormatCtx);

	LOG(INFO) << "存储JPEG图片成功，图片路径" << fileName;

	if (pAVStream) 
	{
		avcodec_close(pAVStream->codec);
	}

	avio_close(pFormatCtx->pb);

	avformat_free_context(pFormatCtx);

	m_bSaveJPEG = false;
}

void ffmpeg_to_web::ffmpegClose()
{
	avformat_free_context(context);
	avformat_free_context(outputContext);
}
