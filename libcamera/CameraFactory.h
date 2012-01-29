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

#ifndef HW_CAMERA_CAMERA_FACTORY_H
#define HW_CAMERA_CAMERA_FACTORY_H

#include <string.h>
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include "CameraHardware.h"

namespace android {

/*
 * Contains declaration of a class CameraFactory that manages cameras
 * available for the emulation. A global instance of this class is statically
 * instantiated and initialized when camera emulation HAL is loaded.
 */

/* Class CameraFactoryManages cameras available.
 *
 * When the global static instance of this class is created on the module load,
 * it enumerates cameras available for the emulation by connecting to the
 * emulator's 'camera' service. For every camera found out there it creates an
 * instance of an appropriate class, and stores it an in array of emulated
 * cameras. In addition to the cameras reported by the emulator, a fake camera
 * emulator is always created, so there is always at least one camera that is
 * available.
 *
 * Instance of this class is also used as the entry point for the camera HAL API,
 * including:
 *  - hw_module_methods_t::open entry point
 *  - camera_module_t::get_number_of_cameras entry point
 *  - camera_module_t::get_camera_info entry point
 *
 */
class CameraFactory {
public:
    /* Constructs CameraFactory instance.
     * In this constructor the factory will create and initialize a list of
     * emulated cameras. All errors that occur on this constructor are reported
     * via mConstructedOK data member of this class.
     */
    CameraFactory();

    /* Destructs CameraFactory instance. */
    ~CameraFactory();

    /****************************************************************************
     * Camera HAL API handlers.
     ***************************************************************************/

public:
    /* Opens (connects to) a camera device.
     * This method is called in response to hw_module_methods_t::open callback.
     */
    int cameraDeviceOpen(const hw_module_t* module,int camera_id, hw_device_t** device);

    /* Gets emulated camera information.
     * This method is called in response to camera_module_t::get_camera_info callback.
     */
    static int getCameraInfo(int camera_id, struct camera_info *info);

	
	/* Returns the number of available cameras */
	static int getCameraNum();
	
    /****************************************************************************
     * Camera HAL API callbacks.
     ***************************************************************************/

public:
    /* camera_module_t::get_number_of_cameras callback entry point. */
    static int get_number_of_cameras(void);

    /* camera_module_t::get_camera_info callback entry point. */
    static int get_camera_info(int camera_id, struct camera_info *info);

private:
    /* hw_module_methods_t::open callback entry point. */
    static int device_open(const hw_module_t* module,
                           const char* name,
                           hw_device_t** device);

private:

    /* Camera hardware */
    CameraHardware* mCamera;

public:
    /* Contains device open entry point, as required by HAL API. */
    static struct hw_module_methods_t   mCameraModuleMethods;
};

}; /* namespace android */

/* References the global CameraFactory instance. */
extern android::CameraFactory   gCameraFactory;

#endif  /* HW_EMULATOR_CAMERA_EMULATED_CAMERA_FACTORY_H */
