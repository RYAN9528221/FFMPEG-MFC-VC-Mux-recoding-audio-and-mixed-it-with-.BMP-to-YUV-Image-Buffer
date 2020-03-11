﻿
// FFRecordPlayDlg.cpp: 實作檔案
//

#include "pch.h"
#include "framework.h"
#include "FF_GAME.h"
#include "FF_GAMEDlg.h"
#include "afxdialogex.h"
#include<opencv.hpp>
#include<cv.h>
#include<highgui.h>
#include<iostream>
using namespace std;
using namespace cv;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <process.h>
//#define USE_FILTER
// TODO:  婓森揭竘蚚最唗剒猁腔坻芛恅璃
#include <Windows.h>
#pragma comment(lib, "Winmm.lib")

#include <dshow.h>
#pragma comment(lib, "Strmiids.lib")

#define __STDC_CONSTANT_MACROS
using namespace cv;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
double SampleRate11025(11025);
bool b_Global_Stop;
#define STREAM_DURATION   18//大約19秒
#define STREAM_FRAME_RATE 30 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC
#define av_ts2str(ts) av_ts_make_string((char[AV_TS_MAX_STRING_SIZE]){0}, ts)
// a wrapper around a single output AVStream
AVFormatContext* video_ctx = NULL;//....................新增變數
AVFormatContext* audio_ctx = NULL;//....................新增變數
int video_index(0);
int audio_index(0);
//====================
//插入式廣告
Mat frameAdvertising;
VideoCapture cap;
CWnd* pDesktop;
//====================
typedef struct OutputStream
{
	AVStream* st;
	AVCodecContext* enc;

	/* pts of the next frame that will be generated */
	int64_t next_pts;
	int samples_count;

	AVFrame* frame;
	AVFrame* tmp_frame;

	float t, tincr, tincr2;

	struct SwsContext* sws_ctx;//video
	struct SwrContext* swr_ctx;//audio
} OutputStream;
//錄音:
//==============================================================
bool b_OpenOK(false);
#define INP_BUFFER_SIZE	441 //1764=SampleRate11025/25
BYTE	lpAudioData[25 * INP_BUFFER_SIZE];//錄音塞到FFMPEG的空間
DWORD		nTimeBase;
DWORD		nLastTick;
//==============================================================
//PictureBox CV畫布:
CRect rect;
CDC* pDC;
void cvShowIMG(Mat m_cvImg, CRect rect, CDC* pDC)
{
	cv::Mat img;
	cv::Rect dst(rect.left, rect.top, rect.right, rect.bottom);
	if (m_cvImg.data && rect.Width() > 0)
	{
		cv::resize(m_cvImg, img, cv::Size(rect.Width(), rect.Height())); //使图像适应控件大小 
		unsigned int m_buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		BITMAPINFO* m_bmi = (BITMAPINFO*)m_buffer;
		BITMAPINFOHEADER* m_bmih = &(m_bmi->bmiHeader);
		memset(m_bmih, 0, sizeof(*m_bmih));
		m_bmih->biSize = sizeof(BITMAPINFOHEADER);
		m_bmih->biWidth = img.cols; //必须为4的倍数
		m_bmih->biHeight = -img.rows; //在自下而上的位图中 高度为负
		m_bmih->biPlanes = 1;
		m_bmih->biCompression = BI_RGB;
		m_bmih->biBitCount = 8 * img.channels();
		if (img.channels() == 1)  //当图像为灰度图像时需要设置调色板颜色
		{
			for (int i = 0; i < 256; i++)
			{
				m_bmi->bmiColors[i].rgbBlue = i;
				m_bmi->bmiColors[i].rgbGreen = i;
				m_bmi->bmiColors[i].rgbRed = i;
				m_bmi->bmiColors[i].rgbReserved = 0;
			}
		}
		::StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, rect.Width(), rect.Height(), img.data, (BITMAPINFO*)m_bmi, DIB_RGB_COLORS, SRCCOPY);
	}
}
//==============================================================

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFFRecordPlayDlg 對話方塊



CFFGAMEDlg::CFFGAMEDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FF_GAME_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFFGAMEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFFGAMEDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Record, &CFFGAMEDlg::OnBnClickedRecord)
END_MESSAGE_MAP()


// CFFRecordPlayDlg 訊息處理常式

// CFFRecordPlayDlg 訊息處理常式

BOOL CFFGAMEDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
	b_OpenOK = false;
	OnBnClickedRecordAudio();//錄音

	//================================================================================================================
	//Windows Picture Box:
	GetDlgItem(IDC_IMG)->GetClientRect(&rect);
	if (rect.Width() % 4 != 0)
	{
		rect.SetRect(rect.left, rect.top, rect.left + (rect.Width() + 3) / 4 * 4, rect.bottom); //调整图像宽度为4的倍数
		GetDlgItem(IDC_IMG)->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOMOVE);
	}
	pDC = GetDlgItem(IDC_IMG)->GetDC();
	//================================================================================================================
	b_Global_Stop = false;

	pDesktop = GetDesktopWindow();

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CFFGAMEDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CFFGAMEDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CFFGAMEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


DWORD CALLBACK CFFGAMEDlg::MicCallback(HWAVEIN hwavein, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	//这个CIMAADPCMDlg就是你的音频采集类
	CFFGAMEDlg* pWnd = (CFFGAMEDlg*)dwInstance;
	switch (uMsg)
	{
	case WIM_OPEN:
		TRACE("WIM_OPEN\n");
		break;
	case WIM_DATA:
		TRACE("WIM_DATA\n");
		if (pWnd->f != NULL && b_OpenOK)
		{

			static int n = 0;

			n++;
			if (((PWAVEHDR)dwParam1)->dwBytesRecorded != INP_BUFFER_SIZE)
			{
				n = 0;
				printf("Invalid Audio Buffer");
			}
			if (n > 25)
			{
				for (int i = 0; i < 24; i++)
				{
					CopyMemory(lpAudioData + INP_BUFFER_SIZE * i, lpAudioData + INP_BUFFER_SIZE * (i + 1), INP_BUFFER_SIZE);
				}
				n = 25;
				nTimeBase = GetTickCount() - 1000;
			}
			if (n == 1)
			{
				nTimeBase = GetTickCount();
			}
			CopyMemory(lpAudioData + (n - 1) * INP_BUFFER_SIZE, ((PWAVEHDR)dwParam1)->lpData, ((PWAVEHDR)dwParam1)->dwBytesRecorded);
			waveInAddBuffer(hwavein, (PWAVEHDR)dwParam1, sizeof(WAVEHDR));


			/*
			//采集到的数据做处理
			//((PWAVEHDR)dwParam1)->lpData是采集到的数据指针
			//((PWAVEHDR)dwParam1)->dwBytesRecorded是采集到的数据长度
			//将采集到的数据写入文件中
			fwrite(((PWAVEHDR)dwParam1)->lpData, ((PWAVEHDR)dwParam1)->dwBytesRecorded, 1, pWnd->f);
			//处理完了之后还要再把这个缓冲数组加回去
			//pWnd->win代表是否继续采集，因为当停止采集的时候，只有当所有的
			//缓冲数组都腾出来之后才能close，所以需要停止采集时就不要再waveInAddBuffer了
			if (pWnd->win)
				waveInAddBuffer(hwavein, (PWAVEHDR)dwParam1, sizeof(WAVEHDR));
			*/
		}
		break;
	case WIM_CLOSE:
		TRACE("WIM_CLOSE\n");
		break;
	default:
		break;
	}
	return 0;
}



void CFFGAMEDlg::OnBnClickedRecordAudio()
{
	//fopen_s(&f, "audio.pcm", "wb");//打开录音文件
	win = true;
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM
	waveform.nSamplesPerSec = SampleRate11025;//采样率，16000次/秒
	waveform.wBitsPerSample = 8;//采样比特，16bits/次
	waveform.nChannels = 1;//采样声道数，1声道
	waveform.nAvgBytesPerSec = SampleRate11025;//每秒的数据率，就是每秒能采集多少字节的数据
	waveform.nBlockAlign = 1;//一个块的大小，采样bit的字节数乘以声道数
	waveform.cbSize = 0;//一般为0
	//使用waveInOpen函数开启音频采集
	//只能支持32位元
	MMRESULT mmr = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD)(MicCallback), DWORD(this), CALLBACK_FUNCTION);
	if (mmr != MMSYSERR_NOERROR)
	{
		b_OpenOK = false;
		//CB_Record.EnableWindow(TRUE);
		return;
	}
	SetWindowText("錄音中...");
	b_OpenOK = true;
	//建立两个数组（这里可以建立多个数组）用来缓冲音频数据
	//DWORD bufsize =441;
	pBuffer1 = new BYTE[INP_BUFFER_SIZE];
	pBuffer2 = new BYTE[INP_BUFFER_SIZE];
	wHdr1.lpData = (LPSTR)pBuffer1;
	wHdr1.dwBufferLength = INP_BUFFER_SIZE;
	wHdr1.dwBytesRecorded = 0;
	wHdr1.dwUser = 0;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1;
	wHdr1.lpNext = NULL;
	wHdr1.reserved = 0;
	//将建立好的wHdr1做为备用
	waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	wHdr2.lpData = (LPSTR)pBuffer2;
	wHdr2.dwBufferLength = INP_BUFFER_SIZE;
	wHdr2.dwBytesRecorded = 0;
	wHdr2.dwUser = 0;
	wHdr2.dwFlags = 0;
	wHdr2.dwLoops = 1;
	wHdr2.lpNext = NULL;
	wHdr2.reserved = 0;
	//将建立好的wHdr2做为备用
	waveInPrepareHeader(hWaveIn, &wHdr2, sizeof(WAVEHDR));
	//将两个wHdr添加到waveIn中去
	waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wHdr2, sizeof(WAVEHDR));
	//开始音频采集
	waveInStart(hWaveIn);
}


void CFFGAMEDlg::Stop()
{
	win = false;
	Sleep(2000);
	//在这里可以等待一定的时间，确保所有缓冲数组全都退出来了
	//也可以在WIM_DATA消息处理中加个计数器，计算全部buffer都
	//退出来了，就可以调用下面的close了
	/*
	if (f)
	{
		fclose(f);
	}
	*/
	waveInClose(hWaveIn);
	SetWindowText("已停止錄音");
}





static void log_packet(const AVFormatContext* fmt_ctx, const AVPacket* pkt)
{
	AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
	/*
	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
		av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
		av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),

		pkt->stream_index);
		*/
}

static int write_frame(AVFormatContext* fmt_ctx, const AVRational* time_base, AVStream* st, AVPacket* pkt)
{
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, st->time_base);
	pkt->stream_index = st->index;

	/* Write the compressed frame to the media file. */
	log_packet(fmt_ctx, pkt);
	return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
static void add_stream(OutputStream* ost, AVFormatContext* oc, AVCodec** codec, enum AVCodecID codec_id)
{
	AVCodecContext* c;
	int i;

	//設定編碼器
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec))
	{
		fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(codec_id));
		exit(1);
	}

	ost->st = avformat_new_stream(oc, NULL);
	if (!ost->st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	ost->st->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}
	ost->enc = c;

	switch ((*codec)->type)
	{
	case AVMEDIA_TYPE_AUDIO:
		c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 64000;
		c->sample_rate = 44100;
		if ((*codec)->supported_samplerates)
		{
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++)
			{
				if ((*codec)->supported_samplerates[i] == 44100)
					c->sample_rate = 44100;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((*codec)->channel_layouts)
		{
			c->channel_layout = (*codec)->channel_layouts[0];
			for (i = 0; (*codec)->channel_layouts[i]; i++)
			{
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		ost->st->time_base = AVRational{ 1, c->sample_rate };
		break;

	case AVMEDIA_TYPE_VIDEO:
		c->codec_id = codec_id;
		c->bit_rate = 400000;
		//解析度設定(必需為2的倍數)
		c->width = 1920;
		c->height = 1080;
		// timebase: 這是基本時間單位（以秒為單位）表示其中的幀時間戳。 對於固定fps內容
		//時基應為1 / framerate，時間戳增量應為1
		ost->st->time_base = AVRational{ 1, STREAM_FRAME_RATE };
		c->time_base = ost->st->time_base;

		c->gop_size = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = STREAM_PIX_FMT;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
		{
			/* just for testing, we also add B-frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
		{
			//需要避免使用其中一些係數溢出的宏塊。
			//普通視頻不會發生這種情況，因為
			//色度平面的運動與亮度平面不匹配。
			c->mb_decision = 2;
		}



		break;

	default:
		break;
	}

	//某些格式希望流頭分開
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
	{
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	return;
}

/**************************************************************/
/* audio output */

static AVFrame* alloc_audio_frame(enum AVSampleFormat sample_fmt,
	uint64_t channel_layout,
	int sample_rate, int nb_samples)
{
	AVFrame* frame = av_frame_alloc();
	int ret;

	if (!frame)
	{
		fprintf(stderr, "Error allocating an audio frame\n");
		exit(1);
	}

	frame->format = sample_fmt;
	frame->channel_layout = channel_layout;
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;

	if (nb_samples)
	{
		ret = av_frame_get_buffer(frame, 0);
		if (ret < 0)
		{
			fprintf(stderr, "Error allocating an audio buffer\n");
			exit(1);
		}
	}

	return frame;
}

static void open_audio(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg)
{
	AVCodecContext* c;
	int nb_samples;
	int ret;
	AVDictionary* opt = NULL;

	c = ost->enc;

	/* open it */
	av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0)
	{
		//fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
		exit(1);
	}

	/* init signal generator */
	ost->t = 0;
	ost->tincr = 2 * M_PI * 220 / c->sample_rate;
	/* increment frequency by 110 Hz per second */
	ost->tincr2 = 2 * M_PI * 220 / c->sample_rate / c->sample_rate;

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout,
		c->sample_rate, nb_samples);
	ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
		c->sample_rate, nb_samples);

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}

	/* create resampler context */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx)
	{
		fprintf(stderr, "Could not allocate resampler context\n");
		exit(1);
	}

	av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	//av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
	av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

	///初始化重採樣編碼器
	if ((ret = swr_init(ost->swr_ctx)) < 0)
	{
		fprintf(stderr, "Failed to initialize the resampling context\n");
		exit(1);
	}
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */


static AVFrame* get_audio_frame(OutputStream* ost, DWORD& lastTick)
{

	AVFrame* frame = ost->tmp_frame;
	int j, i;
	int16_t* q = (int16_t*)frame->data[0];
	AVRational avr;
	avr.num = 1;
	avr.den = 1;

	/* check if we want to generate more frames */
	//if (av_compare_ts(ost->next_pts, ost->enc->time_base,
	//	STREAM_DURATION, avr) >= 0)
	//	return NULL;
	if (b_Global_Stop) //如果停止
	{
		return NULL;
	}
	if (av_compare_ts(ost->next_pts, ost->enc->time_base, STREAM_DURATION, AVRational{ 1, 1 }) > 0)
	{
		return NULL;
	}

	if (lastTick < nTimeBase)
		lastTick = nTimeBase;

	DWORD curTick = lastTick - nTimeBase;
	int curPos = curTick * SampleRate11025 / 1000;
	if (curPos + frame->nb_samples < INP_BUFFER_SIZE * 25)
	{
		for (j = 0; j < frame->nb_samples; j++)
		{
			//int v = (int)(sin(ost->t) * 10000);
			q[ost->enc->channels * j] = lpAudioData[curPos + j] * 500;
			for (i = 1; i < ost->enc->channels; i++)
			{
				q[ost->enc->channels * j + i] = lpAudioData[curPos + j] * 500;
			}
			ost->t += ost->tincr;
			ost->tincr += ost->tincr2;
		}
		frame->pts = ost->next_pts;
		ost->next_pts += 1024;// frame->nb_samples;
	}
	else
	{
		return NULL;
	}
	return frame;
}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
static int write_audio_frame(AVFormatContext* oc, OutputStream* ost, DWORD& lastTick)
{
	AVCodecContext* c;
	AVPacket pkt = { 0 }; // data and size must be 0;
	AVFrame* frame;
	int ret;
	int got_packet;
	int dst_nb_samples;

	av_init_packet(&pkt);
	c = ost->enc;

	frame = get_audio_frame(ost, lastTick);


	if (frame)
	{
		//使用重採樣器將樣本從本機格式轉換為目標編解碼器格式
		dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples, c->sample_rate, c->sample_rate, AV_ROUND_UP);
		av_assert0(dst_nb_samples == frame->nb_samples);

		/* when we pass a frame to the encoder, it may keep a reference to it
		 * internally;
		 * make sure we do not overwrite it here
		 */
		ret = av_frame_make_writable(ost->frame);
		if (ret < 0)
			exit(1);

		/* convert to destination format */
		ret = swr_convert(ost->swr_ctx, ost->frame->data, dst_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);

		if (ret < 0)
		{
			fprintf(stderr, "Error while converting\n");
			exit(1);
		}
		frame = ost->frame;

		frame->pts = av_rescale_q(ost->samples_count, AVRational{ 1, c->sample_rate }, c->time_base);
		ost->samples_count += dst_nb_samples;
	}

	ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
	if (ret < 0)
	{
		//fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
		exit(1);
	}

	if (got_packet)
	{
		ret = write_frame(oc, &c->time_base, ost->st, &pkt);
		if (ret < 0)
		{
			//fprintf(stderr, "Error while writing audio frame: %s\n",av_err2str(ret));
			exit(1);
		}
	}

	return (frame || got_packet) ? 0 : 1;
}

/**************************************************************/
/* video output */

static AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
	AVFrame* picture;
	int ret;

	picture = av_frame_alloc();
	if (!picture)
		return NULL;

	picture->format = pix_fmt;
	picture->width = width;
	picture->height = height;

	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		exit(1);
	}

	return picture;
}

static void open_video(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg)
{
	int ret;
	AVCodecContext* c = ost->enc;
	AVDictionary* opt = NULL;

	av_dict_copy(&opt, opt_arg, 0);

	/* open the codec */
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0)
	{
		//fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
		exit(1);
	}

	/* allocate and init a re-usable frame */
	ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
	if (!ost->frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	/* If the output format is not YUV420P, then a temporary YUV420P
	 * picture is needed too. It is then converted to the required
	 * output format. */
	ost->tmp_frame = NULL;
	if (c->pix_fmt != AV_PIX_FMT_YUV420P)
	{
		ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
		if (!ost->tmp_frame)
		{
			fprintf(stderr, "Could not allocate temporary picture\n");
			exit(1);
		}
	}

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0)
	{
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}
}

/* Prepare a dummy image. */
static void fill_yuv_image(AVFrame* pict, int frame_index, int width, int height)
{
	int x, y, i;

	i = frame_index;

	/* Y */
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
	/*
	// Cb and Cr
	for (y = 0; y < height / 2; y++)
	{
		for (x = 0; x < width / 2; x++)
		{
			pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
			pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
		}
	}
	*/
}

Mat hwnd2mat(HWND hwnd)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
	width = windowsize.right / 1;

	src.create(height, width, CV_8UC3);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}


static AVFrame* get_video_frame(OutputStream* ost)
{
	AVCodecContext* c_ctx = ost->enc;

	//判斷是否還要繼續產生影片
	if (av_compare_ts(ost->next_pts, c_ctx->time_base, STREAM_DURATION, AVRational{ 1, 1 }) > 0)
	{
		return NULL;
	}
	/* when we pass a frame to the encoder, it may keep a reference to it
	 * internally; make sure we do not overwrite it here */
	if (av_frame_make_writable(ost->frame) < 0)
		exit(1);

	if (c_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
	{
		/* as we only generate a YUV420P picture, we must convert it
		 * to the codec pixel format if needed */
		if (!ost->sws_ctx)
		{
			ost->sws_ctx = sws_getContext(c_ctx->width, c_ctx->height,
				AV_PIX_FMT_YUV420P,
				c_ctx->width, c_ctx->height,
				c_ctx->pix_fmt,
				SCALE_FLAGS, NULL, NULL, NULL);

			if (!ost->sws_ctx)
			{
				fprintf(stderr, "Could not initialize the conversion context\n");
				exit(1);
			}
		}
		fill_yuv_image(ost->tmp_frame, ost->next_pts, c_ctx->width, c_ctx->height);


		sws_scale(ost->sws_ctx, (const uint8_t* const*)ost->tmp_frame->data, ost->tmp_frame->linesize, 0, c_ctx->height, ost->frame->data, ost->frame->linesize);
	}
	else
	{
		//fill_yuv_image(ost->frame, ost->next_pts, c_ctx->width, c_ctx->height);

		AVFrame* pict = ost->frame;
		int x, y, i;

		i = ost->next_pts;

		Mat img = imread("lego.bmp");
		int nChannels = img.channels();
		int stepWidth = img.step;
		uchar* pData = img.ptr(0);

		/*
		HWND hwndDesktop = GetDesktopWindow();
		Mat img = hwnd2mat(hwndDesktop).clone();
		int nChannels = img.channels();
		int stepWidth = img.step;
		uchar* pData = img.ptr(0);
		*/

		cap.read(frameAdvertising);//廣告
		if (frameAdvertising.data) {
			cv::resize(frameAdvertising, frameAdvertising, cv::Size(418, 544));
			frameAdvertising.copyTo(img(cv::Rect(840, 130, frameAdvertising.cols, frameAdvertising.rows)));
		}

		cvShowIMG(img, rect, pDC);

		/* Y */
		int index;
		for (int row = 0; row < pict->height; row++)
		{
			for (int col = 0; col < pict->width; col++)
			{
				double R = (*(pData + row * stepWidth + col * nChannels + 2));
				double G = (*(pData + row * stepWidth + col * nChannels + 1));
				double B = (*(pData + row * stepWidth + col * nChannels));

				pict->data[0][row * pict->linesize[0] + col] = 0.257 * R + 0.504 * G + 0.098 * B + 16;
			}
		}


		// Cb and Cr
		for (int row = 0; row < pict->height / 2; row++)
		{
			for (int col = 0; col < pict->width / 2; col++)
			{
				double R = (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 2));
				double G = (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 1));
				double B = (*(pData + row * stepWidth * 2 + col * nChannels * 2));
				double Y = 0.257 * R + 0.504 * G + 0.098 * B + 16;

				pict->data[1][row * pict->linesize[1] + col] = -0.148 * R - 0.291 * G + 0.439 * B + 128;
				pict->data[2][row * pict->linesize[2] + col] = 0.439 * R - 0.368 * G - 0.071 * B + 128;
			}
		}

		/*
		// Y
		for (int row = 0; row < pict->height; row++)
			for (int col = 0; col < pict->width; col++)
				pict->data[0][row * pict->linesize[0] + col] =
				0.299 * (*(pData + row * stepWidth + col * nChannels + 2)) +
				0.587 * (*(pData + row * stepWidth + col * nChannels + 1)) +
				0.114 * (*(pData + row * stepWidth + col * nChannels + 0));
		//CB
		for (int row = 0; row < pict->height/2; row++)
			for (int col = 0; col < pict->width/2; col++)
				pict->data[1][row * pict->linesize[1] + col] =
				-0.168 * (*(pData + row * stepWidth*2 + col * nChannels * 2 + 2)) +
				-0.3316 * (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 1)) +
				0.5 * (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 0));
		//CR
		for (int row = 0; row < pict->height/2; row++)
			for (int col = 0; col < pict->width/2; col++)
				pict->data[2][row * pict->linesize[2] + col] =
				0.4 * (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 2)) +
				-0.4 * (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 1)) +
				-0.08 * (*(pData + row * stepWidth * 2 + col * nChannels * 2 + 0));
		*/
		/*
		//Cb and Cr
		for (int row = 0; row < height / 2; y++)
		{
			for (x = 0; x < width / 2; x++)
			{
				pict->data[1][y * pict->linesize[1] + x] = *(pData + row * stepWidth + col * nChannels + 0);
				pict->data[2][y * pict->linesize[2] + x] = *(pData + row * stepWidth + col * nChannels + 0);
			}
		}
		*/

		struct SwsContext* img_convert_ctx2 = NULL;
		int stream_index(0);
		int w = pict->width;
		int h = pict->height;
		img_convert_ctx2 = sws_getContext(w, h, AV_PIX_FMT_RGB24, w, h, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

		//sws_scale(img_convert_ctx2, pict->data, pict->linesize, 0, pict->height, pict->data, pict->linesize);

	}

	ost->frame->pts = ost->next_pts++;

	return ost->frame;
}

//===================================================================
//編碼一個視頻幀並將其發送到多路復用器
//編碼完成後返回1，否則返回0
static int write_video_frame(AVFormatContext* oc, OutputStream* ost)
{
	int ret;
	AVCodecContext* c;
	AVFrame* frame;
	int got_packet = 0;
	AVPacket pkt = { 0 };

	c = ost->enc;

	frame = get_video_frame(ost);//影片擷取竟然跟寫入放在一起......

	av_init_packet(&pkt);
	//================================================================
	//開始編碼到輸出Packet
	ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
	if (ret < 0)
	{
		printf("Error encoding video frame");
		//(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
		exit(1);
	}
	if (got_packet)
	{
		ret = write_frame(oc, &c->time_base, ost->st, &pkt);
	}
	else
	{
		ret = 0;
	}

	if (ret < 0)
	{
		printf("Error while writing video frame");
		//fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));
		exit(1);
	}

	return (frame || got_packet) ? 0 : 1;
}

static void close_stream(AVFormatContext* oc, OutputStream* ost)
{
	avcodec_free_context(&ost->enc);
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}




static int open_input_file(const char* filename) {

	return 0;
}

Mat WaterData(AVFrame* pFrame, int W, int H)
{
	int		nChannels;
	int		stepWidth;
	int width = W;
	int height = H;
	Mat frameImage = Mat::ones(cv::Size(W, H), CV_8UC3);
	uchar* pData = frameImage.ptr(0);
	stepWidth = frameImage.step;
	nChannels = frameImage.channels();

	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			*(pData + row * stepWidth + col * nChannels + 0) = pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 2];
			*(pData + row * stepWidth + col * nChannels + 1) = pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 1];
			*(pData + row * stepWidth + col * nChannels + 2) = pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 0];
		}
	}
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 2] = *(pData + row * stepWidth + col * nChannels + 0);
			pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 1] = *(pData + row * stepWidth + col * nChannels + 1);
			pFrame->data[0][row * pFrame->linesize[0] + col * nChannels + 0] = *(pData + row * stepWidth + col * nChannels + 2);
		}
	}
	return frameImage;
}


unsigned int __stdcall captrue_video(void* param);






UINT __cdecl CFFGAMEDlg::VideoPlay(LPVOID lparam) //线程1函数定义
{

	//==========================================
	//插入式廣告:
	cap.open("chinapig.mp4");
	if (!cap.isOpened())
	{
		cerr << "ERROR! Unable to open camera\n";
		return -1;
	}
	//==========================================
	int got_frame(0);
	bool b_UseGPUNVenc(false);
	//=======================================
	//AVFormatContext* video_ctx = NULL;//....................新增變數
	//AVFormatContext* audio_ctx = NULL;//....................新增變數
	AVFormatContext* out_ctx;
	//=======================================
	OutputStream video_st = { 0 }, audio_st = { 0 };
	const char* out_filename = "VRecord.mp4";//輸出檔案
	AVOutputFormat* out_fmt;
	AVCodec* audio_Encoder, * video_Encoder;//編碼器
	int ret;
	int have_video = 0, have_audio = 0;
	int encode_video = 0, encode_audio = 0;
	AVDictionary* opt = NULL;
	int i;
	//======================================================================
	//1.OpenInoput讀取視頻資訊(開啟解碼器)
	video_index = (0);
	audio_index = (0);
	av_register_all();
	avformat_network_init();
	avdevice_register_all();//Registe
	if ((ret = avformat_open_input(&video_ctx, "123.mp4", NULL, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return -1;
	}
	if ((ret = avformat_find_stream_info(video_ctx, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return -1;
	}

	for (i = 0; i < video_ctx->nb_streams; i++) {
		AVStream* stream;
		AVCodecContext* codec_ctx;
		stream = video_ctx->streams[i];
		codec_ctx = stream->codec;
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index = i;
			/* Open decoder */
			if (b_UseGPUNVenc)
			{
				ret = avcodec_open2(codec_ctx, avcodec_find_decoder_by_name("h264_cuvid"), NULL);
				if (!ret)
				{
					b_UseGPUNVenc = false;
					ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
				}
			}
			else {
				ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
			}
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return -1;
			}
		}
	}
	av_dump_format(video_ctx, 0, "123.mp4", 0);


	//2.開啟音訊解碼器和影片
	audio_ctx = NULL;


	av_register_all();
	avdevice_register_all();
	avfilter_register_all();

	//AVInputFormat* ifmt = av_find_input_format("dshow");
	//if ((ret = avformat_open_input(&audio_ctx, "audio=Stereo Mix (Realtek(R) Audio)", ifmt, NULL)) < 0)

	if ((ret = avformat_open_input(&audio_ctx, "123.mp4", NULL, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return -1;
	}
	if ((ret = avformat_find_stream_info(audio_ctx, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return -1;
	}
	for (i = 0; i < audio_ctx->nb_streams; i++) {
		AVStream* stream;
		AVCodecContext* codec_ctx;
		stream = audio_ctx->streams[i];
		codec_ctx = stream->codec;
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index = i;
			/* Open decoder */
			ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return -1;
			}
		}
	}
	av_dump_format(audio_ctx, 0, "123.mp4", 0);



	_beginthreadex(NULL, 0, captrue_video, NULL, 0, NULL);//影片可以開始先解碼了


//試試看解碼
	/*
	struct SwsContext* img_convert_ctx = NULL;
	struct SwsContext* img_convert_ctx2 = NULL;
	int stream_index(0);
	int w = video_ctx->streams[stream_index]->codec->width;
	int h = video_ctx->streams[stream_index]->codec->height;
	img_convert_ctx = sws_getContext(w, h, video_ctx->streams[stream_index]->codec->pix_fmt, w, h, AV_PIX_FMT_RGB24, 4, NULL, NULL, NULL);
	img_convert_ctx2 = sws_getContext(w, h, AV_PIX_FMT_RGB24, w, h, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	AVPacket  packet;
	enum AVMediaType type;
	while (ret >= 0)
	{
		ret = av_read_frame(video_ctx, &packet);
		stream_index = packet.stream_index;
		type = video_ctx->streams[packet.stream_index]->codec->codec_type;
		av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
			stream_index);
		if (type == AVMEDIA_TYPE_VIDEO)
		{
			av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
			AVFrame* frame = av_frame_alloc();
			if (!frame) {
				ret = AVERROR(ENOMEM);
				break;
			}
			av_packet_rescale_ts(&packet,
				video_ctx->streams[stream_index]->time_base,
				video_ctx->streams[stream_index]->codec->time_base);

			ret = avcodec_decode_video2(video_ctx->streams[stream_index]->codec, frame, &got_frame, &packet);
			if (ret < 0)
			{
				av_frame_free(&frame);
				av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
				break;
			}
			if (got_frame)
			{

				AVFrame* pFrameRGB = av_frame_alloc();
				int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);// Determine required buffer size and allocate buffer
				uint8_t* H264_out_buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
				avpicture_fill((AVPicture*)pFrameRGB, H264_out_buffer, AV_PIX_FMT_RGB24, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);// Assign appropriate parts of buffer to image planes in pFrameRGB
				sws_scale(img_convert_ctx, frame->data, frame->linesize, 0, video_ctx->streams[stream_index]->codec->height, pFrameRGB->data, pFrameRGB->linesize);
				//Inference here:
				Mat img=WaterData(pFrameRGB, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);
				imshow("1", img);
				waitKey(1);
			}
		}
	}*/



	//===============================================================================================
	//3.設置輸出OpenOut:
	avformat_alloc_output_context2(&out_ctx, NULL, NULL, out_filename);//MP4的輸出類型
	if (!out_ctx)
	{
		printf("Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&out_ctx, NULL, "mpeg", out_filename);
	}
	if (!out_ctx)
	{
		return -1;
	}

	out_fmt = out_ctx->oformat;
	//====================================================================
	//3.設定編碼器(視頻)
	//視頻流(video_st)
	if (out_fmt->video_codec != AV_CODEC_ID_NONE)
	{
		//=========================================
		//用來告訴輸出端的輸入流
		AVStream* in_stream;
		in_stream = video_ctx->streams[video_index];
		AVCodecContext* dec_ctx = in_stream->codec;
		//=========================================
		//add_stream(&video_st, out_ctx, &video_Encoder, out_fmt->video_codec);
		//static void add_stream(OutputStream* ost, AVFormatContext* oc, AVCodec** codec, enum AVCodecID codec_id)
		AVCodecContext* c;
		int i;

		video_Encoder = avcodec_find_encoder(out_fmt->video_codec);
		if (!(video_Encoder))
		{
			fprintf(stderr, "Could not find encoder for '%s'\n",
				avcodec_get_name(out_fmt->video_codec));
			exit(1);
		}
		video_st.st = avformat_new_stream(out_ctx, NULL);
		if (!video_st.st) {
			fprintf(stderr, "Could not allocate stream\n");
			exit(1);
		}
		video_st.st->id = out_ctx->nb_streams - 1;
		c = avcodec_alloc_context3(video_Encoder);
		if (!c) {
			fprintf(stderr, "Could not alloc an encoding context\n");
			exit(1);
		}
		video_st.enc = c;
		//===========================================================================
		//SWITCH內:
		c->codec_id = out_fmt->video_codec;
		c->bit_rate = dec_ctx->bit_rate;
		//解析度設定(必需為2的倍數)
		//c->width = dec_ctx->width;
		//c->height = dec_ctx->height;

		c->width = 1920;
		c->height = 1080;


		// timebase: 這是基本時間單位（以秒為單位）表示其中的幀時間戳。 對於固定fps內容
		//時基應為1 / framerate，時間戳增量應為1
		//video_st.st->time_base = dec_ctx->time_base;//每秒60禎: {1001,6000}
		video_st.st->time_base = AVRational{ 1, STREAM_FRAME_RATE };//每秒多少楨
		//video_st.st->time_base = AVRational{ 1, 30 };
		c->time_base = video_st.st->time_base;

		c->gop_size = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = STREAM_PIX_FMT;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
		{
			/* just for testing, we also add B-frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
		{
			//需要避免使用其中一些係數溢出的宏塊。
			//普通視頻不會發生這種情況，因為
			//色度平面的運動與亮度平面不匹配。
			c->mb_decision = 2;
		}
		if (c->codec_id == AV_CODEC_ID_H264)
		{
			c->me_range = 1;
			c->max_qdiff = 3;
			c->qmin = 1;
			c->qmax = 20;
			c->qcompress = 0.9;
			c->noise_reduction = 0;
		}

		//===========================================================================
		//某些格式希望流頭分開
		if (out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		have_video = 1;
		encode_video = 1;
	}
	//============================================================================================================
	//4.設定編碼器(音頻)
	if (out_fmt->audio_codec != AV_CODEC_ID_NONE)
	{
		//=========================================
		//用來告訴輸出端的輸入流
		AVStream* in_stream;
		in_stream = audio_ctx->streams[audio_index];//聲音部分
		AVCodecContext* dec_ctx = in_stream->codec;
		//=========================================
		//add_stream(&audio_st, out_ctx, &audio_Encoder, out_fmt->audio_codec);
		//static void add_stream(OutputStream* ost, AVFormatContext* oc, AVCodec** codec, enum AVCodecID codec_id)
		AVCodecContext* c;
		int i;

		//設定編碼器
		audio_Encoder = avcodec_find_encoder(out_fmt->audio_codec);
		if (!(audio_Encoder))
		{
			fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(out_fmt->audio_codec));
			exit(1);
		}

		audio_st.st = avformat_new_stream(out_ctx, NULL);
		if (!audio_st.st)
		{
			fprintf(stderr, "Could not allocate stream\n");
			exit(1);
		}
		audio_st.st->id = out_ctx->nb_streams - 1;
		c = avcodec_alloc_context3(audio_Encoder);
		if (!c)
		{
			fprintf(stderr, "Could not alloc an encoding context\n");
			exit(1);
		}
		audio_st.enc = c;
		//===========================================================================
		//SWITCH內:
		c->sample_fmt = (audio_Encoder)->sample_fmts ? (audio_Encoder)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 60000;//128000 64000
		c->sample_rate = SampleRate11025;//44100
		if ((audio_Encoder)->supported_samplerates)
		{
			c->sample_rate = (audio_Encoder)->supported_samplerates[0];
			for (i = 0; (audio_Encoder)->supported_samplerates[i]; i++)
			{
				if ((audio_Encoder)->supported_samplerates[i] == SampleRate11025)
				{
					c->sample_rate = SampleRate11025;
				}
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((audio_Encoder)->channel_layouts)
		{
			c->channel_layout = (audio_Encoder)->channel_layouts[0];
			for (i = 0; (audio_Encoder)->channel_layouts[i]; i++)
			{
				if ((audio_Encoder)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		audio_st.st->time_base = AVRational{ 1, c->sample_rate };
		//===========================================================================
		//某些格式希望流頭分開
		if (out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		have_audio = 1;
		encode_audio = 1;
	}

	//現在已經設置了所有參數，我們可以打開音頻並
	//正式打開視頻編解碼器(avcodec_open2)，並分配必要的編碼緩衝區。 
	if (have_video)
	{
		open_video(out_ctx, video_Encoder, &video_st, opt);
	}

	if (have_audio)
	{
		open_audio(out_ctx, audio_Encoder, &audio_st, opt);
	}
	av_dump_format(out_ctx, 0, out_filename, 1);

	/* open the output file, if needed */
	if (!(out_fmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&out_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			//fprintf(stderr, "Could not open '%s': %s\n", filename,av_err2str(ret));
			return -1;
		}
	}
	//=====================================================================================
	//寫入輸出檔案的檔頭
	ret = avformat_write_header(out_ctx, &opt);
	if (ret < 0)
	{
		//fprintf(stderr, "Error occurred when opening output file: %s\n",av_err2str(ret));
		return -1;
	}
	//=====================================================================================


	while (encode_video || encode_audio)
	{
		/* select the stream to encode */
		if (encode_video && (!encode_audio || av_compare_ts(video_st.next_pts, video_st.enc->time_base, audio_st.next_pts, audio_st.enc->time_base) <= 0))
		{
			encode_video = !write_video_frame(out_ctx, &video_st);//寫入影片
		}
		else
		{
			nLastTick = GetTickCount() - 150;
			encode_audio = !write_audio_frame(out_ctx, &audio_st, nLastTick);//寫入聲音
		}
	}
	av_write_trailer(out_ctx);
	AfxMessageBox("結束");
	/* Close each codec. */
	if (have_video)
		close_stream(out_ctx, &video_st);
	if (have_audio)
		close_stream(out_ctx, &audio_st);

	if (!(out_fmt->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_closep(&out_ctx->pb);

	/* free the stream */
	avformat_free_context(out_ctx);
	b_Global_Stop = false;//停止旗標結束
	return -1;
}

unsigned int __stdcall captrue_video(void* param)
{
	return 0;
	int ret(0);
	int got_frame(0);
	struct SwsContext* img_convert_ctx = NULL;
	struct SwsContext* img_convert_ctx2 = NULL;
	int stream_index(0);
	int w = video_ctx->streams[stream_index]->codec->width;
	int h = video_ctx->streams[stream_index]->codec->height;
	img_convert_ctx = sws_getContext(w, h, video_ctx->streams[stream_index]->codec->pix_fmt, w, h, AV_PIX_FMT_RGB24, 4, NULL, NULL, NULL);
	img_convert_ctx2 = sws_getContext(w, h, AV_PIX_FMT_RGB24, w, h, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	AVPacket  packet;
	enum AVMediaType type;
	while (ret >= 0)
	{
		ret = av_read_frame(video_ctx, &packet);
		stream_index = packet.stream_index;
		type = video_ctx->streams[packet.stream_index]->codec->codec_type;
		av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
			stream_index);
		if (type == AVMEDIA_TYPE_VIDEO)
		{
			av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
			AVFrame* frame = av_frame_alloc();
			if (!frame) {
				ret = AVERROR(ENOMEM);
				break;
			}
			av_packet_rescale_ts(&packet, video_ctx->streams[stream_index]->time_base, video_ctx->streams[stream_index]->codec->time_base);
			ret = avcodec_decode_video2(video_ctx->streams[stream_index]->codec, frame, &got_frame, &packet);
			if (ret < 0)
			{
				av_frame_free(&frame);
				av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
				break;
			}
			if (got_frame)
			{

				AVFrame* pFrameRGB = av_frame_alloc();
				int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);// Determine required buffer size and allocate buffer
				uint8_t* H264_out_buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
				avpicture_fill((AVPicture*)pFrameRGB, H264_out_buffer, AV_PIX_FMT_RGB24, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);// Assign appropriate parts of buffer to image planes in pFrameRGB
				sws_scale(img_convert_ctx, frame->data, frame->linesize, 0, video_ctx->streams[stream_index]->codec->height, pFrameRGB->data, pFrameRGB->linesize);
				//Inference here:
				Mat img = WaterData(pFrameRGB, video_ctx->streams[stream_index]->codec->width, video_ctx->streams[stream_index]->codec->height);
				imshow("1", img);
				waitKey(1);
			}
		}
	}
	return 0;
}





void CFFGAMEDlg::OnBnClickedRecord()
{
	m_pthreadVideo = AfxBeginThread(VideoPlay, (LPVOID)this);
}
