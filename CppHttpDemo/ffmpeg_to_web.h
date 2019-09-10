#pragma once
#include "all.h"
#include "xmlConfig.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil//time.h>
#include <libavutil/avconfig.h>
}

class ffmpeg_to_web
{
public:
	ffmpeg_to_web();
	~ffmpeg_to_web();

	std::string m_strFileName;	//input stream url or file
	std::string m_strUrl;				//out put stream url
	std::string m_fileExt;	//Transcode format suffix .flv .hls

	bool m_bSaveJPEG;
	int m_iPort;
	int videoStream;	//Video stream index

	bool m_bIsRunning;	//thread flag
	AVFormatContext* outputContext;
	AVFormatContext* context;
	//AVCodecContext* pAVCodecContext;

	//************************************
	// Method:    start
	// FullName:  ffmpeg_to_web::start
	// Access:    public 
	// Returns:   void
	// Qualifier: start main thread
	//************************************
	void start();
	//************************************
	// Method:    stop
	// FullName:  ffmpeg_to_web::stop
	// Access:    public 
	// Returns:   void
	// Qualifier:	exit thread
	//************************************
	void stop();

	//************************************
	// Method:    ffmpegInit
	// FullName:  ffmpeg_to_web::ffmpegInit
	// Access:    public 
	// Returns:   void
	// Qualifier: null
	//************************************
	void ffmpegInit();

	//************************************
	// Method:    openInputStream
	// FullName:  ffmpeg_to_web::openInputStream
	// Access:    public 
	// Returns:   int
	// Qualifier:	open ffmpeg input stream
	//************************************
	int openInputStream();

	//************************************
	// Method:    openOutputStream
	// FullName:  ffmpeg_to_web::openOutputStream
	// Access:    public 
	// Returns:   int
	// Qualifier: open output stream 
	//************************************
	int openOutputStream();

	//************************************
	// Method:    readPacketFromSource
	// FullName:  ffmpeg_to_web::readPacketFromSource
	// Access:    public 
	// Returns:   std::shared_ptr<AVPacket>
	// Qualifier: read one packet from source stream
	//************************************
	std::shared_ptr<AVPacket> readPacketFromSource();

	//************************************
	// Method:    writePacket
	// FullName:  ffmpeg_to_web::writePacket
	// Access:    public 
	// Returns:   int
	// Qualifier: write one packet to the output stream
	// Parameter: std::shared_ptr<AVPacket> packet
	//************************************
	int writePacket(std::shared_ptr<AVPacket> packet);

	//************************************
	// Method:    mainThread
	// FullName:  ffmpeg_to_web::mainThread
	// Access:    public 
	// Returns:   void
	// Qualifier: main thread
	//************************************
	void mainThread();

	//************************************
	// Method:    writeJPEG
	// FullName:  ffmpeg_to_web::writeJPEG
	// Access:    public 
	// Returns:   void
	// Qualifier:	Write JPEG information
	// Parameter: AVFrame * pFrame
	// Parameter: int width
	// Parameter: int height
	// Parameter: int iIndex
	//************************************
	void writeJPEG(AVFrame* pFrame, int width, int height);

	//************************************
	// Method:    ffmpegClose
	// FullName:  ffmpeg_to_web::ffmpegClose
	// Access:    public 
	// Returns:   void
	// Qualifier:	Destroy ffmpeg related variables
	//************************************
	void ffmpegClose();


	//************************************
	// Method:    interruptReadFrame
	// FullName:  ffmpeg_to_web::interruptReadFrame
	// Access:    public 
	// Returns:   int
	// Qualifier:	Threads need to interrupt when interrupting to continuously read video frames
	// Parameter: void* ctx
	//************************************
	static int interruptReadFrame(void* ctx);
};

