/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "CameraHardware"

extern "C" {
#include <utils/Log.h>
#include <fcntl.h>
#include <sys/mman.h>

	
#include <fcntl.h>
#include <sys/stat.h> /* for mode definitions */
};

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "CameraHardware.h"
#include "Converter.h"

#define VIDEO_DEVICE	"/dev/video0"
#define MIN_WIDTH  		320
#define MIN_HEIGHT 		240

#ifndef PIXEL_FORMAT_RGB_888
#define PIXEL_FORMAT_RGB_888 3 /* */
#endif

#ifndef PIXEL_FORMAT_RGBA_8888
#define PIXEL_FORMAT_RGBA_8888 1 /* [ov] */
#endif

#ifndef PIXEL_FORMAT_RGBX_8888
#define PIXEL_FORMAT_RGBX_8888 2 
#endif

#ifndef PIXEL_FORMAT_BGRA_8888
#define PIXEL_FORMAT_BGRA_8888 5 /* [ov] */
#endif

#ifndef PIXEL_FORMAT_RGB_565
#define PIXEL_FORMAT_RGB_565  4 /* [ov] */
#endif

// We need this format to allow special preview modes
#ifndef PIXEL_FORMAT_YCrCb_422_I
#define PIXEL_FORMAT_YCrCb_422_I 100
#endif

#ifndef PIXEL_FORMAT_YCbCr_422_SP
#define PIXEL_FORMAT_YCbCr_422_SP 0x10    /* NV16  [ov] */
#endif

#ifndef PIXEL_FORMAT_YCbCr_420_SP
#define PIXEL_FORMAT_YCbCr_420_SP 0x21    /* NV12 */
#endif

#ifndef PIXEL_FORMAT_UNKNOWN
#define PIXEL_FORMAT_UNKNOWN 0
#endif

    /*
     * Android YUV format:
     *
     * This format is exposed outside of the HAL to software
     * decoders and applications.
     * EGLImageKHR must support it in conjunction with the
     * OES_EGL_image_external extension.
     *
     * YV12 is 4:2:0 YCrCb planar format comprised of a WxH Y plane followed
     * by (W/2) x (H/2) Cr and Cb planes.
     *
     * This format assumes
     * - an even width
     * - an even height
     * - a horizontal stride multiple of 16 pixels
     * - a vertical stride equal to the height
     *
     *   y_size = stride * height
     *   c_size = ALIGN(stride/2, 16) * height/2
     *   size = y_size + c_size * 2
     *   cr_offset = y_size
     *   cb_offset = y_size + c_size
     *
     */
#ifndef PIXEL_FORMAT_YV12
#define PIXEL_FORMAT_YV12  0x32315659 /* YCrCb 4:2:0 Planar */
#endif

#ifndef PIXEL_FORMAT_YV16
#define PIXEL_FORMAT_YV16  0x36315659 /* YCrCb 4:2:2 Planar */
#endif

// File to control camera power
#define CAMERA_POWER	    "/sys/devices/platform/shuttle-pm-camera/power_on"

namespace android {

bool CameraHardware::PowerOn()
{
	LOGD("CameraHardware::PowerOn: Power ON camera.");
	
	// power on camera
//RvdB	int handle = ::open(CAMERA_POWER,O_RDWR);
        int handle ; 
//RvdB	if (handle >= 0) {
//RvdB		::write(handle,"1\n",2);
//RvdB		::close(handle);
//RvdB	} else {
//RvdB		LOGE("Could not open %s for writing.", CAMERA_POWER);
//RvdB		return false;
   //RvdB } 
	
	// Wait until the camera is recognized or timed out
	int timeOut = 500;
	do {
		// Try to open the video capture device
		handle = ::open(VIDEO_DEVICE,O_RDWR);
		if (handle >= 0)
			break;
		// Wait a bit
		::usleep(10000);
	} while (--timeOut > 0);
	
	if (handle >= 0) {
		LOGD("Camera powered on");
		::close(handle);
		return true;
	} else {
		LOGE("Unable to power camera");
	}
	
	return false;
}

bool CameraHardware::PowerOff()
{
	LOGD("CameraHardware::PowerOff: Power OFF camera.");
return true ; //RvdB	
	// power on camera
	int handle = ::open(CAMERA_POWER,O_RDWR);
	if (handle >= 0) {
		::write(handle,"0\n",2);
		::close(handle);
	} else {
		LOGE("Could not open %s for writing.", CAMERA_POWER);
		return false;
    } 
	return true;
}

CameraHardware::CameraHardware(const hw_module_t* module)
        :
		mWin(0),	
		mPreviewWinFmt(PIXEL_FORMAT_UNKNOWN),
		mPreviewWinWidth(0),
		mPreviewWinHeight(0),

		mParameters(),
		
		mRawPreviewHeap(0),
		mRawPreviewFrameSize(0),

		mRawPreviewWidth(0),
		mRawPreviewHeight(0),

        mPreviewHeap(0),
        mPreviewFrameSize(0),		
		mPreviewFmt(PIXEL_FORMAT_UNKNOWN),
		
        mRawPictureHeap(0),
		mRawPictureBufferSize(0),
		
        mRecordingHeap(0),
		mRecordingFrameSize(0),
		mRecFmt(PIXEL_FORMAT_UNKNOWN),
		
        mJpegPictureHeap(0),
		mJpegPictureBufferSize(0),
		
		mRecordingEnabled(0),		
		
        mNotifyCb(0),
        mDataCb(0),
        mDataCbTimestamp(0),
		mRequestMemory(0),
        mCallbackCookie(0),
		
        mMsgEnabled(0),
        mCurrentPreviewFrame(0),
        mCurrentRecordingFrame(0)	
		
{
    /*
     * Initialize camera_device descriptor for this object.
     */

    /* Common header */
    common.tag = HARDWARE_DEVICE_TAG;
    common.version = 0;
    common.module = const_cast<hw_module_t*>(module);
    common.close = CameraHardware::close;

    /* camera_device fields. */
    ops = &mDeviceOps;
    priv = this;

	// Power on camera
	PowerOn();

	// Init default parameters
    initDefaultParameters();
}

CameraHardware::~CameraHardware()
{
    LOGD("CameraHardware::destruct");
    if (mPreviewThread != 0) {
        stopPreview();
    }
	
	// Release all memory heaps
	if (mRawPreviewHeap) {
		mRawPreviewHeap->release(mRawPreviewHeap);
		mRawPreviewHeap = NULL;
	}
	
	if (mPreviewHeap) {
		mPreviewHeap->release(mPreviewHeap);
		mPreviewHeap = NULL;
	}

	if (mRawPictureHeap) {
		mRawPictureHeap->release(mRawPictureHeap);
		mRawPictureHeap = NULL;
	}
	
	if (mRecordingHeap) {
		mRecordingHeap->release(mRecordingHeap);
		mRecordingHeap = NULL;
	}

	if (mJpegPictureHeap) {
		mJpegPictureHeap->release(mJpegPictureHeap);
		mJpegPictureHeap = NULL;
	}
	
	// Power off camera
	PowerOff();
}

bool CameraHardware::NegotiatePreviewFormat(struct preview_stream_ops* win)
{
	LOGD("CameraHardware::NegotiatePreviewFormat");
	
	// Get the preview size... If we are recording, use the recording video size instead of the preview size
	int pw, ph;
	if (mRecordingEnabled && mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
		mParameters.getVideoSize(&pw, &ph);
	} else {
		mParameters.getPreviewSize(&pw, &ph);
	}
			
	LOGD("Trying to set preview window geometry to %dx%d",pw,ph);
	mPreviewWinFmt = PIXEL_FORMAT_UNKNOWN;
	mPreviewWinWidth = 0;
	mPreviewWinHeight = 0;
	
	// Set the buffer geometry of the surface and YV12 as the preview format
	//RB if (win->set_buffers_geometry(win,pw,ph,PIXEL_FORMAT_YV12) != NO_ERROR) {
	if (win->set_buffers_geometry(win,pw,ph,PIXEL_FORMAT_RGBA_8888) != NO_ERROR) {
		LOGE("Unable to set buffer geometry");
		return false;
	}

	// Store the preview window format
//RB 	mPreviewWinFmt = PIXEL_FORMAT_YV12;
	mPreviewWinFmt = PIXEL_FORMAT_RGBA_8888;
	mPreviewWinWidth = pw;
	mPreviewWinHeight = ph;
	
	return true;
}

/****************************************************************************
 * Camera API implementation.
 ***************************************************************************/

status_t CameraHardware::connectCamera(hw_device_t** device)
{
	LOGD("CameraHardware::connectCamera");

    *device = &common;
    return NO_ERROR;
}

status_t CameraHardware::closeCamera()
{
	LOGD("CameraHardware::closeCamera");
	releaseCamera();
    return NO_ERROR;
}

status_t CameraHardware::getCameraInfo(struct camera_info* info)
{
    LOGD("CameraHardware::getCameraInfo");

    info->facing = CAMERA_FACING_FRONT;
    info->facing = CAMERA_FACING_BACK; //RvdB 
    info->orientation = 0;

    return NO_ERROR;
}

status_t CameraHardware::setPreviewWindow(struct preview_stream_ops* window)
{
    LOGD("CameraHardware::setPreviewWindow: preview_stream_ops: %p", window);
    {
        Mutex::Autolock lock(mLock);
        
		if (window != NULL) {
			/* The CPU will write each frame to the preview window buffer.
			 * Note that we delay setting preview window buffer geometry until
			 * frames start to come in. */
			status_t res = window->set_usage(window, GRALLOC_USAGE_SW_WRITE_OFTEN);
			if (res != NO_ERROR) {
				res = -res; // set_usage returns a negative errno.
				LOGE("%s: Error setting preview window usage %d -> %s",
					 __FUNCTION__, res, strerror(res));
				return res;
			}
		}
		
		mWin = window;
		
		// setup the preview window geometry to be able to use the full preview window
		if (mPreviewThread != 0 && mWin != 0) {
			
			LOGD("CameraHardware::setPreviewWindow - Negotiating preview format");
			NegotiatePreviewFormat(mWin);
	
		}
		
    }
    return NO_ERROR;
}

void CameraHardware::setCallbacks(camera_notify_callback notify_cb,
                                  camera_data_callback data_cb,
                                  camera_data_timestamp_callback data_cb_timestamp,
								  camera_request_memory get_memory,
                                  void* user)
{
    LOGD("CameraHardware::setCallbacks");
    {
        Mutex::Autolock lock(mLock);
        mNotifyCb = notify_cb;
        mDataCb = data_cb;
        mDataCbTimestamp = data_cb_timestamp;
		mRequestMemory = get_memory;
        mCallbackCookie = user;
    }
}


void CameraHardware::enableMsgType(int32_t msgType)
{
    LOGD("CameraHardware::enableMsgType: %d", msgType);
    {
        Mutex::Autolock lock(mLock);
		int32_t old = mMsgEnabled;
        mMsgEnabled |= msgType;
		
		// If something changed related to the starting or stopping of
		//  the recording process...
		if ((msgType & CAMERA_MSG_VIDEO_FRAME) && 
			(mMsgEnabled ^ old) & CAMERA_MSG_VIDEO_FRAME && mRecordingEnabled) {
			
			// Recreate the heaps if toggling recording changes the raw preview size
			//  and also restart the preview so we use the new size if needed
			initHeapLocked();
		}
    }
}


void CameraHardware::disableMsgType(int32_t msgType)
{
    LOGD("CameraHardware::disableMsgType: %d", msgType);
    {
        Mutex::Autolock lock(mLock);
		int32_t old = mMsgEnabled;
        mMsgEnabled &= ~msgType;
		
		// If something changed related to the starting or stopping of
		//  the recording process...
		if ((msgType & CAMERA_MSG_VIDEO_FRAME) && 
			(mMsgEnabled ^ old) & CAMERA_MSG_VIDEO_FRAME && mRecordingEnabled) {
			
			// Recreate the heaps if toggling recording changes the raw preview size
			//  and also restart the preview so we use the new size if needed
			initHeapLocked();
		}
	}
}

/**
 * Query whether a message, or a set of messages, is enabled.
 * Note that this is operates as an AND, if any of the messages
 * queried are off, this will return false.
 */ 
int CameraHardware::isMsgTypeEnabled(int32_t msgType)
{
	Mutex::Autolock lock(mLock);
	
	// All messages queried must be enabled to return true
    int enabled = (mMsgEnabled & msgType) == msgType;
	
    LOGD("CameraHardware::isMsgTypeEnabled(%d): %d", msgType, enabled);
    return enabled;
}

CameraHardware::PreviewThread::PreviewThread(CameraHardware* hw) :
	Thread(false),
	mHardware(hw) 
{ 
}

	
void CameraHardware::PreviewThread::onFirstRef() 
{
	run("CameraPreviewThread", PRIORITY_URGENT_DISPLAY);
}

bool CameraHardware::PreviewThread::threadLoop() 
{
	mHardware->previewThread();
	// loop until we need to quit
	return true;
}

status_t CameraHardware::startPreviewLocked()
{
    LOGD("CameraHardware::startPreviewLocked");

    if (mPreviewThread != 0) {
        LOGD("CameraHardware::startPreviewLocked: preview already running");
        return NO_ERROR;
    }

    int width, height;
	
	// If we are recording, use the recording video size instead of the preview size
	if (mRecordingEnabled && mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
		mParameters.getVideoSize(&width, &height);
	} else {
		mParameters.getPreviewSize(&width, &height);
	}
	
	int fps = mParameters.getPreviewFrameRate();
	
    LOGD("CameraHardware::startPreviewLocked: Open, %dx%d", width, height);

    status_t ret = camera.Open(VIDEO_DEVICE);
	if (ret != NO_ERROR) {
		LOGE("Failed to initialize Camera");
		return ret;
	}

    LOGD("CameraHardware::startPreviewLocked: Init");

    ret = camera.Init(width, height, fps);
	if (ret != NO_ERROR) {
		LOGE("Failed to setup streaming");
		return ret;
	}
	
	/* Retrieve the real size being used */
	camera.getSize(width, height);
	
	LOGD("CameraHardware::startPreviewLocked: effective size: %dx%d",width, height);

	// If we are recording, use the recording video size instead of the preview size
	if (mRecordingEnabled && mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
	
		/* Store it as the video size to use */
		mParameters.setVideoSize(width, height);
	} else {
	
		/* Store it as the preview size to use */
		mParameters.setPreviewSize(width, height);
	}

	/* And reinit the memory heaps to reflect the real used size if needed */
	initHeapLocked();

    LOGD("CameraHardware::startPreviewLocked: StartStreaming");

    ret = camera.StartStreaming();
	if (ret != NO_ERROR) {
		LOGE("Failed to start streaming");
		return ret;
	}

	// setup the preview window geometry in order to use it to zoom the image
	if (mWin != 0) {
		LOGD("CameraHardware::setPreviewWindow - Negotiating preview format");
		NegotiatePreviewFormat(mWin);
	}
	
    LOGD("CameraHardware::startPreviewLocked: starting PreviewThread");

    mPreviewThread = new PreviewThread(this);

    LOGD("CameraHardware::startPreviewLocked: O - this:0x%p",this);

    return NO_ERROR;
}

status_t CameraHardware::startPreview()
{
	LOGD("CameraHardware::startPreview");
	
    Mutex::Autolock lock(mLock);
	return startPreviewLocked();
}


void CameraHardware::stopPreviewLocked()
{
    LOGD("CameraHardware::stopPreviewLocked");

    if (mPreviewThread != 0) {
        LOGD("CameraHardware::stopPreviewLocked: stopping PreviewThread");

        mPreviewThread->requestExitAndWait();
		mPreviewThread.clear();	

        LOGD("CameraHardware::stopPreviewLocked: Uninit");
        camera.Uninit();
        LOGD("CameraHardware::stopPreviewLocked: StopStreaming");
        camera.StopStreaming();
        LOGD("CameraHardware::stopPreviewLocked: Close");
        camera.Close();
    }

    LOGD("CameraHardware::stopPreviewLocked: OK");
}

void CameraHardware::stopPreview()
{
    LOGD("CameraHardware::stopPreview");

    Mutex::Autolock lock(mLock);
	stopPreviewLocked();
}

int CameraHardware::isPreviewEnabled() 
{
    int enabled = 0;
    {
        Mutex::Autolock lock(mLock);
        enabled = (mPreviewThread != 0);
    }
    LOGD("CameraHardware::isPreviewEnabled: %d", enabled);

    return enabled;
}

status_t CameraHardware::storeMetaDataInBuffers(int value)
{
    LOGD("CameraHardware::storeMetaDataInBuffers: %d", value);
	
	// Do not accept to store metadata in buffers - We will always store
	//  YUV data on video buffers. Metadata, in the case of Nvidia Tegra2
	//  is a descriptor of an OpenMax endpoint that was filled with the
	//  data.
    return (value) ? INVALID_OPERATION : NO_ERROR;
}

status_t CameraHardware::startRecording()
{
    LOGD("CameraHardware::startRecording");
    {
        Mutex::Autolock lock(mLock);
		if (!mRecordingEnabled) {
			mRecordingEnabled = true;
			
			// If something changed related to the starting or stopping of
			//  the recording process...
			if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
				
				// Recreate the heaps if toggling recording changes the raw preview size
				//  and also restart the preview so we use the new size if needed
				initHeapLocked();
			}
		}
    }
    return NO_ERROR;
}

void CameraHardware::stopRecording()
{
    LOGD("CameraHardware::stopRecording");
    {
        Mutex::Autolock lock(mLock);
		if (mRecordingEnabled) {
			mRecordingEnabled = false;
			
			// If something changed related to the starting or stopping of
			//  the recording process...
			if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
				
				// Recreate the heaps if toggling recording changes the raw preview size
				//  and also restart the preview so we use the new size if needed
				initHeapLocked();
			}
		}
    }
}

int CameraHardware::isRecordingEnabled()
{
    //int enabled = 0;
    int enabled = 1;
    {
        Mutex::Autolock lock(mLock);
        enabled = mRecordingEnabled;
    }
    LOGD("CameraHardware::isRecordingEnabled: %d", mRecordingEnabled);
    return enabled;
}

void CameraHardware::releaseRecordingFrame(const void* mem)
{
    LOGD("CameraHardware::releaseRecordingFrame");
}


status_t CameraHardware::setAutoFocus()
{
    LOGD("CameraHardware::setAutoFocus");
    Mutex::Autolock lock(mLock);
    if (createThread(beginAutoFocusThread, this) == false)
        return UNKNOWN_ERROR;
    return NO_ERROR;
}

status_t CameraHardware::cancelAutoFocus()
{
    LOGD("CameraHardware::cancelAutoFocus");
    return NO_ERROR;
}

status_t CameraHardware::takePicture()
{
    LOGD("CameraHardware::takePicture");
    if (createThread(beginPictureThread, this) == false)
        return UNKNOWN_ERROR;
		
    return NO_ERROR;
}

status_t CameraHardware::cancelPicture()
{
    LOGD("CameraHardware::cancelPicture");
    return NO_ERROR;
}

status_t CameraHardware::setParameters(const char* parms)
{
    LOGD("CameraHardware::setParameters");

    CameraParameters params;
    String8 str8_param(parms);
    params.unflatten(str8_param);
	
    Mutex::Autolock lock(mLock);
	
	// If no changes, trivially accept it!
	if (params.flatten() == mParameters.flatten()) {
		LOGD("Trivially accept it. No changes detected");
		return NO_ERROR;
	}

	if (strcmp(params.getPreviewFormat(),"yuv422i-yuyv") && 
		strcmp(params.getPreviewFormat(),"yuv422sp") && 
		strcmp(params.getPreviewFormat(),"yuv420sp") && 
		strcmp(params.getPreviewFormat(),"yuv420p")) {
        LOGE("CameraHardware::setParameters: Unsupported format '%s' for preview",params.getPreviewFormat());
        return BAD_VALUE;
    }

    if (strcmp(params.getPictureFormat(), CameraParameters::PIXEL_FORMAT_JPEG)) {
        LOGE("CameraHardware::setParameters: Only jpeg still pictures are supported");
        return BAD_VALUE;
    }

	if (strcmp(params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv422i-yuyv") &&
		strcmp(params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv422sp") && 
		strcmp(params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv420sp") && 
		strcmp(params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv420p")) {
        LOGE("CameraHardware::setParameters: Unsupported format '%s' for recording",params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT));
        return BAD_VALUE;
	}	
	
    int w, h;

    params.getPreviewSize(&w, &h);
    LOGD("CameraHardware::setParameters: PREVIEW: Size %dx%d, %d fps, format: %s", w, h, params.getPreviewFrameRate(), params.getPreviewFormat());

    params.getPictureSize(&w, &h);
    LOGD("CameraHardware::setParameters: PICTURE: Size %dx%d, format: %s", w, h, params.getPictureFormat());

    params.getVideoSize(&w, &h);
    LOGD("CameraHardware::setParameters: VIDEO: Size %dx%d, format: %s", w, h, params.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT));
	
	// Store the new parameters
    mParameters = params;

	// Recreate the heaps if toggling recording changes the raw preview size
	//  and also restart the preview so we use the new size if needed
	initHeapLocked();
	
    LOGD("CameraHardware::setParameters: OK");

    return NO_ERROR;
}

/* A dumb variable indicating "no params" / error on the exit from
 * EmulatedCamera::getParameters(). */
static char lNoParam = '\0';
char* CameraHardware::getParameters()
{
    LOGD("CameraHardware::getParameters");

    String8 params;
    {
        Mutex::Autolock lock(mLock);
		params = mParameters.flatten();
    }
    
    char* ret_str =
        reinterpret_cast<char*>(malloc(sizeof(char) * (params.length()+1)));
    memset(ret_str, 0, params.length()+1);
    if (ret_str != NULL) {
        strncpy(ret_str, params.string(), params.length()+1);
        return ret_str;
    }

	LOGE("%s: Unable to allocate string for %s", __FUNCTION__, params.string());
	/* Apparently, we can't return NULL fron this routine. */
	return &lNoParam;
}

void CameraHardware::putParameters(char* params)
{
	LOGD("CameraHardware::putParameters");
    /* This method simply frees parameters allocated in getParameters(). */
    if (params != NULL && params != &lNoParam) {
        free(params);
    }
}

status_t CameraHardware::sendCommand(int32_t command, int32_t arg1, int32_t arg2)
{
    LOGD("CameraHardware::sendCommand");
    return 0;
}

void CameraHardware::releaseCamera()
{
    LOGD("CameraHardware::releaseCamera");
    if (mPreviewThread != 0) {
        stopPreview();
    }
}

status_t CameraHardware::dumpCamera(int fd)
{
    LOGD("dump");
    return -EINVAL;
}

// ---------------------------------------------------------------------------

void CameraHardware::initDefaultParameters()
{
    LOGD("CameraHardware::initDefaultParameters");
	
	CameraParameters p;
	unsigned int i;
	
	int pw = MIN_WIDTH;
	int ph = MIN_HEIGHT;
	int pfps = 30;
	int fw = MIN_WIDTH;
	int fh = MIN_HEIGHT;
	SortedVector<SurfaceSize> avSizes;
	SortedVector<int> avFps;
	
    if (camera.Open(VIDEO_DEVICE) != NO_ERROR) {
	    LOGE("cannot open device.");

    } else {
	
		// Get the default preview format
		pw = camera.getBestPreviewFmt().getWidth();
		ph = camera.getBestPreviewFmt().getHeight();
		pfps = camera.getBestPreviewFmt().getFps();
		
		// Get the default picture format
		fw = camera.getBestPictureFmt().getWidth();
		fh = camera.getBestPictureFmt().getHeight();

		// Get all the available sizes
		avSizes = camera.getAvailableSizes();
		
		// Add some sizes that some specific apps expect to find:
		//  GTalk expects 320x200
		//  Fring expects 240x160
		// And also add standard resolutions found in low end cameras, as 
		//  android apps could be expecting to find them
		// The V4LCamera handles those resolutions by choosing the next
		//  larger one and cropping the captured frames to the requested size
		
		avSizes.add(SurfaceSize(480,320)); // HVGA
		avSizes.add(SurfaceSize(432,320)); // 1.35-to-1, for photos. (Rounded up from 1.3333 to 1)
		avSizes.add(SurfaceSize(352,288)); // CIF
		avSizes.add(SurfaceSize(320,240)); // QVGA
		avSizes.add(SurfaceSize(320,200));
		avSizes.add(SurfaceSize(240,160)); // SQVGA
		avSizes.add(SurfaceSize(176,144)); // QCIF 
		
		// Get all the available Fps
		avFps = camera.getAvailableFps();
	}

	
	// Convert the sizes to text
	String8 szs("");
	for (i = 0; i < avSizes.size(); i++)
	{
		char descr[32];
		SurfaceSize ss = avSizes[i];
		sprintf(descr,"%dx%d",ss.getWidth(),ss.getHeight());
		szs.append(descr);
		if (i < avSizes.size() - 1) 
		{
			szs.append(",");
		}
	}

	// Convert the fps to ranges in text
	String8 fpsranges("");
	for (i = 0; i < avFps.size(); i++)
	{
		char descr[32];
		int ss = avFps[i];
		sprintf(descr,"(%d,%d)",ss,ss);
		fpsranges.append(descr);
		if (i < avFps.size() - 1) 
		{
			fpsranges.append(",");
		}
	}

	// Convert the fps to text
	String8 fps("");
	for (i = 0; i < avFps.size(); i++)
	{
		char descr[32];
		int ss = avFps[i];
		sprintf(descr,"%d",ss);
		fps.append(descr);
		if (i < avFps.size() - 1) 
		{
			fps.append(",");
		}
	}
	
	LOGI("Default preview size: (%d x %d), fps:%d\n",pw,ph,pfps);
	LOGI("All available formats: %s",(const char*)szs);
	LOGI("All available fps: %s",(const char*)fpsranges);
	LOGI("Default picture size: (%d x %d)\n",fw,fh);

	// Now store the data
	
	// Antibanding
	p.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING,"auto");
	p.set(CameraParameters::KEY_ANTIBANDING,"auto");
	
	// Effects
	p.set(CameraParameters::KEY_SUPPORTED_EFFECTS,"none"); // "none,mono,sepia,negative,solarize"
	p.set(CameraParameters::KEY_EFFECT,"none");
	
	// Flash modes
	p.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES,"off");
	p.set(CameraParameters::KEY_FLASH_MODE,"off");
	
	// Focus modes
	p.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,"fixed");
	p.set(CameraParameters::KEY_FOCUS_MODE,"fixed");
	
#if 0
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT,0); 
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY,75);
	p.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,"0x0");
	p.set("jpeg-thumbnail-size","0x0");
	p.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH,0);
#endif
	
	// Picture - Only JPEG supported
	p.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,CameraParameters::PIXEL_FORMAT_JPEG); // ONLY jpeg
	p.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
	p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, szs);
	p.setPictureSize(fw,fh);
	p.set(CameraParameters::KEY_JPEG_QUALITY, 85);
	
	// Preview - Supporting yuv422i-yuyv,yuv422sp,yuv420sp, defaulting to yuv420sp, as that is the android Defacto default
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,"yuv422i-yuyv,yuv422sp,yuv420sp,yuv420p"); // All supported preview formats
	p.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV422SP); // For compatibility sake ... Default to the android standard
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, fpsranges);
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, fps);
	p.setPreviewFrameRate( pfps );
	p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, szs);
	p.setPreviewSize(pw,ph); 

	// Video - Supporting yuv422i-yuyv,yuv422sp,yuv420sp and defaulting to yuv420p
    p.set("video-size-values"/*CameraParameters::KEY_SUPPORTED_VIDEO_SIZES*/, szs);
    p.setVideoSize(pw,ph);
    p.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420P);
    p.set("preferred-preview-size-for-video", "640x480");
	
	// supported rotations
	p.set("rotation-values","0");
	p.set(CameraParameters::KEY_ROTATION,"0");
	
	// scenes modes
	p.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,"auto");
	p.set(CameraParameters::KEY_SCENE_MODE,"auto");
	
	// white balance
	p.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,"auto");
	p.set(CameraParameters::KEY_WHITE_BALANCE,"auto");

	// zoom
	p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED,"false");
	p.set("max-video-continuous-zoom", 0 );
	p.set(CameraParameters::KEY_ZOOM, "0");
    p.set(CameraParameters::KEY_MAX_ZOOM, "100");
    p.set(CameraParameters::KEY_ZOOM_RATIOS, "100");
    p.set(CameraParameters::KEY_ZOOM_SUPPORTED, "false");

    if (setParameters(p.flatten()) != NO_ERROR) {
        LOGE("CameraHardware::initDefaultParameters: Failed to set default parameters.");
    }
}

void CameraHardware::initHeapLocked()
{
    LOGD("CameraHardware::initHeapLocked");

    int preview_width, preview_height;
    int picture_width, picture_height;
	int video_width, video_height;

	if (!mRequestMemory) {	
		LOGE("No memory allocator available");
		return;
	}
	
	bool restart_preview = false;
	
    mParameters.getPreviewSize(&preview_width, &preview_height);
    mParameters.getPictureSize(&picture_width, &picture_height);
	mParameters.getVideoSize(&video_width, &video_height);

    LOGD("CameraHardware::initHeapLocked: preview size=%dx%d", preview_width, preview_height);
    LOGD("CameraHardware::initHeapLocked: picture size=%dx%d", picture_width, picture_height);
	LOGD("CameraHardware::initHeapLocked: video size=%dx%d", video_width, video_height);

	int how_raw_preview_big = 0;
	
	// If we are recording, use the recording video size instead of the preview size
	if (mRecordingEnabled && mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
		how_raw_preview_big = video_width * video_height << 1; 		// Raw preview heap always in YUYV
		
		// If something changed ...
		if (mRawPreviewWidth != video_width ||
			mRawPreviewHeight != video_height) {
			
			// Stop the preview thread if needed
			if (mPreviewThread != 0) {
				restart_preview	= true;
				stopPreviewLocked();
				LOGD("Stopping preview to allow changes");
			}
			
			// Store the new effective size
			mRawPreviewWidth = video_width;
			mRawPreviewHeight = video_height;
		}
		
	} else {
		how_raw_preview_big = preview_width * preview_height << 1; 	// Raw preview heap always in YUYV

		// If something changed ...
		if (mRawPreviewWidth != preview_width ||
			mRawPreviewHeight != preview_height) {

			// Stop the preview thread if needed
			if (mPreviewThread != 0) {
				restart_preview	= true;
				stopPreviewLocked();
				LOGD("Stopping preview to allow changes");
			}
		
			// Store the effective size
			mRawPreviewWidth = preview_width;
			mRawPreviewHeight = preview_height;
		}
	}
	
    if (how_raw_preview_big != mRawPreviewFrameSize) {

		// Stop the preview thread if needed
		if (!restart_preview && mPreviewThread != 0) {
			restart_preview	= true;
			stopPreviewLocked();
			LOGD("Stopping preview to allow changes");
		}

        mRawPreviewFrameSize = how_raw_preview_big;
		
        // Create raw picture heap.
		if (mRawPreviewHeap) {
			mRawPreviewHeap->release(mRawPreviewHeap);
			mRawPreviewHeap = NULL;
		}
		mRawPreviewBuffer = NULL;

		mRawPreviewHeap = mRequestMemory(-1,mRawPreviewFrameSize,1,mCallbackCookie);
		if (mRawPreviewHeap) { 
			mRawPreviewBuffer = mRawPreviewHeap->data;
		} else {
			LOGE("Unable to allocate memory for RawPreview");
		}
		
        LOGD("CameraHardware::initHeapLocked: Raw preview heap allocated");
    }
	

	int how_preview_big = 0;
	if (!strcmp(mParameters.getPreviewFormat(),"yuv422i-yuyv")) {
		mPreviewFmt = PIXEL_FORMAT_YCrCb_422_I;
		how_preview_big = preview_width * preview_height << 1; // 2 bytes per pixel
	} else
	if (!strcmp(mParameters.getPreviewFormat(),"yuv422sp")) {
		mPreviewFmt = PIXEL_FORMAT_YCbCr_422_SP;
		how_preview_big = (preview_width * preview_height * 3) >> 1; // 1.5 bytes per pixel
	} else
	if (!strcmp(mParameters.getPreviewFormat(),"yuv420sp")) {
		mPreviewFmt = PIXEL_FORMAT_YCbCr_420_SP;
		how_preview_big = (preview_width * preview_height * 3) >> 1; // 1.5 bytes per pixel
	} else
	if (!strcmp(mParameters.getPreviewFormat(),"yuv420p")) {
		mPreviewFmt = PIXEL_FORMAT_YV12;
		
		/*
		 * This format assumes
		 * - an even width
		 * - an even height
		 * - a horizontal stride multiple of 16 pixels
		 * - a vertical stride equal to the height
		 *
		 *   y_size = stride * height
		 *   c_size = ALIGN(stride/2, 16) * height/2
		 *   cr_offset = y_size
		 *   cb_offset = y_size + c_size 
		 *   size = y_size + c_size * 2
		 */
		 
		int stride 		= (preview_width + 15) & (-16); // Round to 16 pixels
		int y_size  	= stride * preview_height;
		int c_stride 	= ((stride >> 1) + 15) & (-16); // Round to 16 pixels
		int c_size		= c_stride * preview_height >> 1;
		int cr_offset	= y_size;
		int cb_offset	= y_size + c_size;
		int size		= y_size + (c_size << 1);
		
		how_preview_big = size;
	}
	
    if (how_preview_big != mPreviewFrameSize) {

		// Stop the preview thread if needed
		if (!restart_preview && mPreviewThread != 0) {
			restart_preview	= true;
			stopPreviewLocked();
			LOGD("Stopping preview to allow changes");
		}
	
        mPreviewFrameSize = how_preview_big;

        // Make a new mmap'ed heap that can be shared across processes.
        // use code below to test with pmem
		if (mPreviewHeap) {
			mPreviewHeap->release(mPreviewHeap);
			mPreviewHeap = NULL;
		}
		memset(mPreviewBuffer,0,sizeof(mPreviewBuffer));

		mPreviewHeap = mRequestMemory(-1,mPreviewFrameSize,kBufferCount,mCallbackCookie);
		if (mPreviewHeap) { 
			// Make an IMemory for each frame so that we can reuse them in callbacks.
			for (int i = 0; i < kBufferCount; i++) {
				mPreviewBuffer[i] = (char*)mPreviewHeap->data + (i * mPreviewFrameSize);
			}
		} else {
			LOGE("Unable to allocate memory for Preview");
		}
        
        LOGD("CameraHardware::initHeapLocked: preview heap allocated");
    }
	
	int how_recording_big = 0;
	if (!strcmp(mParameters.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv422i-yuyv")) {
		mRecFmt = PIXEL_FORMAT_YCrCb_422_I;
		how_recording_big = video_width * video_height << 1; // 2 bytes per pixel
	} else
	if (!strcmp(mParameters.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv422sp")) {
		mRecFmt = PIXEL_FORMAT_YCbCr_422_SP;
		how_recording_big = (video_width * video_height * 3) >> 1; // 1.5 bytes per pixel
	} else
	if (!strcmp(mParameters.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv420sp")) {
		mRecFmt = PIXEL_FORMAT_YCbCr_420_SP;
		how_recording_big = (video_width * video_height * 3) >> 1; // 1.5 bytes per pixel
	} else
	if (!strcmp(mParameters.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT),"yuv420p")) {
		mRecFmt = PIXEL_FORMAT_YV12;
		
		/*
		 * This format assumes
		 * - an even width
		 * - an even height
		 * - a horizontal stride multiple of 16 pixels
		 * - a vertical stride equal to the height
		 *
		 *   y_size = stride * height
		 *   c_size = ALIGN(stride/2, 16) * height/2
		 *   cr_offset = y_size
		 *   cb_offset = y_size + c_size 
		 *   size = y_size + c_size * 2
		 */
		 
		int stride 		= (video_width + 15) & (-16); 	// Round to 16 pixels
		int y_size  	= stride * video_height;
		int c_stride 	= ((stride >> 1) + 15) & (-16); // Round to 16 pixels
		int c_size		= c_stride * video_height >> 1;
		int cr_offset	= y_size;
		int cb_offset	= y_size + c_size;
		int size		= y_size + (c_size << 1);
		
		how_recording_big = size;
	}	
	
	if (how_recording_big != mRecordingFrameSize) {

		// Stop the preview thread if needed
		if (!restart_preview && mPreviewThread != 0) {
			restart_preview	= true;
			stopPreviewLocked();
			LOGD("Stopping preview to allow changes");
		}
	
        mRecordingFrameSize = how_recording_big;
	
		if (mRecordingHeap) {
			mRecordingHeap->release(mRecordingHeap);
			mRecordingHeap = NULL;
		}
		memset(mRecBuffers,0,sizeof(mRecBuffers));

		mRecordingHeap = mRequestMemory(-1,mRecordingFrameSize,kBufferCount,mCallbackCookie);
		if (mRecordingHeap) { 
			// Make an IMemory for each frame so that we can reuse them in callbacks.
			for (int i = 0; i < kBufferCount; i++) {
				mRecBuffers[i] = (char*)mRecordingHeap->data + (i * mRecordingFrameSize);
			}
		} else {
			LOGE("Unable to allocate memory for Recording");
		}
	
        LOGD("CameraHardware::initHeapLocked: recording heap allocated");
    }

	int how_picture_big = picture_width * picture_height << 1; // Raw picture heap always in YUYV
    if (how_picture_big != mRawPictureBufferSize) {
	
		// Picture does not need to stop the preview, as the mutex ensures
		//  the picture memory pool is not being used, and the camera is not
		//  capturing pictures right now
	
        mRawPictureBufferSize = how_picture_big;
	
        // Create raw picture heap.
		if (mRawPictureHeap) {
			mRawPictureHeap->release(mRawPictureHeap);
			mRawPictureHeap = NULL;
		}
		mRawBuffer = NULL;

		mRawPictureHeap = mRequestMemory(-1,mRawPictureBufferSize,1,mCallbackCookie);
		if (mRawPictureHeap) { 
			mRawBuffer = mRawPictureHeap->data;
		} else {
			LOGE("Unable to allocate memory for RawPicture");
		}
	
        LOGD("CameraHardware::initHeapLocked: Raw picture heap allocated");
    }

	int how_jpeg_big = picture_width * picture_height << 1; // jpeg maximum size
    if (how_jpeg_big != mJpegPictureBufferSize) {

		// Picture does not need to stop the preview, as the mutex ensures
		//  the picture memory pool is not being used, and the camera is not
		//  capturing pictures right now

        mJpegPictureBufferSize = how_jpeg_big;
		
        // Create Jpeg picture heap.
		if (mJpegPictureHeap) {
			mJpegPictureHeap->release(mJpegPictureHeap);
			mJpegPictureHeap = NULL;
		}
		mJpegPictureHeap = mRequestMemory(-1,how_jpeg_big,1,mCallbackCookie);
		if (!mJpegPictureHeap) { 
			LOGE("Unable to allocate memory for RawPicture");
		}

        LOGD("CameraHardware::initHeapLocked: Jpeg picture heap allocated");
    }

	// Don't forget to restart the preview if it was stopped...
	if (restart_preview) {
		LOGD("Restarting preview");
		startPreviewLocked();
	}
	
    LOGD("CameraHardware::initHeapLocked: OK");
}

int CameraHardware::previewThread()
{
    LOGD("CameraHardware::previewThread: this=%p",this);

    int previewFrameRate = mParameters.getPreviewFrameRate();

    // Calculate how long to wait between frames.
    int delay = (int)(1000000 / previewFrameRate);
	
	// Buffers to send messages
	int recBufferIdx = 0;
	int previewBufferIdx = 0;
	
	bool record = false;
	bool preview = false;

	// Get the current timestamp
	nsecs_t timestamp = systemTime(SYSTEM_TIME_MONOTONIC);

	// We must avoid a race condition here when destroying the thread...
	//  So, if we fail to lock the mutex, just retry a bit later, but
	//  let the android thread end if requested!
	if (mLock.tryLock() == NO_ERROR)
    {

		// If no raw preview buffer, we can't do anything...
		if (mRawPreviewBuffer == 0) {
			LOGE("No Raw preview buffer!");
			mLock.unlock();
			return NO_ERROR;
		}

        // Get the preview buffer for the current frame		
		// This is always valid, even if the client died -- the memory
		// is still mapped in our process.
		uint8_t *frame = (uint8_t *)mPreviewBuffer[mCurrentPreviewFrame];
		
		// If no preview buffer, we cant do anything...
		if (frame == 0) {
			LOGE("No preview buffer!");
			mLock.unlock();
			return NO_ERROR;
		}


		//  Get a pointer to the memory area to use... In case of previewing in YUV422I, we
		// can save a buffer copy by directly using the output buffer. But ONLY if NOT recording
		// or, in case of recording, when size matches
		uint8_t* rawBase = (mPreviewFmt == PIXEL_FORMAT_YCrCb_422_I && 
							(!mRecordingEnabled || mRawPreviewFrameSize == mPreviewFrameSize)) 
							? frame
							:(uint8_t*)mRawPreviewBuffer;
							
		// Grab a frame in the raw format YUYV
		camera.GrabRawFrame(rawBase, mRawPreviewFrameSize);

		// If the recording is enabled...
		if (mRecordingEnabled && mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
			//LOGD("CameraHardware::previewThread: posting video frame...");

			// Get the video size. We are warrantied here that the current capture
			// size IS exacty equal to the video size, as this condition is enforced
			// by this driver, that priorizes recording size over preview size requirements
			
			uint8_t *recFrame = (uint8_t *) mRecBuffers[mCurrentRecordingFrame];
			if (recFrame != 0) {

				// Convert from our raw frame to the one the Record requires
				switch (mRecFmt) {
				
				// Note: Apparently, Android's "YCbCr_422_SP" is merely an arbitrary label
				// The preview data comes in a YUV 4:2:0 format, with Y plane, then VU plane
				case PIXEL_FORMAT_YCbCr_422_SP:
					yuyv_to_yvu420sp(recFrame, mRawPreviewWidth, mRawPreviewHeight, rawBase, (mRawPreviewWidth<<1), mRawPreviewWidth, mRawPreviewHeight);
					break;
					
				case PIXEL_FORMAT_YCbCr_420_SP:
					yuyv_to_yvu420sp(recFrame, mRawPreviewWidth, mRawPreviewHeight, rawBase, (mRawPreviewWidth<<1), mRawPreviewWidth, mRawPreviewHeight);
					break;
				
				case PIXEL_FORMAT_YV12:
					/* OMX recorder needs YUV */
					yuyv_to_yuv420p(recFrame, mRawPreviewWidth, mRawPreviewHeight, rawBase, (mRawPreviewWidth<<1), mRawPreviewWidth, mRawPreviewHeight);
					break;
				
				case PIXEL_FORMAT_YCrCb_422_I:
					memcpy(recFrame, rawBase, mRecordingFrameSize);
					break; 
				}
				
				// Remember we must schedule the callback
				record = true;
				
				// Advance the buffer pointer.
				recBufferIdx = mCurrentRecordingFrame;
				mCurrentRecordingFrame = (mCurrentRecordingFrame + 1) % kBufferCount;
			}
		}

		if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
			//LOGD("CameraHardware::previewThread: posting preview frame...");

			// Here we could eventually have a problem: If we are recording, the recording size
			//  takes precedence over the preview size. So, the rawBase buffer could be of a 
			//  different size than the preview buffer. Handle this situation by centering/cropping
			//  if needed.
			
			// Get the preview size
			int width = 0, height = 0;
			mParameters.getPreviewSize(&width,&height);
			
			// Assume we will be able to copy at least those pixels
			int cwidth = width;
			int cheight = height;
			
			// If we are trying to display a preview larger than the effective capture, truncate to it
			if (cwidth > mRawPreviewWidth)
				cwidth = mRawPreviewWidth;
			if (cheight > mRawPreviewHeight)
				cheight = mRawPreviewHeight;

			// Convert from our raw frame to the one the Preview requires
			switch (mPreviewFmt) {
			
				// Note: Apparently, Android's "YCbCr_422_SP" is merely an arbitrary label
				// The preview data comes in a YUV 4:2:0 format, with Y plane, then VU plane
			case PIXEL_FORMAT_YCbCr_422_SP: // This is misused by android...
				yuyv_to_yvu420sp(frame, width, height, rawBase, (mRawPreviewWidth<<1), cwidth, cheight);
				break;
				
			case PIXEL_FORMAT_YCbCr_420_SP:
				yuyv_to_yvu420sp(frame, width, height, rawBase, (mRawPreviewWidth<<1), cwidth, cheight);
				break;

			case PIXEL_FORMAT_YV12:
				yuyv_to_yvu420p(frame, width, height, rawBase, (mRawPreviewWidth<<1), cwidth, cheight);
				break;
				
			case PIXEL_FORMAT_YCrCb_422_I:
				// Nothing to do here. Is is handled as a special case without buffer copies...
				//  but ONLY in special cases... Otherwise, handle the copy!
				if (mRecordingEnabled && mRawPreviewFrameSize != mPreviewFrameSize) {
					// We need to copy ... do it
					uint8_t* dst = frame;
					uint8_t* src = rawBase;
					int h;
					for (h = 0; h < cheight; h++) {
						memcpy(dst,src,cwidth<<1);
						dst += width << 1;
						src += mRawPreviewWidth<<1;
					}
				}
				break; 
				
			default:
				LOGE("Unhandled pixel format");

			}
			
			// Remember we must schedule the callback
			preview = true;
			
			// Advance the buffer pointer.
			previewBufferIdx = mCurrentPreviewFrame;
			mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % kBufferCount;
		}

		// Display the preview image
		fillPreviewWindow(rawBase, mRawPreviewWidth, mRawPreviewHeight);
		
		// Release the lock
		mLock.unlock();
		
    } else {
	
		// Delay a little ... and reattempt the lock on the next iteration
		delay >>= 7;
	}

	// We must schedule the callbacks Outside the lock, or the caller
	//  could call us and cause a deadlock!
	if (preview) {
	    mDataCb(CAMERA_MSG_PREVIEW_FRAME, mPreviewHeap, previewBufferIdx, NULL, mCallbackCookie);
	}
	
	if (record) {
		// Record callback uses a timestamped frame
        mDataCbTimestamp(timestamp, CAMERA_MSG_VIDEO_FRAME, mRecordingHeap, recBufferIdx, mCallbackCookie);
	}

    LOGD("previewThread OK");

    // Wait for it...
    usleep(delay);

    return NO_ERROR;
}

void CameraHardware::fillPreviewWindow(uint8_t* yuyv, int srcWidth, int srcHeight) 
{
	// Preview to a preview window...
	if (mWin == 0) {
		LOGE("%s: No preview window",__FUNCTION__);
		return;
	}
	
	// Get a videobuffer
	buffer_handle_t* buf = NULL;
	int stride = 0;
	status_t res = mWin->dequeue_buffer(mWin, &buf, &stride);
	if (res != NO_ERROR || buf == NULL) {
        LOGE("%s: Unable to dequeue preview window buffer: %d -> %s",
            __FUNCTION__, -res, strerror(-res));
        return;
	}

    /* Let the preview window to lock the buffer. */
    res = mWin->lock_buffer(mWin, buf);
    if (res != NO_ERROR) {
        LOGE("%s: Unable to lock preview window buffer: %d -> %s",
             __FUNCTION__, -res, strerror(-res));
        mWin->cancel_buffer(mWin, buf);
        return;
    }
		
    /* Now let the graphics framework to lock the buffer, and provide
     * us with the framebuffer data address. */
	void* vaddr = NULL;
    
    const Rect bounds(srcWidth, srcHeight);
    GraphicBufferMapper& grbuffer_mapper(GraphicBufferMapper::get());
    res = grbuffer_mapper.lock(*buf, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &vaddr);
    if (res != NO_ERROR || vaddr == NULL) {
        LOGE("%s: grbuffer_mapper.lock failure: %d -> %s",
             __FUNCTION__, res, strerror(res));
        mWin->cancel_buffer(mWin, buf);
        return;
    }
		
	// Calculate the source stride...
	int srcStride = srcWidth<<1;
	uint8_t* src  = (uint8_t*)yuyv;

	// Center into the preview surface if needed
	int xStart = (mPreviewWinWidth   - srcWidth ) >> 1;
	int yStart = (mPreviewWinHeight  - srcHeight) >> 1;

	// Make sure not to overflow the preview surface
	if (xStart < 0 || yStart < 0) {
		LOGE("Preview window is smaller than video preview size - Cropping image.");
		
		if (xStart < 0) {
			srcWidth += xStart;
			src += ((-xStart) >> 1) << 1; 		// Center the crop rectangle
			xStart = 0;
		}
		
		if (yStart < 0) {
			srcHeight += yStart;
			src += ((-yStart) >> 1) * srcStride; // Center the crop rectangle
			yStart = 0;
		}
	} 		
	
	// Calculate the bytes per pixel
	int bytesPerPixel = 2;
	if (mPreviewWinFmt == PIXEL_FORMAT_YCbCr_422_SP ||
		mPreviewWinFmt == PIXEL_FORMAT_YCbCr_420_SP ||
		mPreviewWinFmt == PIXEL_FORMAT_YV12 ||
		mPreviewWinFmt == PIXEL_FORMAT_YV16 ) {
		bytesPerPixel = 1; // Planar Y
	} else
	if (mPreviewWinFmt == PIXEL_FORMAT_RGB_888) {
		bytesPerPixel = 3;
	} else
	if (mPreviewWinFmt == PIXEL_FORMAT_RGBA_8888 ||
		mPreviewWinFmt == PIXEL_FORMAT_RGBX_8888 ||
		mPreviewWinFmt == PIXEL_FORMAT_BGRA_8888) {
		bytesPerPixel = 4;
	} else
	if (mPreviewWinFmt == PIXEL_FORMAT_YCrCb_422_I) {
		bytesPerPixel = 2;
	}

	LOGD("ANativeWindow: bits:%p, stride in pixels:%d, w:%d, h: %d, format: %d",vaddr,stride,mPreviewWinWidth,mPreviewWinHeight,mPreviewWinFmt);

	// Based on the destination pixel type, we must convert from YUYV to it
	int dstStride = bytesPerPixel * stride;
	uint8_t* dst  = ((uint8_t*)vaddr) + (xStart * bytesPerPixel) + (dstStride * yStart);

	switch (mPreviewWinFmt) {
	case PIXEL_FORMAT_YCbCr_422_SP: // This is misused by android...
		yuyv_to_yvu420sp( dst, dstStride, mPreviewWinHeight, src, srcStride, srcWidth, srcHeight);
		break;
		
	case PIXEL_FORMAT_YCbCr_420_SP:
		yuyv_to_yvu420sp( dst, dstStride, mPreviewWinHeight,src, srcStride, srcWidth, srcHeight);
		break;
		
	case PIXEL_FORMAT_YV12:
		yuyv_to_yvu420p( dst, dstStride, mPreviewWinHeight, src, srcStride, srcWidth, srcHeight);
		break;

	case PIXEL_FORMAT_YV16:
		yuyv_to_yvu422p( dst, dstStride, mPreviewWinHeight, src, srcStride, srcWidth, srcHeight);
		break;
		
	case PIXEL_FORMAT_YCrCb_422_I:
	{
		// We need to copy ... do it
		uint8_t* pdst = dst;
		uint8_t* psrc = src;
		int h;
		for (h = 0; h < srcHeight; h++) {
			memcpy(pdst,psrc,srcWidth<<1);
			pdst += dstStride;
			psrc += srcStride;
		}
		break; 
	}
	
	case PIXEL_FORMAT_RGB_888:
		yuyv_to_rgb24(src, srcStride, dst, dstStride, srcWidth, srcHeight);
		break;
			
	case PIXEL_FORMAT_RGBA_8888:
		yuyv_to_rgb32(src, srcStride, dst, dstStride, srcWidth, srcHeight);
		break;
			
	case PIXEL_FORMAT_RGBX_8888:
		yuyv_to_rgb32(src, srcStride, dst, dstStride, srcWidth, srcHeight);
		break;
			
	case PIXEL_FORMAT_BGRA_8888:
		yuyv_to_bgr32(src, srcStride, dst, dstStride, srcWidth, srcHeight);
		break; 				
		
	case PIXEL_FORMAT_RGB_565:
		yuyv_to_rgb565(src, srcStride, dst, dstStride, srcWidth, srcHeight);
		break;
		
	default:
		LOGE("Unhandled pixel format");
	}
				
	/* Show it. */
	mWin->enqueue_buffer(mWin, buf);
				
	// Post the filled buffer!
	grbuffer_mapper.unlock(*buf);
}

int CameraHardware::beginAutoFocusThread(void *cookie)
{
    LOGD("CameraHardware::beginAutoFocusThread");
    CameraHardware *c = (CameraHardware *)cookie;
    return c->autoFocusThread();
}

int CameraHardware::autoFocusThread()
{
    LOGD("CameraHardware::autoFocusThread");
    if (mMsgEnabled & CAMERA_MSG_FOCUS)
        mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
    return NO_ERROR;
}


int CameraHardware::beginPictureThread(void *cookie)
{
    LOGD("CameraHardware::beginPictureThread");
    CameraHardware *c = (CameraHardware *)cookie;
    return c->pictureThread();
}

int CameraHardware::pictureThread()
{
    LOGD("CameraHardware::pictureThread");

    bool raw = false;
    bool jpeg = false;
	bool shutter = false;
    {
        Mutex::Autolock lock(mLock);

        int w, h;
        mParameters.getPictureSize(&w, &h);
		LOGD("CameraHardware::pictureThread: taking picture of %dx%d", w, h);

		/* Make sure to remember if the shutter must be enabled or not */
		if (mMsgEnabled & CAMERA_MSG_SHUTTER) {
			shutter = true;
		}
		
		/* The camera application will restart preview ... */
        if (mPreviewThread != 0) {
            stopPreviewLocked();
        }

		LOGD("CameraHardware::pictureThread: taking picture (%d x %d)", w, h);

		if (camera.Open(VIDEO_DEVICE) == NO_ERROR) {
			camera.Init(w, h, 1);
			
			/* Retrieve the real size being used */
			camera.getSize(w,h);

			LOGD("CameraHardware::pictureThread: effective size: %dx%d",w, h);

			/* Store it as the picture size to use */
			mParameters.setPictureSize(w, h);

			/* And reinit the capture heap to reflect the real used size if needed */
			initHeapLocked();

			camera.StartStreaming();
			
			LOGD("CameraHardware::pictureThread: waiting until camera picture stabilizes...");
	
			int maxFramesToWait = 8;
			int luminanceStableFor = 0;
			int prevLuminance = 0;
			int prevDif = -1;
			int stride = w << 1;
			int thresh = (w >> 4) * (h >> 4) * 12; // 5% of full range
	
			while (maxFramesToWait > 0 && luminanceStableFor < 4) {
				uint8_t* ptr = (uint8_t *)mRawBuffer;
				
				// Get the image
				camera.GrabRawFrame(ptr, (w * h << 1)); // Always YUYV
			
				// luminance metering points
				int luminance = 0;
				for (int x = 0; x < (w<<1); x += 32) {
					for (int y = 0; y < h*stride; y += 16*stride) {
						luminance += ptr[y + x];
					}
				}
			  
				// Calculate variation of luminance
				int dif = prevLuminance - luminance;
				if (dif < 0) dif = -dif;
				prevLuminance = luminance;

				// Wait until variation is less than 5%
				if (dif > thresh) {
					luminanceStableFor = 1;
				} else {
					luminanceStableFor++;
				}
			    
				maxFramesToWait--;
	    
				LOGD("luminance: %4d, dif: %4d, thresh: %d, stableFor: %d, maxWait: %d", luminance, dif, thresh, luminanceStableFor, maxFramesToWait);
			}
	
			LOGD("CameraHardware::pictureThread: picture taken"); 			
			
			if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
								
				LOGD("CameraHardware::pictureThread: took raw picture");
				raw = true;
			}
			
	        if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
			
				int quality = mParameters.getInt(CameraParameters::KEY_JPEG_QUALITY);

				uint8_t* jpegBuff = (uint8_t*) malloc(mJpegPictureBufferSize);
				if (jpegBuff) {
					
					// Compress the raw captured image to our buffer
					int fileSize = yuyv_to_jpeg((uint8_t *)mRawBuffer, jpegBuff, mJpegPictureBufferSize, w, h, w << 1,quality);
					
					// Create a buffer with the exact compressed size
					if (mJpegPictureHeap) {
						mJpegPictureHeap->release(mJpegPictureHeap);
						mJpegPictureHeap = NULL;
					}

					mJpegPictureHeap = mRequestMemory(-1,fileSize,1,mCallbackCookie);
					if (mJpegPictureHeap) { 
						memcpy(mJpegPictureHeap->data,jpegBuff,fileSize);
						LOGD("CameraHardware::pictureThread: took jpeg picture compressed to %d bytes, q=%d", fileSize, quality);
						jpeg = true;				
					} else {
						LOGE("Unable to allocate memory for RawPicture");
					}
					free(jpegBuff);					
					
				} else {
				
					LOGE("Unable to allocate temporary memory for Jpeg compression");
				}
				
			}
			
			camera.Uninit();
			camera.StopStreaming();
			camera.Close();
		
		} else {
			LOGE("CameraHardware::pictureThread: failed to grab image");
		}
    }
	
	/* All this callbacks can potentially call one of our methods. 
	   Make sure to dispatch them OUTSIDE the lock! */
	if (shutter) {
		LOGD("Sending the Shutter message");
		mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
	}

    if (raw) {
		LOGD("Sending the raw message");
        mDataCb(CAMERA_MSG_RAW_IMAGE, mRawPictureHeap, 0, NULL, mCallbackCookie);
    }

    if (jpeg) {
		LOGD("Sending the jpeg message");
        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mJpegPictureHeap, 0, NULL, mCallbackCookie);
    }

    LOGD("CameraHardware::pictureThread OK");

    return NO_ERROR;
}

/****************************************************************************
 * Camera API callbacks as defined by camera_device_ops structure.
 *
 * Callbacks here simply dispatch the calls to an appropriate method inside
 * CameraHardware instance, defined by the 'dev' parameter.
 ***************************************************************************/

int CameraHardware::set_preview_window(struct camera_device* dev,
                                       struct preview_stream_ops* window)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setPreviewWindow(window);
}

void CameraHardware::set_callbacks(
        struct camera_device* dev,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void* user)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->setCallbacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);
}

void CameraHardware::enable_msg_type(struct camera_device* dev, int32_t msg_type)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->enableMsgType(msg_type);
}

void CameraHardware::disable_msg_type(struct camera_device* dev, int32_t msg_type)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->disableMsgType(msg_type);
}

int CameraHardware::msg_type_enabled(struct camera_device* dev, int32_t msg_type)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isMsgTypeEnabled(msg_type);
}

int CameraHardware::start_preview(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->startPreview();
}

void CameraHardware::stop_preview(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->stopPreview();
}

int CameraHardware::preview_enabled(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isPreviewEnabled();
}

int CameraHardware::store_meta_data_in_buffers(struct camera_device* dev,
                                               int enable)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->storeMetaDataInBuffers(enable);
}

int CameraHardware::start_recording(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->startRecording();
}

void CameraHardware::stop_recording(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->stopRecording();
}

int CameraHardware::recording_enabled(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isRecordingEnabled();
}

void CameraHardware::release_recording_frame(struct camera_device* dev,
                                             const void* opaque)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->releaseRecordingFrame(opaque);
}

int CameraHardware::auto_focus(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setAutoFocus();
}

int CameraHardware::cancel_auto_focus(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->cancelAutoFocus();
}

int CameraHardware::take_picture(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->takePicture();
}

int CameraHardware::cancel_picture(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->cancelPicture();
}

int CameraHardware::set_parameters(struct camera_device* dev, const char* parms)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setParameters(parms);
}

char* CameraHardware::get_parameters(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return NULL;
    }
    return ec->getParameters();
}

void CameraHardware::put_parameters(struct camera_device* dev, char* params)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->putParameters(params);
}

int CameraHardware::send_command(struct camera_device* dev,
                                 int32_t cmd,
                                 int32_t arg1,
                                 int32_t arg2)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->sendCommand(cmd, arg1, arg2);
}

void CameraHardware::release(struct camera_device* dev)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->releaseCamera();
}

int CameraHardware::dump(struct camera_device* dev, int fd)
{
    CameraHardware* ec = reinterpret_cast<CameraHardware*>(dev->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->dumpCamera(fd);
}

int CameraHardware::close(struct hw_device_t* device)
{
    CameraHardware* ec =
        reinterpret_cast<CameraHardware*>(reinterpret_cast<struct camera_device*>(device)->priv);
    if (ec == NULL) {
        LOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->closeCamera();
}

/****************************************************************************
 * Static initializer for the camera callback API
 ****************************************************************************/

camera_device_ops_t CameraHardware::mDeviceOps = {
    CameraHardware::set_preview_window,
    CameraHardware::set_callbacks,
    CameraHardware::enable_msg_type,
    CameraHardware::disable_msg_type,
    CameraHardware::msg_type_enabled,
    CameraHardware::start_preview,
    CameraHardware::stop_preview,
    CameraHardware::preview_enabled,
    CameraHardware::store_meta_data_in_buffers,
    CameraHardware::start_recording,
    CameraHardware::stop_recording,
    CameraHardware::recording_enabled,
    CameraHardware::release_recording_frame,
    CameraHardware::auto_focus,
    CameraHardware::cancel_auto_focus,
    CameraHardware::take_picture,
    CameraHardware::cancel_picture,
    CameraHardware::set_parameters,
    CameraHardware::get_parameters,
    CameraHardware::put_parameters,
    CameraHardware::send_command,
    CameraHardware::release,
    CameraHardware::dump
};

}; // namespace android













