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

#ifndef ANDROID_HARDWARE_CAMERA_HARDWARE_H
#define ANDROID_HARDWARE_CAMERA_HARDWARE_H

#include <camera/CameraParameters.h>
#include <hardware/camera.h>
#include <utils/threads.h>
#include <utils/threads.h>
#include "V4L2Camera.h"

namespace android {

class CameraHardware : public camera_device {

public:

    /* Actual handler for camera_device_ops_t::set_preview_window callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t setPreviewWindow(struct preview_stream_ops *window);
	
    /* Actual handler for camera_device_ops_t::set_callbacks callback.
     * NOTE: When this method is called the object is locked.
     */
	void 	 setCallbacks(camera_notify_callback notify_cb, 
								  camera_data_callback data_cb,
								  camera_data_timestamp_callback data_cb_timestamp,
								  camera_request_memory get_memory,
								  void* user);
	
    /* Actual handler for camera_device_ops_t::enable_msg_type callback.
     * NOTE: When this method is called the object is locked.
     */
    void enableMsgType(int32_t msg_type);

    /* Actual handler for camera_device_ops_t::disable_msg_type callback.
     * NOTE: When this method is called the object is locked.
     */
    void disableMsgType(int32_t msg_type);
	
    /* Actual handler for camera_device_ops_t::msg_type_enabled callback.
     * NOTE: When this method is called the object is locked.
     * Return:
     *  0 if message(s) is (are) disabled, != 0 if enabled.
     */
    int isMsgTypeEnabled(int32_t msg_type);

    /* Actual handler for camera_device_ops_t::start_preview callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t startPreview();

    /* Actual handler for camera_device_ops_t::stop_preview callback.
     * NOTE: When this method is called the object is locked.
     */
    void stopPreview();

    /* Actual handler for camera_device_ops_t::preview_enabled callback.
     * NOTE: When this method is called the object is locked.
     * Return:
     *  0 if preview is disabled, != 0 if enabled.
     */
    int isPreviewEnabled();
	
    /* Actual handler for camera_device_ops_t::store_meta_data_in_buffers callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t storeMetaDataInBuffers(int enable);

    /* Actual handler for camera_device_ops_t::start_recording callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t startRecording();

    /* Actual handler for camera_device_ops_t::stop_recording callback.
     * NOTE: When this method is called the object is locked.
     */
    void stopRecording();

    /* Actual handler for camera_device_ops_t::recording_enabled callback.
     * NOTE: When this method is called the object is locked.
     * Return:
     *  0 if recording is disabled, != 0 if enabled.
     */
    int isRecordingEnabled();
	
    /* Actual handler for camera_device_ops_t::release_recording_frame callback.
     * NOTE: When this method is called the object is locked.
     */
    void releaseRecordingFrame(const void* opaque);
	
    /* Actual handler for camera_device_ops_t::auto_focus callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t setAutoFocus();

    /* Actual handler for camera_device_ops_t::cancel_auto_focus callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t cancelAutoFocus();

    /* Actual handler for camera_device_ops_t::take_picture callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t takePicture();
	
    /* Actual handler for camera_device_ops_t::cancel_picture callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t cancelPicture();

    /* Actual handler for camera_device_ops_t::set_parameters callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t setParameters(const char* parms);

    /* Actual handler for camera_device_ops_t::get_parameters callback.
     * NOTE: When this method is called the object is locked.
     * Return:
     *  Flattened parameters string. The caller will free the buffer allocated
     *  for the string by calling camera_device_ops_t::put_parameters callback.
     */
    char* getParameters();

	/*
     * Called to free the string returned from camera_device_ops_t::get_parameters
     * callback. There is nothing more to it: the name of the callback is just
     * misleading.
     * NOTE: When this method is called the object is locked.
     */
    void putParameters(char* params);

    /* Actual handler for camera_device_ops_t::send_command callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

    /* Actual handler for camera_device_ops_t::release callback.
     * NOTE: When this method is called the object is locked.
     */
    void releaseCamera();
	
	
	status_t dumpCamera(int fd);

private:

	bool PowerOn();
	bool PowerOff();
	bool NegotiatePreviewFormat(struct preview_stream_ops* win);

public:
    /* Constructs Camera instance.
     * Param:
     *  cameraId - Zero based camera identifier, which is an index of the camera
     *      instance in camera factory's array.
     *  module - Emulated camera HAL module descriptor.
     */
    CameraHardware(const hw_module_t* module);

    /* Destructs EmulatedCamera instance. */
    virtual ~CameraHardware();
	
    /****************************************************************************
     * Camera API implementation
     ***************************************************************************/

public:
    /* Creates connection to the emulated camera device.
     * This method is called in response to hw_module_methods_t::open callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t connectCamera(hw_device_t** device);

    /* Closes connection to the emulated camera.
     * This method is called in response to camera_device::close callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    status_t closeCamera();

    /* Gets camera information.
     * This method is called in response to camera_module_t::get_camera_info
     * callback.
     * NOTE: When this method is called the object is locked.
     * Note that failures in this method are reported as negave EXXX statuses.
     */
    static status_t getCameraInfo(struct camera_info* info);
	
private:

    static const int kBufferCount = 4;

    void initDefaultParameters();
    void initHeapLocked();

	class PreviewThread : public Thread {
		CameraHardware* mHardware;
		
	public:
		PreviewThread(CameraHardware* hw);
		virtual void onFirstRef();
		virtual bool threadLoop();
	};

    status_t startPreviewLocked();
    void 	 stopPreviewLocked();
	
    int previewThread();

    static int beginAutoFocusThread(void *cookie);
    int autoFocusThread();

    static int beginPictureThread(void *cookie);
    int pictureThread();

    void fillPreviewWindow(uint8_t* yuyv, int srcWidth, int srcHeight);

    mutable Mutex       mLock;

    preview_stream_ops*	mWin;
	int					mPreviewWinFmt;
	int					mPreviewWinWidth;
	int					mPreviewWinHeight;

    CameraParameters    mParameters;


    camera_memory_t*  	mRawPreviewHeap;
	int					mRawPreviewFrameSize;
	void*			    mRawPreviewBuffer;
	int					mRawPreviewWidth;
	int					mRawPreviewHeight;
	
    camera_memory_t*  	mPreviewHeap;
	int                 mPreviewFrameSize;
	void*               mPreviewBuffer[kBufferCount];
	int					mPreviewFmt;
		
    camera_memory_t*  	mRawPictureHeap;
	void*			    mRawBuffer;
	int					mRawPictureBufferSize;
    
	camera_memory_t*  	mRecordingHeap;
    void*		        mRecBuffers[kBufferCount];
	int                 mRecordingFrameSize;
	int					mRecFmt;
	
    camera_memory_t*  	mJpegPictureHeap;
	int					mJpegPictureBufferSize;

    V4L2Camera          camera;
    bool                mRecordingEnabled;
    
    // protected by mLock
    sp<PreviewThread>   mPreviewThread;

    camera_notify_callback    	mNotifyCb;
    camera_data_callback      	mDataCb;
    camera_data_timestamp_callback mDataCbTimestamp;
	camera_request_memory mRequestMemory;
    void               *mCallbackCookie;

    int32_t             mMsgEnabled;

    // only used from PreviewThread
    int                 mCurrentPreviewFrame;
    int                 mCurrentRecordingFrame;
	
    /****************************************************************************
     * Camera API callbacks as defined by camera_device_ops structure.
     * See hardware/libhardware/include/hardware/camera.h for information on
     * each of these callbacks. Implemented in this class, these callbacks simply
     * dispatch the call into an instance of EmulatedCamera class defined by the
     * 'camera_device' parameter.
     ***************************************************************************/

private:
    static int set_preview_window(struct camera_device* dev,
                                   struct preview_stream_ops* window);
    static void set_callbacks(struct camera_device* dev,
                              camera_notify_callback notify_cb,
                              camera_data_callback data_cb,
                              camera_data_timestamp_callback data_cb_timestamp,
                              camera_request_memory get_memory,
                              void* user);
    static void enable_msg_type(struct camera_device* dev, int32_t msg_type);
    static void disable_msg_type(struct camera_device* dev, int32_t msg_type);
    static int msg_type_enabled(struct camera_device* dev, int32_t msg_type);
    static int start_preview(struct camera_device* dev);
    static void stop_preview(struct camera_device* dev);
    static int preview_enabled(struct camera_device* dev);
    static int store_meta_data_in_buffers(struct camera_device* dev, int enable);
    static int start_recording(struct camera_device* dev);
    static void stop_recording(struct camera_device* dev);
    static int recording_enabled(struct camera_device* dev);
    static void release_recording_frame(struct camera_device* dev,
                                        const void* opaque);
    static int auto_focus(struct camera_device* dev);
    static int cancel_auto_focus(struct camera_device* dev);
    static int take_picture(struct camera_device* dev);
    static int cancel_picture(struct camera_device* dev);
    static int set_parameters(struct camera_device* dev, const char* parms);
    static char* get_parameters(struct camera_device* dev);
    static void put_parameters(struct camera_device* dev, char* params);
    static int send_command(struct camera_device* dev,
                            int32_t cmd,
                            int32_t arg1,
                            int32_t arg2);
    static void release(struct camera_device* dev);
    static int dump(struct camera_device* dev, int fd);
    static int close(struct hw_device_t* device);
	
private:

    /* Registered callbacks implementing camera API. */
    static camera_device_ops_t      mDeviceOps;

};

}; // namespace android

#endif
