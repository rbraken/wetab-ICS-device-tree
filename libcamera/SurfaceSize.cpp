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


#include "SurfaceSize.h"

namespace android {

int SurfaceSize::compare(const SurfaceSize& other) const
{
	/* If bigger in both directions, is is the biggest */
	if (width > other.width &&
		height > other.height)
		return 1;
		
	/* If smaller in both directions, is is the smaller */
	if (width < other.width &&
		height < other.height)
		return -1;
		
	/* In the other cases, we sort by surface */
	return getArea() - other.getArea();
}

};

