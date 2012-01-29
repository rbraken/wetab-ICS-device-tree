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


#ifndef __SURFSIZE_H
#define __SURFSIZE_H

namespace android {

class SurfaceSize {
protected:
	int width;
	int height;

public:	
	// Constructors
	SurfaceSize() : width(0), height(0) {}
	SurfaceSize(const SurfaceSize& v) : width(v.width), height(v.height) {}
	SurfaceSize(int pwidth,int pheight) :
		width(pwidth), height(pheight) {}
	
	// Assignment operators
	const SurfaceSize& operator=(const SurfaceSize& v) {
		width = v.width; height = v.height;
		return *this;
	}
	
	// Accessors
	inline int getWidth() const { return width; }
	inline int getHeight() const { return height; }
	inline int getArea() const { return width * height; }
	
	// Operators
	inline void set(int w,int h) { width = w; height = h; }

	// Comparison operators 
	int compare(const SurfaceSize& other) const;

	inline bool operator<(const SurfaceSize& other) const;
	inline bool operator<=(const SurfaceSize& other) const;
	inline bool operator==(const SurfaceSize& other) const;
	inline bool operator!=(const SurfaceSize& other) const;
	inline bool operator>=(const SurfaceSize& other) const;
	inline bool operator>(const SurfaceSize& other) const;
	
};

inline int compare_type(const SurfaceSize& lhs, const SurfaceSize& rhs)
{
    return lhs.compare(rhs);
}

inline int strictly_order_type(const SurfaceSize& lhs, const SurfaceSize& rhs)
{
    return compare_type(lhs, rhs) < 0;
}

inline bool SurfaceSize::operator<(const SurfaceSize& other) const
{
    return compare(other) < 0;
}

inline bool SurfaceSize::operator<=(const SurfaceSize& other) const
{
    return compare(other) <= 0;
}

inline bool SurfaceSize::operator==(const SurfaceSize& other) const
{
    return compare(other) == 0;
}

inline bool SurfaceSize::operator!=(const SurfaceSize& other) const
{
    return compare(other) != 0;
}

inline bool SurfaceSize::operator>=(const SurfaceSize& other) const
{
    return compare(other) >= 0;
}

inline bool SurfaceSize::operator>(const SurfaceSize& other) const
{
    return compare(other) > 0;
}  

};

#endif