/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#define LOG_TAG "V4L2Camera"
#include <utils/Log.h>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include "uvc_compat.h"
#include "v4l2_formats.h"
};

#include "V4L2Camera.h"
#include "Utils.h"
#include "Converter.h"

#define HEADERFRAME1 0xaf

namespace android {

V4L2Camera::V4L2Camera ()
        : fd(-1), nQueued(0), nDequeued(0)
{
    videoIn = (struct vdIn *) calloc (1, sizeof (struct vdIn));
}

V4L2Camera::~V4L2Camera()
{
	Close();
    free(videoIn);
}

int V4L2Camera::Open (const char *device)
{
    int ret;

	/* Close the previous instance, if any */
	Close();
	
    memset(videoIn, 0, sizeof (struct vdIn));

    if ((fd = open(device, O_RDWR)) == -1) {
        LOGE("ERROR opening V4L interface: %s", strerror(errno));
        return -1;
    }

    ret = ioctl (fd, VIDIOC_QUERYCAP, &videoIn->cap);
    if (ret < 0) {
        LOGE("Error opening device: unable to query device.");
        return -1;
    }

    if ((videoIn->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        LOGE("Error opening device: video capture not supported.");
        return -1;
    }

    if (!(videoIn->cap.capabilities & V4L2_CAP_STREAMING)) {
        LOGE("Capture device does not support streaming i/o");
        return -1;
    }
	
	/* Enumerate all available frame formats */
	EnumFrameFormats();

    return ret;
}

void V4L2Camera::Close ()
{
	/* Release the temporary buffer, if any */
	if (videoIn->tmpBuffer)
		free(videoIn->tmpBuffer);
	videoIn->tmpBuffer = NULL;

	/* Close the file descriptor */
	if (fd > 0)
		close(fd);
	fd = -1;
}

static int my_abs(int x)
{
	return (x < 0) ? -x : x;
}

int V4L2Camera::Init(int width, int height, int fps)
{
	LOGD("V4L2Camera::Init");
	
	/* Initialize the capture to the specified width and height */
	static const struct {
		int fmt;			/* PixelFormat */
		int bpp;			/* bytes per pixel */
		int isplanar;		/* If format is planar or not */
		int allowscrop;		/* If we support cropping with this pixel format */
	} pixFmtsOrder[] = { 
		{V4L2_PIX_FMT_YUYV,		2,0,1},
		{V4L2_PIX_FMT_YVYU,		2,0,1},
		{V4L2_PIX_FMT_UYVY,		2,0,1},
		{V4L2_PIX_FMT_YYUV,		2,0,1},
		{V4L2_PIX_FMT_SPCA501,	2,0,0},
		{V4L2_PIX_FMT_SPCA505,	2,0,0},
		{V4L2_PIX_FMT_SPCA508,	2,0,0},
		{V4L2_PIX_FMT_YUV420,	0,1,0},
		{V4L2_PIX_FMT_YVU420,	0,1,0},
		{V4L2_PIX_FMT_NV12,		0,1,0},
		{V4L2_PIX_FMT_NV21,		0,1,0},
		{V4L2_PIX_FMT_NV16,		0,1,0},
		{V4L2_PIX_FMT_NV61,		0,1,0},
		{V4L2_PIX_FMT_Y41P,		0,0,0},
		{V4L2_PIX_FMT_SGBRG8,	0,0,0},
		{V4L2_PIX_FMT_SGRBG8,	0,0,0},
		{V4L2_PIX_FMT_SBGGR8,	0,0,0},
		{V4L2_PIX_FMT_SRGGB8,	0,0,0},
		{V4L2_PIX_FMT_BGR24,	3,0,1},
		{V4L2_PIX_FMT_RGB24,	3,0,1},
		{V4L2_PIX_FMT_MJPEG,	0,1,0},
		{V4L2_PIX_FMT_JPEG,		0,1,0},
		{V4L2_PIX_FMT_GREY,		1,0,1},
		{V4L2_PIX_FMT_Y16,		2,0,1},
	};

    int ret;

	// If no formats, break here
	if (m_AllFmts.isEmpty()) {
		LOGE("No video formats available");
		return -1;
	}

	// Try to get the closest match ... 
	SurfaceDesc closest;
	int closestDArea = -1;
	int closestDFps = -1;
	unsigned int i;
	int area = width * height;
	for (i = 0; i < m_AllFmts.size(); i++) {
		SurfaceDesc sd = m_AllFmts[i];
		
		// Always choose a bigger or equal surface
		if (sd.getWidth() >= width &&
			sd.getHeight() >= height) {

			int difArea = sd.getArea() - area;
			int difFps = my_abs(sd.getFps() - fps);
	
			LOGD("Trying format: (%d x %d), Fps: %d [difArea:%d, difFps:%d, cDifArea:%d, cDifFps:%d]",sd.getWidth(),sd.getHeight(),sd.getFps(), difArea, difFps, closestDArea, closestDFps);	
			if (closestDArea < 0 || 
				difArea < closestDArea ||
				(difArea == closestDArea && difFps < closestDFps)) {
			
				// Store approximation
				closestDArea = difArea;
				closestDFps = difFps;
				
				// And the new surface descriptor
				closest = sd;
			}
		}
	}
	
	if (closestDArea == -1) {
		LOGE("Size not available: (%d x %d)",width,height);
		return -1;
	}

	LOGD("Selected format: (%d x %d), Fps: %d",closest.getWidth(),closest.getHeight(),closest.getFps());
	
	// Check if we will have to crop the captured image
	bool crop = width != closest.getWidth() || height != closest.getHeight();
	
	// Iterate through pixel formats from best to worst
	ret = -1;
	for (i=0; i < (sizeof(pixFmtsOrder) / sizeof(pixFmtsOrder[0])); i++) {
	
		// If we will need to crop, make sure to only select formats we can crop...
		if (!crop || pixFmtsOrder[i].allowscrop) {
		
			memset(&videoIn->format,0,sizeof(videoIn->format));
			videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			videoIn->format.fmt.pix.width = closest.getWidth();
			videoIn->format.fmt.pix.height = closest.getHeight();
			videoIn->format.fmt.pix.pixelformat = pixFmtsOrder[i].fmt;

			ret = ioctl(fd, VIDIOC_TRY_FMT, &videoIn->format);
			if (ret >= 0) {
				break;
			}
		}
	}
    if (ret < 0) {
        LOGE("Open: VIDIOC_TRY_FMT Failed: %s", strerror(errno));
        return ret;
    }

	/* Set the format */
	memset(&videoIn->format,0,sizeof(videoIn->format));
	videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->format.fmt.pix.width = closest.getWidth();
	videoIn->format.fmt.pix.height = closest.getHeight();
	videoIn->format.fmt.pix.pixelformat = pixFmtsOrder[i].fmt;
	ret = ioctl(fd, VIDIOC_S_FMT, &videoIn->format);
    if (ret < 0) {
        LOGE("Open: VIDIOC_S_FMT Failed: %s", strerror(errno));
        return ret;
    }

	
	/* Query for the effective video format used */
	memset(&videoIn->format,0,sizeof(videoIn->format));
	videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_FMT, &videoIn->format);
    if (ret < 0) {
        LOGE("Open: VIDIOC_G_FMT Failed: %s", strerror(errno));
        return ret;
    }
	
	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	unsigned int min = videoIn->format.fmt.pix.width * 2;
	if (videoIn->format.fmt.pix.bytesperline < min)
		videoIn->format.fmt.pix.bytesperline = min;
	min = videoIn->format.fmt.pix.bytesperline * videoIn->format.fmt.pix.height;
	if (videoIn->format.fmt.pix.sizeimage < min)
		videoIn->format.fmt.pix.sizeimage = min;

	/* Store the pixel formats we will use */
	videoIn->outWidth 			= width;
	videoIn->outHeight 			= height;
	videoIn->outFrameSize 		= width * height << 1; // Calculate the expected output framesize in YUYV
	videoIn->capBytesPerPixel	= pixFmtsOrder[i].bpp;
	
	/* Now calculate cropping margins, if needed, rounding to even */
	int startX = ((closest.getWidth() - width) >> 1) & (-2);
	int startY = ((closest.getHeight() - height) >> 1) & (-2);
	
	/* Avoid crashing if the mode found is smaller than the requested */
	if (startX < 0) {
		videoIn->outWidth += startX;
		startX = 0;
	}
	if (startY < 0) {
		videoIn->outHeight += startY;
		startY = 0;
	}
	
	/* Calculate the starting offset into each captured frame */
	videoIn->capCropOffset = (startX * videoIn->capBytesPerPixel) +
			(videoIn->format.fmt.pix.bytesperline * startY);	
	
	LOGI("Cropping from origin: %dx%d - size: %dx%d  (offset:%d)", 
		startX,startY,
		videoIn->outWidth,videoIn->outHeight,
		videoIn->capCropOffset);
	
	/* sets video device frame rate */
	memset(&videoIn->params,0,sizeof(videoIn->params));
	videoIn->params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->params.parm.capture.timeperframe.numerator = 1;
	videoIn->params.parm.capture.timeperframe.denominator = closest.getFps();

	/* Set the framerate. If it fails, it wont be fatal */
	if (ioctl(fd,VIDIOC_S_PARM,&videoIn->params) < 0) 
	{
		LOGE("VIDIOC_S_PARM error: Unable to set %d fps", closest.getFps());
	} 
	
	/* Gets video device defined frame rate (not real - consider it a maximum value) */
	if (ioctl(fd,VIDIOC_G_PARM,&videoIn->params) < 0) 
	{
		LOGE("VIDIOC_G_PARM - Unable to get timeperframe");
	} 
	
	LOGI("Actual format: (%d x %d), Fps: %d, pixfmt: '%c%c%c%c', bytesperline: %d",
		videoIn->format.fmt.pix.width,
		videoIn->format.fmt.pix.height,
		videoIn->params.parm.capture.timeperframe.denominator,
		videoIn->format.fmt.pix.pixelformat & 0xFF, (videoIn->format.fmt.pix.pixelformat >> 8) & 0xFF,
		(videoIn->format.fmt.pix.pixelformat >> 16) & 0xFF, (videoIn->format.fmt.pix.pixelformat >> 24) & 0xFF,
		videoIn->format.fmt.pix.bytesperline);

	/* Configure JPEG quality, if dealing with those formats */
	if (videoIn->format.fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG ||
		videoIn->format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {

		/* Get the compression format */
		ioctl(fd,VIDIOC_G_JPEGCOMP, &videoIn->jpegcomp);

		/* Set to maximum */
		videoIn->jpegcomp.quality = 100;
		
		/* Try to set it */
		if(ioctl(fd,VIDIOC_S_JPEGCOMP, &videoIn->jpegcomp) >= 0)
		{
			LOGE("VIDIOC_S_COMP:");
			if(errno == EINVAL)
			{
				videoIn->jpegcomp.quality = -1; //not supported
				LOGE("   compression control not supported\n");
			}
		}

		/* gets video stream jpeg compression parameters */
		if(ioctl(fd,VIDIOC_G_JPEGCOMP, &videoIn->jpegcomp) >= 0)
		{
			LOGD("VIDIOC_G_COMP:\n");
			LOGD("    quality:      %i\n", videoIn->jpegcomp.quality);
			LOGD("    APPn:         %i\n", videoIn->jpegcomp.APPn);
			LOGD("    APP_len:      %i\n", videoIn->jpegcomp.APP_len);
			LOGD("    APP_data:     %s\n", videoIn->jpegcomp.APP_data);
			LOGD("    COM_len:      %i\n", videoIn->jpegcomp.COM_len);
			LOGD("    COM_data:     %s\n", videoIn->jpegcomp.COM_data);
			LOGD("    jpeg_markers: 0x%x\n", videoIn->jpegcomp.jpeg_markers);
		}
		else
		{
			LOGE("VIDIOC_G_COMP:");
			if(errno == EINVAL)
			{
				videoIn->jpegcomp.quality = -1; //not supported
				LOGE("   compression control not supported\n");
			}
		}
	}
	
    /* Check if camera can handle NB_BUFFER buffers */
	memset(&videoIn->rb,0,sizeof(videoIn->rb));
    videoIn->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    videoIn->rb.memory = V4L2_MEMORY_MMAP;
    videoIn->rb.count = NB_BUFFER;

    ret = ioctl(fd, VIDIOC_REQBUFS, &videoIn->rb);
    if (ret < 0) {
        LOGE("Init: VIDIOC_REQBUFS failed: %s", strerror(errno));
        return ret;
    }

    for (int i = 0; i < NB_BUFFER; i++) {

        memset (&videoIn->buf, 0, sizeof (struct v4l2_buffer));
        videoIn->buf.index = i;
        videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoIn->buf.memory = V4L2_MEMORY_MMAP;

        ret = ioctl (fd, VIDIOC_QUERYBUF, &videoIn->buf);
        if (ret < 0) {
            LOGE("Init: Unable to query buffer (%s)", strerror(errno));
            return ret;
        }

        videoIn->mem[i] = mmap (0,
                                videoIn->buf.length,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fd,
                                videoIn->buf.m.offset);

        if (videoIn->mem[i] == MAP_FAILED) {
            LOGE("Init: Unable to map buffer (%s)", strerror(errno));
            return -1;
        }

        ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
        if (ret < 0) {
            LOGE("Init: VIDIOC_QBUF Failed");
            return -1;
        }

        nQueued++;
    }
	
	// Reserve temporary buffers, if they will be needed
	size_t tmpbuf_size=0;
	switch (videoIn->format.fmt.pix.pixelformat) 
	{
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YUV420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_YVU420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_Y41P:   // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_SPCA501:
		case V4L2_PIX_FMT_SPCA505:
		case V4L2_PIX_FMT_SPCA508:
		case V4L2_PIX_FMT_GREY:
	    case V4L2_PIX_FMT_Y16:
		
		case V4L2_PIX_FMT_YUYV:
			//  YUYV doesn't need a temp buffer but we will set it if/when
			//  video processing disable control is checked (bayer processing).
			//            (logitech cameras only) 
			break;
		
		case V4L2_PIX_FMT_SGBRG8: //0
		case V4L2_PIX_FMT_SGRBG8: //1
		case V4L2_PIX_FMT_SBGGR8: //2
		case V4L2_PIX_FMT_SRGGB8: //3
			// Raw 8 bit bayer 
			// when grabbing use:
			//    bayer_to_rgb24(bayer_data, RGB24_data, width, height, 0..3)
			//    rgb2yuyv(RGB24_data, pFrameBuffer, width, height)
	
			// alloc a temp buffer for converting to YUYV
			// rgb buffer for decoding bayer data
			tmpbuf_size = videoIn->format.fmt.pix.width * videoIn->format.fmt.pix.height * 3;
			if (videoIn->tmpBuffer)
				free(videoIn->tmpBuffer);
			videoIn->tmpBuffer = (uint8_t*)calloc(1, tmpbuf_size);
			if (!videoIn->tmpBuffer) 
			{
				LOGE("couldn't calloc %lu bytes of memory for frame buffer\n",
					(unsigned long) tmpbuf_size);
				return -ENOMEM;
			} 
			
		
			break;
			
		case V4L2_PIX_FMT_RGB24: //rgb or bgr (8-8-8)
		case V4L2_PIX_FMT_BGR24:
			break;
			
		default:
			LOGE("Should never arrive (1)- exit fatal !!\n");
			return -1;
	} 	

    return 0;
}

void V4L2Camera::Uninit ()
{
    int ret;

	memset(&videoIn->buf,0,sizeof(videoIn->buf));
    videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    videoIn->buf.memory = V4L2_MEMORY_MMAP;

    /* Dequeue everything */
    int DQcount = nQueued - nDequeued;

    for (int i = 0; i < DQcount-1; i++) {
        ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
        if (ret < 0)
            LOGE("Uninit: VIDIOC_DQBUF Failed");
    }
    nQueued = 0;
    nDequeued = 0;

    /* Unmap buffers */
    for (int i = 0; i < NB_BUFFER; i++)
		if (videoIn->mem[i] != NULL) {
			if (munmap(videoIn->mem[i], videoIn->buf.length) < 0)
				LOGE("Uninit: Unmap failed");
			videoIn->mem[i] = NULL;
		}
		
	if (videoIn->tmpBuffer)
		free(videoIn->tmpBuffer);
	videoIn->tmpBuffer = NULL;
		
}

int V4L2Camera::StartStreaming ()
{
    enum v4l2_buf_type type;
    int ret;

    if (!videoIn->isStreaming) {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ret = ioctl (fd, VIDIOC_STREAMON, &type);
        if (ret < 0) {
            LOGE("StartStreaming: Unable to start capture: %s", strerror(errno));
            return ret;
        }

        videoIn->isStreaming = true;
    }

    return 0;
}

int V4L2Camera::StopStreaming ()
{
    enum v4l2_buf_type type;
    int ret;

    if (videoIn->isStreaming) {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ret = ioctl (fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
            LOGE("StopStreaming: Unable to stop capture: %s", strerror(errno));
            return ret;
        }

        videoIn->isStreaming = false;
    }

    return 0;
}

/* Returns the effective capture size */
void V4L2Camera::getSize(int& width, int& height) const
{
	width =  videoIn->outWidth;
	height = videoIn->outHeight;
}

/* Returns the effective fps */
int V4L2Camera::getFps() const
{
	return videoIn->params.parm.capture.timeperframe.denominator;
}

/* Grab frame in YUYV mode */
void V4L2Camera::GrabRawFrame (void *frameBuffer, int maxSize)
{
	LOGD("V4L2Camera::GrabRawFrame: frameBuffer:%p, len:%d",frameBuffer,maxSize);
    int ret;

	/* DQ */
	memset(&videoIn->buf,0,sizeof(videoIn->buf));
    videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    videoIn->buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
    if (ret < 0) {
        LOGE("GrabPreviewFrame: VIDIOC_DQBUF Failed");
        return;
    }

    nDequeued++;
	
	// Calculate the stride of the output image (YUYV) in bytes
	int strideOut = videoIn->outWidth << 1;
	
	// And the pointer to the start of the image
	uint8_t* src = (uint8_t*)videoIn->mem[videoIn->buf.index] + videoIn->capCropOffset;
	
	LOGD("V4L2Camera::GrabRawFrame - Got Raw frame (%dx%d) (buf:%d@0x%p, len:%d)",videoIn->format.fmt.pix.width,videoIn->format.fmt.pix.height,videoIn->buf.index,src,videoIn->buf.bytesused);
	
	/* Avoid crashing! - Make sure there is enough room in the output buffer! */
	if (maxSize < videoIn->outFrameSize) {
	
		LOGE("V4L2Camera::GrabRawFrame: Insufficient space in output buffer: Required: %d, Got %d - DROPPING FRAME",videoIn->outFrameSize,maxSize);
		
	} else {
	
		switch (videoIn->format.fmt.pix.pixelformat) 
		{
			case V4L2_PIX_FMT_JPEG:
			case V4L2_PIX_FMT_MJPEG:
				if(videoIn->buf.bytesused <= HEADERFRAME1) 
				{
					// Prevent crash on empty image
					LOGE("Ignoring empty buffer ...\n");
					break;
				}

				if (jpeg_decode((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight) < 0) 
				{
					LOGE("jpeg decode errors\n");
					break;
				}
				break;
			
			case V4L2_PIX_FMT_UYVY:
				uyvy_to_yuyv((uint8_t*)frameBuffer, strideOut,
							 src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_YVYU:
				yvyu_to_yuyv((uint8_t*)frameBuffer, strideOut,
							 src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_YYUV:
				yyuv_to_yuyv((uint8_t*)frameBuffer, strideOut,
							 src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_YUV420:
				yuv420_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_YVU420:
				yvu420_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_NV12:
				nv12_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_NV21:
				nv21_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_NV16:
				nv16_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_NV61:
				nv61_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_Y41P: 
				y41p_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_GREY:
				grey_to_yuyv((uint8_t*)frameBuffer, strideOut,
							src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_Y16:
				y16_to_yuyv((uint8_t*)frameBuffer, strideOut,
							src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_SPCA501:
				s501_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_SPCA505:
				s505_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_SPCA508:
				s508_to_yuyv((uint8_t*)frameBuffer, strideOut, src, videoIn->outWidth, videoIn->outHeight);
				break;
			
			case V4L2_PIX_FMT_YUYV:
				{
					int h;
					uint8_t* pdst = (uint8_t*)frameBuffer;
					uint8_t* psrc = src;
					int ss = videoIn->outWidth << 1;
					for (h = 0; h < videoIn->outHeight; h++) {
						memcpy(pdst,psrc,ss);
						pdst += strideOut;
						psrc += videoIn->format.fmt.pix.bytesperline;
					}
				}
				break;
				
			case V4L2_PIX_FMT_SGBRG8: //0
				bayer_to_rgb24 (src,(uint8_t*) videoIn->tmpBuffer, videoIn->outWidth, videoIn->outHeight, 0);
				rgb_to_yuyv ((uint8_t*) frameBuffer, strideOut, 
							(uint8_t*)videoIn->tmpBuffer, videoIn->outWidth*3, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_SGRBG8: //1
				bayer_to_rgb24 (src,(uint8_t*) videoIn->tmpBuffer, videoIn->outWidth, videoIn->outHeight, 1);
				rgb_to_yuyv ((uint8_t*) frameBuffer, strideOut, 
							(uint8_t*)videoIn->tmpBuffer, videoIn->outWidth*3, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_SBGGR8: //2
				bayer_to_rgb24 (src,(uint8_t*) videoIn->tmpBuffer, videoIn->outWidth, videoIn->outHeight, 2);
				rgb_to_yuyv ((uint8_t*) frameBuffer, strideOut, 
							(uint8_t*)videoIn->tmpBuffer, videoIn->outWidth*3, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_SRGGB8: //3
				bayer_to_rgb24 (src,(uint8_t*) videoIn->tmpBuffer, videoIn->outWidth, videoIn->outHeight, 3);
				rgb_to_yuyv ((uint8_t*) frameBuffer, strideOut, 
							(uint8_t*)videoIn->tmpBuffer, videoIn->outWidth*3, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_RGB24:
				rgb_to_yuyv((uint8_t*) frameBuffer, strideOut, 
							src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
				
			case V4L2_PIX_FMT_BGR24:
				bgr_to_yuyv((uint8_t*) frameBuffer, strideOut, 
							src, videoIn->format.fmt.pix.bytesperline, videoIn->outWidth, videoIn->outHeight);
				break;
			
			default:
				LOGE("error grabbing: unknown format: %i\n", videoIn->format.fmt.pix.pixelformat);
				break;
		}
		
		LOGD("V4L2Camera::GrabRawFrame - Copied frame to destination 0x%p",frameBuffer);
	}
	
	/* And Queue the buffer again */
    ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
    if (ret < 0) {
        LOGE("GrabPreviewFrame: VIDIOC_QBUF Failed");
        return;
    }

    nQueued++;
	
	LOGD("V4L2Camera::GrabRawFrame - Queued buffer");

}

/* enumerate frame intervals (fps)
 * args:
 * pixfmt: v4l2 pixel format that we want to list frame intervals for
 * width: video width that we want to list frame intervals for
 * height: video height that we want to list frame intervals for
 * 
 * returns 0 if enumeration succeded or errno otherwise               */
bool V4L2Camera::EnumFrameIntervals(int pixfmt, int width, int height)
{
	LOGD("V4L2Camera::EnumFrameIntervals: pixfmt: 0x%08x, w:%d, h:%d",pixfmt,width,height);
	
	struct v4l2_frmivalenum fival;
	int list_fps=0;
	memset(&fival, 0, sizeof(fival));
	fival.index = 0;
	fival.pixel_format = pixfmt;
	fival.width = width;
	fival.height = height;
	
	LOGD("\tTime interval between frame: ");
	while (ioctl(fd,VIDIOC_ENUM_FRAMEINTERVALS, &fival) >=0 ) 
	{
		fival.index++;
		if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) 
		{
			LOGD("%u/%u", fival.discrete.numerator, fival.discrete.denominator);
			
			m_AllFmts.add( SurfaceDesc( width, height, fival.discrete.denominator ) );
			list_fps++;
		} 
		else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) 
		{
			LOGD("{min { %u/%u } .. max { %u/%u } }",
				fival.stepwise.min.numerator, fival.stepwise.min.numerator,
				fival.stepwise.max.denominator, fival.stepwise.max.denominator);
			break;
		} 
		else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) 
		{
			LOGD("{min { %u/%u } .. max { %u/%u } / "
				"stepsize { %u/%u } }",
				fival.stepwise.min.numerator, fival.stepwise.min.denominator,
				fival.stepwise.max.numerator, fival.stepwise.max.denominator,
				fival.stepwise.step.numerator, fival.stepwise.step.denominator);
			break;
		}
	}

	// Assume at least 1fps
	if (list_fps == 0)
	{
		m_AllFmts.add( SurfaceDesc( width, height, 1 ) );
	}
	
	return true;
}

/* enumerate frame sizes
 * pixfmt: v4l2 pixel format that we want to list frame sizes for
 * 
 * returns 0 if enumeration succeded or errno otherwise               */
bool V4L2Camera::EnumFrameSizes(int pixfmt)
{
	LOGD("V4L2Camera::EnumFrameSizes: pixfmt: 0x%08x",pixfmt);
	int ret=0;
	int fsizeind = 0;
	struct v4l2_frmsizeenum fsize;

	memset(&fsize, 0, sizeof(fsize));
	fsize.index = 0;
	fsize.pixel_format = pixfmt;
	while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsize) >= 0) 
	{
		fsize.index++;
		if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) 
		{
			LOGD("{ discrete: width = %u, height = %u }",
				fsize.discrete.width, fsize.discrete.height);
			
			fsizeind++;
			
			if (!EnumFrameIntervals(pixfmt,fsize.discrete.width, fsize.discrete.height)) 
				LOGD("  Unable to enumerate frame intervals");
		} 
		else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) 
		{
			LOGD("{ continuous: min { width = %u, height = %u } .. "
				"max { width = %u, height = %u } }",
				fsize.stepwise.min_width, fsize.stepwise.min_height,
				fsize.stepwise.max_width, fsize.stepwise.max_height);
			LOGD("  will not enumerate frame intervals.\n");
		} 
		else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) 
		{
			LOGD("{ stepwise: min { width = %u, height = %u } .. "
				"max { width = %u, height = %u } / "
				"stepsize { width = %u, height = %u } }",
				fsize.stepwise.min_width, fsize.stepwise.min_height,
				fsize.stepwise.max_width, fsize.stepwise.max_height,
				fsize.stepwise.step_width, fsize.stepwise.step_height);
			LOGD("  will not enumerate frame intervals.");
		} 
		else 
		{
			LOGE("  fsize.type not supported: %d\n", fsize.type);
			LOGE("     (Discrete: %d   Continuous: %d  Stepwise: %d)",
				V4L2_FRMSIZE_TYPE_DISCRETE,
				V4L2_FRMSIZE_TYPE_CONTINUOUS,
				V4L2_FRMSIZE_TYPE_STEPWISE);
		}
	}
	if (fsizeind == 0) 
	{		
		/* ------ gspca doesn't enumerate frame sizes ------ */
		/*       negotiate with VIDIOC_TRY_FMT instead       */
		static const struct {
			int w,h;
		} defMode[] = { 
			{800,600},
			{768,576},
			{768,480},
			{720,576},
			{720,480},
			{704,576},
			{704,480},
			{640,480},
			{352,288},
			{320,240}
		};
		
		unsigned int i;
		for (i = 0 ; i < (sizeof(defMode) / sizeof(defMode[0])); i++) {
		
			fsizeind++;
			struct v4l2_format fmt;
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width = defMode[i].w;
			fmt.fmt.pix.height = defMode[i].h;
			fmt.fmt.pix.pixelformat = pixfmt;
			fmt.fmt.pix.field = V4L2_FIELD_ANY;
			
			if (ioctl(fd,VIDIOC_TRY_FMT, &fmt) >= 0) 
			{
				LOGD("{ ?GSPCA? : width = %u, height = %u }\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

				// Add the mode descriptor
				m_AllFmts.add( SurfaceDesc( fmt.fmt.pix.width, fmt.fmt.pix.height, 25 ) );
			}
		}
	}
	
	return true;
}

/* enumerate frames (formats, sizes and fps)
 * args:
 * width: current selected width
 * height: current selected height
 *
 * returns: pointer to LFormats struct containing list of available frame formats */
bool V4L2Camera::EnumFrameFormats()
{
	LOGD("V4L2Camera::EnumFrameFormats");
	struct v4l2_fmtdesc fmt;
	
	// Start with no modes
	m_AllFmts.clear();
	
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while (ioctl(fd,VIDIOC_ENUM_FMT, &fmt) >= 0) 
	{
		fmt.index++;
		LOGD("{ pixelformat = '%c%c%c%c', description = '%s' }",
				fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
				(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
				fmt.description);

		//enumerate frame sizes for this pixel format
		if (!EnumFrameSizes(fmt.pixelformat)) {
			LOGE("  Unable to enumerate frame sizes.");
		}
	};
	
	// Now, select the best preview format and the best PictureFormat
	m_BestPreviewFmt = SurfaceDesc();
	m_BestPictureFmt = SurfaceDesc();
	
	unsigned int i;
	for (i=0; i<m_AllFmts.size(); i++) 
	{
		SurfaceDesc s = m_AllFmts[i];
		
		// Prioritize size over everything else when taking pictures. use the 
		// least fps possible, as that usually means better quality
		if ((s.getSize()  > m_BestPictureFmt.getSize()) ||
			(s.getSize() == m_BestPictureFmt.getSize() && s.getFps() < m_BestPictureFmt.getFps() )
			) {
			m_BestPictureFmt = s;
		}
		
		// Prioritize fps, then size when doing preview
		if ((s.getFps()  > m_BestPreviewFmt.getFps()) ||
			(s.getFps() == m_BestPreviewFmt.getFps() && s.getSize() > m_BestPreviewFmt.getSize() )
			) {
			m_BestPreviewFmt = s;
		}
		
	}
	
	return true;
} 

SortedVector<SurfaceSize> V4L2Camera::getAvailableSizes() const
{
	LOGD("V4L2Camera::getAvailableSizes");
	SortedVector<SurfaceSize> ret;
	
	// Iterate through the list. All duplicated entries will be removed
	unsigned int i;
	for (i = 0; i< m_AllFmts.size() ; i++) 
	{
		ret.add(m_AllFmts[i].getSize());
	}
	return ret;
}


SortedVector<int> V4L2Camera::getAvailableFps() const
{
	LOGD("V4L2Camera::getAvailableFps");
	SortedVector<int> ret;
	
	// Iterate through the list. All duplicated entries will be removed
	unsigned int i;
	for (i = 0; i< m_AllFmts.size() ; i++) 
	{
		ret.add(m_AllFmts[i].getFps());
	}
	return ret;
	
}

const SurfaceDesc& V4L2Camera::getBestPreviewFmt() const
{
	return m_BestPreviewFmt;
}

const SurfaceDesc& V4L2Camera::getBestPictureFmt() const
{
	return m_BestPictureFmt;
}
 

}; // namespace android
