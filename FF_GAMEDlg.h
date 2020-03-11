
// FF_GAMEDlg.h: 標頭檔
//
#include"pch.h"
#pragma once


// CFFGAMEDlg 對話方塊
class CFFGAMEDlg : public CDialogEx
{
// 建構
public:
	CFFGAMEDlg(CWnd* pParent = nullptr);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FF_GAME_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedRecord();
	void OnBnClickedRecordAudio();
	void Stop();
	//====================================================================================================================
//錄音
	bool win;//采集數據標志位
	FILE* f;//存儲采集到的音頻文件
	HWAVEIN hWaveIn;  //輸入設備
	WAVEFORMATEX waveform; //采集音頻的格式，結構體
	BYTE* pBuffer1, * pBuffer2;//采集音頻時的數據緩存
	WAVEHDR wHdr1, wHdr2; //采集音頻時包含數據緩存的結構體
	static DWORD CALLBACK MicCallback(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);//消息
	//====================================================================================================================
	CWinThread* m_pthreadVideo;
	static UINT __cdecl  VideoPlay(LPVOID lparam);
};
