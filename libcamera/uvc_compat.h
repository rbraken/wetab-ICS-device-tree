/* 
	libcamera: An implementation of the library required by Android OS 3.2 so
	it can access V4L2 devices as cameras.
 
    (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com>
	
	Based on several packages:
		- luvcview: Sdl video Usb Video Class grabber
			(C) 2005,2006,2007 Laurent Pinchart && Michel Xhaard

		- spcaview 
			(C) 2003,2004,2005,2006 Michel Xhaard
		
		- JPEG decoder from http://www.bootsplash.org/
			(C) August 2001 by Michael Schroeder, <mls@suse.de> 

		- libcamera V4L for Android 2.2
			(C) 2009 0xlab.org - http://0xlab.org/
			(C) 2010 SpectraCore Technologies
				Author: Venkat Raju <codredruids@spectracoretech.com>
				Based on a code from http://code.google.com/p/android-m912/downloads/detail?name=v4l2_camera_v2.patch
 
		- guvcview:  http://guvcview.berlios.de
			Paulo Assis <pj.assis@gmail.com>
			Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
	 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
 */


#ifndef _UVC_COMPAT_H
#define _UVC_COMPAT_H

#include <linux/version.h>
#include <linux/videodev2.h>

#ifndef VIDIOC_ENUM_FRAMESIZES

/*
 * Frame size and frame rate enumeration
 *
 * Included in Linux 2.6.19
 */
enum v4l2_frmsizetypes {
	V4L2_FRMSIZE_TYPE_DISCRETE	= 1,
	V4L2_FRMSIZE_TYPE_CONTINUOUS	= 2,
	V4L2_FRMSIZE_TYPE_STEPWISE	= 3,
};

struct v4l2_frmsize_discrete {
	__u32			width;		/* Frame width [pixel] */
	__u32			height;		/* Frame height [pixel] */
};

struct v4l2_frmsize_stepwise {
	__u32			min_width;	/* Minimum frame width [pixel] */
	__u32			max_width;	/* Maximum frame width [pixel] */
	__u32			step_width;	/* Frame width step size [pixel] */
	__u32			min_height;	/* Minimum frame height [pixel] */
	__u32			max_height;	/* Maximum frame height [pixel] */
	__u32			step_height;	/* Frame height step size [pixel] */
};

struct v4l2_frmsizeenum {
	__u32			index;		/* Frame size number */
	__u32			pixel_format;	/* Pixel format */
	__u32			type;		/* Frame size type the device supports. */

	union {					/* Frame size */
		struct v4l2_frmsize_discrete	discrete;
		struct v4l2_frmsize_stepwise	stepwise;
	};

	__u32   reserved[2];			/* Reserved space for future use */
};

#define VIDIOC_ENUM_FRAMESIZES		_IOWR('V', 74, struct v4l2_frmsizeenum)
#endif

#ifndef VIDIOC_ENUM_FRAMEINTERVALS

enum v4l2_frmivaltypes {
	V4L2_FRMIVAL_TYPE_DISCRETE	= 1,
	V4L2_FRMIVAL_TYPE_CONTINUOUS	= 2,
	V4L2_FRMIVAL_TYPE_STEPWISE	= 3,
};

struct v4l2_frmival_stepwise {
	struct v4l2_fract	min;		/* Minimum frame interval [s] */
	struct v4l2_fract	max;		/* Maximum frame interval [s] */
	struct v4l2_fract	step;		/* Frame interval step size [s] */
};

struct v4l2_frmivalenum {
	__u32			index;		/* Frame format index */
	__u32			pixel_format;	/* Pixel format */
	__u32			width;		/* Frame width */
	__u32			height;		/* Frame height */
	__u32			type;		/* Frame interval type the device supports. */

	union {					/* Frame interval */
		struct v4l2_fract		discrete;
		struct v4l2_frmival_stepwise	stepwise;
	};

	__u32	reserved[2];			/* Reserved space for future use */
};


#define VIDIOC_ENUM_FRAMEINTERVALS	_IOWR('V', 75, struct v4l2_frmivalenum)
#endif

#endif /* _UVC_COMPAT_H */

