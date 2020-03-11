// pch.h: 此為先行編譯的標頭檔。
// 以下所列檔案只會編譯一次，可改善之後組建的組建效能。
// 這也會影響 IntelliSense 效能，包括程式碼完成以及許多程式碼瀏覽功能。
// 但此處所列的檔案，如果其中任一在組建之間進行了更新，即會重新編譯所有檔案。
// 請勿於此處新增會經常更新的檔案，如此將會對於效能優勢產生負面的影響。

#ifndef PCH_H
#define PCH_H

// 請於此新增您要先行編譯的標頭
#include "framework.h"

#endif //PCH_H
//===================================
//錄音
#include <SDKDDKVer.h>
/*
#include<windows.h>
#include <MMSYSTEM.H>
#pragma comment(lib, "WINMM.LIB")
*/
#include <Windows.h>
#pragma comment(lib, "Winmm.lib")
#include <dshow.h>
#pragma comment(lib, "Strmiids.lib")
//===================================


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libavfilter/buffersink.h>
#include<libavfilter/buffersrc.h>
#include<libavutil/opt.h>

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>



#include <libavutil/avstring.h>
#include <libavutil/mathematics.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>
#include <libavutil/parseutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avassert.h>
#include <libavutil/time.h>

#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavcodec/avfft.h>
#include <libswresample/swresample.h>

#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/hwcontext.h>
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavdevice/avdevice.h"  
#include "libavutil/audio_fifo.h"  
}


#ifdef _DEBUG
#pragma comment(lib,"OpenCV/opencv_calib3d2413d.lib")
#pragma comment(lib,"OpenCV/opencv_contrib2413d.lib")
#pragma comment(lib,"OpenCV/opencv_core2413d.lib")
#pragma comment(lib,"OpenCV/opencv_features2d2413d.lib")
#pragma comment(lib,"OpenCV/opencv_flann2413d.lib")
#pragma comment(lib,"OpenCV/opencv_gpu2413d.lib")
#pragma comment(lib,"OpenCV/opencv_highgui2413d.lib")
#pragma comment(lib,"OpenCV/opencv_imgproc2413d.lib")
#pragma comment(lib,"OpenCV/opencv_legacy2413d.lib")
#pragma comment(lib,"OpenCV/opencv_ml2413d.lib")
#pragma comment(lib,"OpenCV/opencv_nonfree2413d.lib")
#pragma comment(lib,"OpenCV/opencv_objdetect2413d.lib")
#pragma comment(lib,"OpenCV/opencv_ocl2413d.lib")
#pragma comment(lib,"OpenCV/opencv_photo2413d.lib")
#pragma comment(lib,"OpenCV/opencv_stitching2413d.lib")
#pragma comment(lib,"OpenCV/opencv_superres2413d.lib")
#pragma comment(lib,"OpenCV/opencv_ts2413d.lib")
#pragma comment(lib,"OpenCV/opencv_video2413d.lib")
#pragma comment(lib,"OpenCV/opencv_videostab2413d.lib")
#else
#pragma comment(lib,"OpenCV/opencv_calib3d2411.lib")
#pragma comment(lib,"OpenCV/opencv_contrib2411.lib")
#pragma comment(lib,"OpenCV/opencv_core2411.lib")
#pragma comment(lib,"OpenCV/opencv_features2d2411.lib")
#pragma comment(lib,"OpenCV/opencv_flann2411.lib")
#pragma comment(lib,"OpenCV/opencv_gpu2411.lib")
#pragma comment(lib,"OpenCV/opencv_highgui2411.lib")
#pragma comment(lib,"OpenCV/opencv_imgproc2411.lib")
#pragma comment(lib,"OpenCV/opencv_legacy2411.lib")
#pragma comment(lib,"OpenCV/opencv_ml2411.lib")
#pragma comment(lib,"OpenCV/opencv_nonfree2411.lib")
#pragma comment(lib,"OpenCV/opencv_objdetect2411.lib")
#pragma comment(lib,"OpenCV/opencv_ocl2411.lib")
#pragma comment(lib,"OpenCV/opencv_photo2411.lib")
#pragma comment(lib,"OpenCV/opencv_stitching2411.lib")
#pragma comment(lib,"OpenCV/opencv_superres2411.lib")
#pragma comment(lib,"OpenCV/opencv_ts2411.lib")
#pragma comment(lib,"OpenCV/opencv_video2411.lib")
#pragma comment(lib,"OpenCV/opencv_videostab2411.lib")

#endif