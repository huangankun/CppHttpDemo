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
	LOG(INFO) << " ffmpeg_to_web::mainThread start ";

	std::thread([&](ffmpeg_to_web *pointer)
	{
		pointer->mainThread();

	}, this).detach();
}

void ffmpeg_to_web::stop()
{
	m_bIsRunning = false;
}

void ffmpeg_to_web::ffmpegInit()
{
	LOG(INFO) << " ffmpeg_to_web::ffmpegInit initialization ";

	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_ERROR);
}

int ffmpeg_to_web::openInputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openInputStream Open the input stream to find the best stream";

	context = avformat_alloc_context();
	context->interrupt_callback.callback = interruptReadFrame;
	context->interrupt_callback.opaque = this;
	AVDictionary* dicts = NULL;
	AVInputFormat* ifmt = av_find_input_format("h264");
	AVCodec *pCodec;
	AVDictionary* format_opts = NULL;

	int ret = 0;
	//av_dict_set(&dicts, "c:v", "h264_cuvid", 0);
	//av_dict_set(&dicts, "hwaccel", "cuvid", 0);
	//av_dict_set(&dicts, "timeout", "10000", 0);
	//av_dict_set(&dicts, "rtbufsize", "655360", 0);
	//av_dict_set(&dicts, "bufsize", "655360", 0);

	ret = avformat_open_input(&context, m_strFileName.c_str(), nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_open_input failed，return code：" << errStr;
		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info Open the input stream successfully, find the best stream";

	ret = avformat_find_stream_info(context, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info failed，return code：" << errStr;
		return ret;
	}

	//AVCodec *pAVCodec;
	//pAVCodec = avcodec_find_decoder_by_name("h264_cuvid"); //查找n卡解码器
	//if (!pAVCodec) {
	//	LOG(INFO) << "find h264_cuvid failed";
	//	return -1;
	//}

	//pAVCodecContext = context->streams[0]->codec;

	//ret = avcodec_open2(pAVCodecContext, pAVCodec, nullptr);
	//if (ret < 0)
	//{
	//	char errStr[256];
	//	av_strerror(ret, errStr, 256);

	//	LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_write_header Write to output header failed， return code：" << errStr;

	//	return ret;
	//}

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_find_stream_info Success, the best stream has been found, waiting to open the output stream...";

	//av_dump_format(context, 0, m_strFileName.c_str(), 0);

	return ret;
}

int ffmpeg_to_web::openOutputStream()
{
	LOG(INFO) << " ffmpeg_to_web::openOutputStream Open the output stream, assign initialization";

	int ret = 0;
	ret = avformat_alloc_output_context2(&outputContext, nullptr, m_fileExt.c_str(), m_strUrl.c_str());
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_alloc_output_context2 Allocation initialization failed, return code：" << errStr;
		return ret;
	}
	
	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2 initialization，avformat_alloc_output_context2 Allocation initialization succeeded";

	ret = avio_open2(&outputContext->pb, m_strUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2 initialization failed， return code：" << errStr;

		return ret;
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream avio_open2 initialization succeeded，avcodec_copy_context Start decoder replication...";

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

			LOG(INFO) << " ffmpeg_to_web::openOutputStream avcodec_copy_context decoder replication failed， return code：" << errStr;

			return ret;
		}
	}

	LOG(INFO) << " ffmpeg_to_web::openOutputStream，Decoder copy completed，avformat_write_header, Start writing the output header...";

	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);

		LOG(INFO) << " ffmpeg_to_web::openOutputStream avformat_write_header Write to output header failed， return code：" << errStr;

		return ret;
	}


	//av_dump_format(outputContext, 0, m_strUrl.c_str(), 0);

	LOG(INFO) << " ffmpeg_to_web::openInputStream avformat_write_header succeeded，Sending packet to the server...";

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
		LOG(INFO) << " ffmpeg_to_web::openInputStream failed, Until find the best input stream";

		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		while (m_bIsRunning)
		{
			if (openInputStream() >= 0)
			{
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}



	if (openOutputStream() < 0)
	{
		LOG(INFO) << " ffmpeg_to_web::openOutputStream failed, Until find the best output stream";

		while (m_bIsRunning)
		{
			if (openOutputStream() >= 0)
			{
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
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
				//LOG(DEBUG) << " ffmpeg_to_web::writePacket 成功，已发送：" << packetCount << "帧";
			}
			else
			{
				//LOG(DEBUG) << " ffmpeg_to_web::writePacket 失败，第" << packetCount << "帧";
			}
		}
		else
		{
			//LOG(DEBUG) << " ffmpeg_to_web::readPacketFromSource 读取帧失败，第" << packetCount << "帧";
			return;
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
			LOG(INFO) << "Failed to create folder, folder path：" << fileName; 				  // 返回 0 表示创建成功，-1 表示失败
			return;
		}
	fileName = fileName + "\\" + xmlConfig::getCurrentTime() + ".jpg";

	// Assign an AVFormatContext object
	AVFormatContext* pFormatCtx = avformat_alloc_context();

	// Set the output file format
	pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
	// Create and initialize an AVIOContext associated with the url
	if (avio_open(&pFormatCtx->pb, fileName.c_str(), AVIO_FLAG_READ_WRITE) < 0) 
	{
		LOG(INFO) << "Open image path failed, image path" << fileName;
	}

	// Build a new stream
	AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
	if (pAVStream == NULL) 
	{
		LOG(INFO) << "Failed to create image output stream";
		return;
	}

	// Set the information of the stream
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

	// Find decoder
	AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		LOG(INFO) << "Find JPEG picture decoder failed, picture path"<< fileName;
	}
	// Set the decoder of pCodecCtx to pCodec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		LOG(INFO) << "Open JPEG picture decoder failed, picture path" << fileName;
	}

	//Write Header
	avformat_write_header(pFormatCtx, NULL);

	int y_size = pCodecCtx->width * pCodecCtx->height;

	//Encode
	// Allocate enough space for AVPacket
	AVPacket pkt;
	av_new_packet(&pkt, y_size * 3);

	// 
	int got_picture = 0;
	int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
	if (ret < 0) {
		LOG(INFO) << "Encoding JPEG image failed, image path" << fileName;
	}
	if (got_picture == 1) {
		//pkt.stream_index = pAVStream->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}

	av_free_packet(&pkt);

	//Write Trailer
	av_write_trailer(pFormatCtx);

	LOG(INFO) << "Store JPEG images successfully, image path" << fileName;

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
	avio_close(context->pb);
	avio_close(outputContext->pb);
	avformat_free_context(context);
	avformat_free_context(outputContext);
	LOG(INFO) << " ffmpeg_to_web::stop Call, the main thread stops";
}

int ffmpeg_to_web::interruptReadFrame(void* ctx)
{
	auto p = (ffmpeg_to_web *)ctx;
	if (!p->m_bIsRunning)
	{
		return AVERROR_EOF;
	}
	return 0;
}
