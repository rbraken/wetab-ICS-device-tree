/* 
	libcamera: An implementation of the library required by Android OS 3.2 so
	it can access V4L2 devices as cameras.
 
    (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com>
	(C) 2011 RedScorpion 
	
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


#ifndef CONVERTER_H
#define CONVERTER_H

/* Converters from camera format to android format */
void yuyv_to_yvu420sp(uint8_t *dst,int dstStride, int dstHeight, uint8_t *src, int srcStride, int width, int height);

/* YV12: This is the format of choice for many software MPEG codecs. It comprises an NxM Y plane followed by (N/2)x(M/2) V and U planes. */
void yuyv_to_yvu420p(uint8_t *dst,int dstStride, int dstHeight, uint8_t *src, int srcStride, int width, int height);

/* YV12: This is the format of choice for many software MPEG codecs. It comprises an NxM Y plane followed by (N/2)x(M/2) U and V planes. */
void yuyv_to_yuv420p(uint8_t *dst,int dstStride, int dstHeight, uint8_t *src, int srcStride, int width, int height);

/* YV16: This format is basically a version of YV12 with higher chroma resolution. It comprises an NxM Y plane followed by (N/2)xM V and U planes. */
void yuyv_to_yvu422p(uint8_t *dst,int dstStride, int dstHeight, uint8_t *src, int srcStride, int width, int height);


/*convert yuyv to rgb24/32/565
* args: 
*      pyuv: pointer to buffer containing yuv data (yuyv)
*      prgb: pointer to buffer containing rgb24 data
*      width: picture width
*      height: picture height
*/
void yuyv_to_rgb565 (uint8_t *pyuv, int pyuvstride, uint8_t *prgb,int prgbstride, int width, int height);
void yuyv_to_rgb24 (uint8_t *pyuv, int pyuvstride, uint8_t *prgb,int prgbstride, int width, int height);
void yuyv_to_rgb32 (uint8_t *pyuv, int pyuvstride, uint8_t *prgb,int prgbstride, int width, int height);


/*convert yuyv to bgr24/32/565
* args: 
*      pyuv: pointer to buffer containing yuv data (yuyv)
*      prgb: pointer to buffer containing rgb24 data
*      width: picture width
*      height: picture height
*/
void yuyv_to_bgr565 (uint8_t *pyuv, int pyuvstride, uint8_t *pbgr, int pbgrstride, int width, int height);
void yuyv_to_bgr24 (uint8_t *pyuv, int pyuvstride, uint8_t *pbgr, int pbgrstride, int width, int height);
void yuyv_to_bgr32 (uint8_t *pyuv, int pyuvstride, uint8_t *pbgr, int pbgrstride, int width, int height);


/*convert yuv 420 planar (yu12) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yuv420 planar data frame
*      width: picture width
*      height: picture height
*/
void yuv420_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yvu 420 planar (yv12) to yuv 422 (yuyv)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yvu420 planar data frame
*      width: picture width
*      height: picture height
*/
void yvu420_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yuv 420 planar (uv interleaved) (nv12) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yuv420 (nv12) planar data frame
*      width: picture width
*      height: picture height
*/
void nv12_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yuv 420 planar (vu interleaved) (nv21) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yuv420 (nv21) planar data frame
*      width: picture width
*      height: picture height
*/
void nv21_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yuv 422 planar (uv interleaved) (nv16) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yuv422 (nv16) planar data frame
*      width: picture width
*      height: picture height
*/
void nv16_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yuv 422 planar (vu interleaved) (nv61) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yuv422 (nv61) planar data frame
*      width: picture width
*      height: picture height
*/
void nv61_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert y16 (grey) to yuyv (packed)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing y16 (grey) data frame
*      width: picture width
*      height: picture height
*/
void y16_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src,int srcStride, int width, int height);

/*convert yyuv to yuyv
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing a yyuv data frame
*      width: picture width
*      height: picture height
*/
void yyuv_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src,int srcStride, int width, int height);

/*convert uyvy (packed) to yuyv (packed)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing uyvy packed data frame
*      width: picture width
*      height: picture height
*/
void uyvy_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src,int srcStride, int width, int height);

/*convert yvyu (packed) to yuyv (packed)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing yvyu packed data frame
*      width: picture width
*      height: picture height
*/
void yvyu_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src,int srcStride, int width, int height);

/*convert yuv 411 packed (y41p) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing y41p data frame
*      width: picture width
*      height: picture height
*/
void y41p_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert yuv mono (grey) to yuv 422
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing grey (y only) data frame
*      width: picture width
*      height: picture height
*/
void grey_to_yuyv (uint8_t *dst,int dstStride, uint8_t *src,int srcStride, int width, int height);

/*convert SPCA501 (s501) to yuv 422
* s501  |Y0..width..Y0|U..width/2..U|Y1..width..Y1|V..width/2..V|
* signed values (-128;+127) must be converted to unsigned (0; 255)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing s501 data frame
*      width: picture width
*      height: picture height
*/
void s501_to_yuyv(uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert SPCA505 (s505) to yuv 422
* s505  |Y0..width..Y0|Y1..width..Y1|U..width/2..U|V..width/2..V|
* signed values (-128;+127) must be converted to unsigned (0; 255)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing s501 data frame
*      width: picture width
*      height: picture height
*/
void s505_to_yuyv(uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert SPCA508 (s508) to yuv 422
* s508  |Y0..width..Y0|U..width/2..U|V..width/2..V|Y1..width..Y1|
* signed values (-128;+127) must be converted to unsigned (0; 255)
* args: 
*      framebuffer: pointer to frame buffer (yuyv)
*      stride: stride of framebuffer
*      src: pointer to temp buffer containing s501 data frame
*      width: picture width
*      height: picture height
*/
void s508_to_yuyv(uint8_t *dst,int dstStride, uint8_t *src, int width, int height);

/*convert bayer raw data to rgb24
* args: 
*      pBay: pointer to buffer containing Raw bayer data data
*      pRGB24: pointer to buffer containing rgb24 data
*      width: picture width
*      height: picture height
*      pix_order: bayer pixel order (0=gb/rg   1=gr/bg  2=bg/gr  3=rg/bg)
*/
void bayer_to_rgb24(uint8_t *pBay, uint8_t *pRGB24, int width, int height, int pix_order);

/*convert rgb24 to yuyv
* args: 
*	   src: pointer to buffer containing rgb24 data
*      dst: pointer to buffer containing yuv data (yuyv)
*      stride: stride of framebuffer
*      width: picture width
*      height: picture height
*/
void rgb_to_yuyv(uint8_t *dst, int dstStride, uint8_t *src, int srcStride, int width, int height);

/*convert bgr24 to yuyv
* args: 
*      src: pointer to buffer containing bgr24 data
*      dst: pointer to buffer containing yuv data (yuyv)
*      stride: stride of framebuffer
*      width: picture width
*      height: picture height
*/
void bgr_to_yuyv(uint8_t *dst, int dstStride, uint8_t *src, int srcStride, int width, int height);

/* yuyv_to_jpeg
 *  converts an input image in the YUYV format into a jpeg image and puts
 * it in a memory buffer.
 */
int yuyv_to_jpeg(uint8_t* src, uint8_t* dst, int maxsize, int srcwidth, int srcheight, int srcstride, int quality);


#endif
