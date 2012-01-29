/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Contains implementation of a class CameraFactory that manages cameras
 * available
 */

#define LOG_NDEBUG 0
#define LOG_TAG "Camera_Factory"
#include <cutils/log.h>
#include <cutils/properties.h>
#include "CameraFactory.h"

extern camera_module_t HAL_MODULE_INFO_SYM;

/* A global instance of CameraFactory is statically instantiated and
 * initialized when camera HAL is loaded.
 */
android::CameraFactory  gCameraFactory;

namespace android {

CameraFactory::CameraFactory()
        : mCamera(NULL)
{
	LOGD("CameraFactory::CameraFactory");
}

CameraFactory::~CameraFactory()
{
	LOGD("CameraFactory::~CameraFactory");
    if (mCamera != NULL) {
        delete mCamera;
		mCamera = NULL;
    }
}

/****************************************************************************
 * Camera HAL API handlers.
 *
 * Each handler simply verifies existence of an appropriate Camera
 * instance, and dispatches the call to that instance.
 *
 ***************************************************************************/

int CameraFactory::cameraDeviceOpen(const hw_module_t* module,int camera_id, hw_device_t** device)
{
    LOGD("CameraFactory::cameraDeviceOpen: id = %d", camera_id);

    *device = NULL;

    if (camera_id < 0 || camera_id >= getCameraNum()) {
        LOGE("%s: Camera id %d is out of bounds (%d)",
             __FUNCTION__, camera_id, getCameraNum());
        return -EINVAL;
    }
	
	if (!mCamera)
		mCamera = new CameraHardware(module);	

    return mCamera->connectCamera(device);
}

/* Returns the number of available cameras */
int CameraFactory::getCameraNum()
{
	LOGD("CameraFactory::getCameraNum");
	return 1; //RvdB
}


int CameraFactory::getCameraInfo(int camera_id, struct camera_info* info)
{
    LOGD("CameraFactory::getCameraInfo: id = %d,info = %p", camera_id,info);

    if (camera_id < 0 || camera_id >= getCameraNum()) {
        LOGE("%s: Camera id %d is out of bounds (%d)",
             __FUNCTION__, camera_id, getCameraNum());
        return -EINVAL;
    }
	

	LOGD("CameraFactory::getCameraInfo: about to fetch info");
    return CameraHardware::getCameraInfo(info);
}

/****************************************************************************
 * Camera HAL API callbacks.
 ***************************************************************************/

int CameraFactory::device_open(const hw_module_t* module,
                                       const char* name,
                                       hw_device_t** device)
{
	LOGD("CameraFactory::device_open: name = %s", name);
	
    /*
     * Simply verify the parameters, and dispatch the call inside the
     * CameraFactory instance.
     */

    if (module != &HAL_MODULE_INFO_SYM.common) {
        LOGE("%s: Invalid module %p expected %p",
             __FUNCTION__, module, &HAL_MODULE_INFO_SYM.common);
        return -EINVAL;
    }
    if (name == NULL) {
        LOGE("%s: NULL name is not expected here", __FUNCTION__);
        return -EINVAL;
    }

    return gCameraFactory.cameraDeviceOpen(module,atoi(name), device);
}

int CameraFactory::get_number_of_cameras(void)
{
	LOGD("CameraFactory::get_number_of_cameras");
    return gCameraFactory.getCameraNum();
}

int CameraFactory::get_camera_info(int camera_id,
                                           struct camera_info* info)
{
	LOGD("CameraFactory::get_camera_info");
    return gCameraFactory.getCameraInfo(camera_id, info);
}

/********************************************************************************
 * Initializer for the static member structure.
 *******************************************************************************/

/* Entry point for camera HAL API. */
struct hw_module_methods_t CameraFactory::mCameraModuleMethods = {
    open: CameraFactory::device_open
};

}; /* namespace android */
