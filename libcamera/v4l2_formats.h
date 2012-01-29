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

#ifndef V4L2_FORMATS_H
#define V4L2_FORMATS_H 

#include "uvc_compat.h"

/* (Patch) define all supported formats - already done in videodev2.h*/
#ifndef V4L2_PIX_FMT_MJPEG
#define V4L2_PIX_FMT_MJPEG  v4l2_fourcc('M', 'J', 'P', 'G') /*  MJPEG stream     */
#endif

#ifndef V4L2_PIX_FMT_JPEG
#define V4L2_PIX_FMT_JPEG  v4l2_fourcc('J', 'P', 'E', 'G')  /*  JPEG stream      */
#endif

#ifndef V4L2_PIX_FMT_YUYV
#define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y','U','Y','V')   /* YUV 4:2:2        */
#endif

#ifndef V4L2_PIX_FMT_YVYU
#define V4L2_PIX_FMT_YVYU    v4l2_fourcc('Y','V','Y','U')   /* YUV 4:2:2        */
#endif

#ifndef V4L2_PIX_FMT_UYVY
#define V4L2_PIX_FMT_UYVY    v4l2_fourcc('U','Y','V','Y')   /* YUV 4:2:2        */
#endif

#ifndef V4L2_PIX_FMT_YYUV
#define V4L2_PIX_FMT_YYUV    v4l2_fourcc('Y','Y','U','V')   /* YUV 4:2:2        */
#endif

#ifndef V4L2_PIX_FMT_YUV420
#define V4L2_PIX_FMT_YUV420  v4l2_fourcc('Y','U','1','2')   /* YUV 4:2:0 Planar  */
#endif

#ifndef V4L2_PIX_FMT_YVU420
#define V4L2_PIX_FMT_YVU420  v4l2_fourcc('Y','V','1','2')   /* YUV 4:2:0 Planar  */
#endif

#ifndef V4L2_PIX_FMT_NV12
#define V4L2_PIX_FMT_NV12  v4l2_fourcc('N','V','1','2')   /* YUV 4:2:0 Planar (u/v) interleaved */
#endif

#ifndef V4L2_PIX_FMT_NV21
#define V4L2_PIX_FMT_NV21  v4l2_fourcc('N','V','2','1')   /* YUV 4:2:0 Planar (v/u) interleaved */
#endif

#ifndef V4L2_PIX_FMT_NV16
#define V4L2_PIX_FMT_NV16  v4l2_fourcc('N','V','1','6')   /* YUV 4:2:2 Planar (u/v) interleaved */
#endif

#ifndef V4L2_PIX_FMT_NV61
#define V4L2_PIX_FMT_NV61  v4l2_fourcc('N','V','6','1')   /* YUV 4:2:2 Planar (v/u) interleaved */
#endif

#ifndef V4L2_PIX_FMT_Y41P
#define V4L2_PIX_FMT_Y41P  v4l2_fourcc('Y','4','1','P')    /* YUV 4:1:1          */
#endif

#ifndef V4L2_PIX_FMT_GREY
#define V4L2_PIX_FMT_GREY  v4l2_fourcc('G','R','E','Y')    /*      Y only       */
#endif

#ifndef V4L2_PIX_FMT_Y16
#define V4L2_PIX_FMT_Y16  v4l2_fourcc('Y','1','6',' ')    /*      Y only (16 bit)      */
#endif

#ifndef V4L2_PIX_FMT_SPCA501
#define V4L2_PIX_FMT_SPCA501 v4l2_fourcc('S','5','0','1')  /* YUYV - by line     */
#endif

#ifndef V4L2_PIX_FMT_SPCA505
#define V4L2_PIX_FMT_SPCA505 v4l2_fourcc('S','5','0','5')  /* YYUV - by line     */
#endif

#ifndef V4L2_PIX_FMT_SPCA508
#define V4L2_PIX_FMT_SPCA508 v4l2_fourcc('S','5','0','8')  /* YUVY - by line     */
#endif

#ifndef V4L2_PIX_FMT_SGBRG8
#define V4L2_PIX_FMT_SGBRG8  v4l2_fourcc('G', 'B', 'R', 'G') /* GBGB.. RGRG..    */
#endif

#ifndef V4L2_PIX_FMT_SGRBG8
#define V4L2_PIX_FMT_SGRBG8  v4l2_fourcc('G', 'R', 'B', 'G') /* GRGR.. BGBG..    */
#endif

#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B', 'A', '8', '1') /* BGBG.. GRGR..    */
#endif

#ifndef V4L2_PIX_FMT_SRGGB8
#define V4L2_PIX_FMT_SRGGB8  v4l2_fourcc('R', 'G', 'G', 'B') /* RGRG.. GBGB..    */
#endif

#ifndef V4L2_PIX_FMT_BGR24
#define V4L2_PIX_FMT_BGR24   v4l2_fourcc('B', 'G', 'R', '3') /* 24  BGR-8-8-8    */
#endif

#ifndef V4L2_PIX_FMT_RGB24
#define V4L2_PIX_FMT_RGB24   v4l2_fourcc('R', 'G', 'B', '3') /* 24  RGB-8-8-8    */
#endif

#endif
