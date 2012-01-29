/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#ifndef _V4L2CAMERA_H
#define _V4L2CAMERA_H

#define NB_BUFFER 4

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/SortedVector.h>
extern "C" {
#include "uvc_compat.h"
};
#include "SurfaceDesc.h"

namespace android {

struct vdIn {
    struct v4l2_capability cap;
    struct v4l2_format format;				// Capture format being used
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers rb;
	struct v4l2_streamparm params;  		// v4l2 stream parameters struct
	struct v4l2_jpegcompression jpegcomp;	// v4l2 jpeg compression settings 
	
    void *mem[NB_BUFFER];
    bool isStreaming;
	
	void* tmpBuffer;
	
	int outWidth;							// Requested Output width 
	int outHeight;							// Requested Output height
	int outFrameSize;						// The expected output framesize (in YUYV)
	int capBytesPerPixel;					// Capture bytes per pixel
	int capCropOffset;						// The offset in bytes to add to the captured buffer to get to the first pixel
	
};

class V4L2Camera {

public:
    V4L2Camera();
    ~V4L2Camera();

    int Open (const char *device);
    void Close ();

    int Init (int width, int height, int fps);
    void Uninit ();

    int StartStreaming ();
    int StopStreaming ();

    void GrabRawFrame (void *frameBuffer,int maxSize);
    
	void getSize(int& width, int& height) const;
	int getFps() const;  	
	
	SortedVector<SurfaceSize> getAvailableSizes() const;
	SortedVector<int> getAvailableFps() const;
	const SurfaceDesc& getBestPreviewFmt() const;
	const SurfaceDesc& getBestPictureFmt() const; 	
	
private:
	bool EnumFrameIntervals(int pixfmt, int width, int height);
	bool EnumFrameSizes(int pixfmt);
	bool EnumFrameFormats(); 
	int saveYUYVtoJPEG(uint8_t* src, uint8_t* dst, int maxsize, int width, int height, int quality);
	
private:
    struct vdIn *videoIn;
    int fd;

    int nQueued;
    int nDequeued;
	
	SortedVector<SurfaceDesc> m_AllFmts;		// Available video modes
	SurfaceDesc m_BestPreviewFmt;				// Best preview mode. maximum fps with biggest frame
	SurfaceDesc m_BestPictureFmt;				// Best picture format. maximum size
 	
};

}; // namespace android

#endif
