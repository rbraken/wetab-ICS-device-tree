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


#ifndef __SURFACEDESC_H
#define __SURFACEDESC_H

#include "SurfaceSize.h"

namespace android {

// Video Mode information
class SurfaceDesc {
protected:

	SurfaceSize sz;
	int fps;

public:	
	// Constructors
	SurfaceDesc() : fps(0) {}
	SurfaceDesc(const SurfaceDesc& v) : sz(v.sz), fps(v.fps) {}
	SurfaceDesc(int pwidth,int pheight, int pfps) :
		sz(pwidth,pheight), fps(pfps) {}
	
	// Assignment operators
	const SurfaceDesc& operator=(const SurfaceDesc& v) {
		sz = v.sz; fps = v.fps;
		return *this;
	}
	
	// Accessors
	inline int getWidth() const { return sz.getWidth(); }
	inline int getHeight() const { return sz.getHeight(); }
	inline const SurfaceSize& getSize() const { return sz; }
	inline void setSize( const SurfaceSize& psz) { sz = psz; }
	inline void setSize( int pw, int ph) { sz.set(pw,ph); }
	inline int getArea() const { return sz.getArea(); }
	inline int getFps() const { return fps; }
	inline void setFps( int pfps ) { fps = pfps; }

	// Comparison operators 
	int compare(const SurfaceDesc& other) const;

	inline bool operator<(const SurfaceDesc& other) const;
	inline bool operator<=(const SurfaceDesc& other) const;
	inline bool operator==(const SurfaceDesc& other) const;
	inline bool operator!=(const SurfaceDesc& other) const;
	inline bool operator>=(const SurfaceDesc& other) const;
	inline bool operator>(const SurfaceDesc& other) const;
	
};

inline int compare_type(const SurfaceDesc& lhs, const SurfaceDesc& rhs)
{
    return lhs.compare(rhs);
}

inline int strictly_order_type(const SurfaceDesc& lhs, const SurfaceDesc& rhs)
{
    return compare_type(lhs, rhs) < 0;
}

inline bool SurfaceDesc::operator<(const SurfaceDesc& other) const
{
    return compare(other) < 0;
}

inline bool SurfaceDesc::operator<=(const SurfaceDesc& other) const
{
    return compare(other) <= 0;
}

inline bool SurfaceDesc::operator==(const SurfaceDesc& other) const
{
    return compare(other) == 0;
}

inline bool SurfaceDesc::operator!=(const SurfaceDesc& other) const
{
    return compare(other) != 0;
}

inline bool SurfaceDesc::operator>=(const SurfaceDesc& other) const
{
    return compare(other) >= 0;
}

inline bool SurfaceDesc::operator>(const SurfaceDesc& other) const
{
    return compare(other) > 0;
}  

};

#endif