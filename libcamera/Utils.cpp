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

#include "Utils.h"
extern "C" {
#include <malloc.h>
#include <string.h>
};

#define LOG_TAG "CameraHardware"
#include <utils/Log.h>

/*clip value between 0 and 255*/
#define CLIP(value) (uint8_t)(((value)>0xFF)?0xff:(((value)<0)?0:(value)))


/*jpeg decoding  420 planar to 422
* args: 
*      out: pointer to data output of idct (macroblocks yyyy u v)
*      pic: pointer to picture buffer (yuyv)
*      stride: picture stride
*/
static void yuv420pto422(int * out,uint8_t *pic,int stride)
{
	int j, k;
	uint8_t *pic0, *pic1;
	int *outy, *outu, *outv;
	int outy1 = 0;
	int outy2 = 8;

	//yyyyuv
	pic0 = pic;
	pic1 = pic + stride;
	outy = out;
	outu = out + 64 * 4;
	outv = out + 64 * 5;    
	for (j = 0; j < 8; j++) 
	{
		for (k = 0; k < 8; k++)
		{
			if( k == 4) 
			{ 
				outy1 += 56;
				outy2 += 56;
			}
			*pic0++ = CLIP(outy[outy1]);   //y1 line 1
			*pic0++ = CLIP(128 + *outu);   //u  line 1-2
			*pic0++ = CLIP(outy[outy1+1]); //y2 line 1
			*pic0++ = CLIP(128 + *outv);   //v  line 1-2
			*pic1++ = CLIP(outy[outy2]);   //y1 line 2
			*pic1++ = CLIP(128 + *outu);   //u  line 1-2
			*pic1++ = CLIP(outy[outy2+1]); //y2 line 2
			*pic1++ = CLIP(128 + *outv);   //v  line 1-2
			outy1 +=2; outy2 += 2; outu++; outv++;
		}
		if(j==3)
		{
			outy = out + 128;
		} 
		else 
		{
			outy += 16;
		}
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (stride -16);
		pic1 += 2 * (stride -16);
	}
}

/*jpeg decoding 422 planar to 422
* args: 
*      out: pointer to data output of idct (macroblocks yyyy u v)
*      pic: pointer to picture buffer (yuyv)
*      stride: picture stride
*/
static void yuv422pto422(int * out,uint8_t *pic,int stride)
{
	int j, k;
	uint8_t *pic0, *pic1;
	int *outy, *outu, *outv;
	int outy1 = 0;
	int outy2 = 8;
	int outu1 = 0;
	int outv1 = 0;
 
	//yyyyuv
	pic0 = pic;
	pic1 = pic + stride;
	outy = out;
	outu = out + 64 * 4;
	outv = out + 64 * 5;    
	for (j = 0; j < 4; j++) 
	{
		for (k = 0; k < 8; k++) 
		{
			if( k == 4)
			{ 
				outy1 += 56;
				outy2 += 56;
			}
			*pic0++ = CLIP(outy[outy1]);        //y1 line 1
			*pic0++ = CLIP(128 + outu[outu1]);  //u  line 1
			*pic0++ = CLIP(outy[outy1+1]);      //y2 line 1
			*pic0++ = CLIP(128 + outv[outv1]);  //v  line 1
			*pic1++ = CLIP(outy[outy2]);        //y1 line 2
			*pic1++ = CLIP(128 + outu[outu1+8]);//u  line 2
			*pic1++ = CLIP(outy[outy2+1]);      //y2 line 2
			*pic1++ = CLIP(128 + outv[outv1+8]);//v  line 2
			outv1 += 1; outu1 += 1;
			outy1 +=2; outy2 +=2;
		}
		outy += 16;outu +=8; outv +=8;
		outv1 = 0; outu1=0;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (stride -16);
		pic1 += 2 * (stride -16);
	}
}

/*use in utils.c for jpeg decoding 444 planar to 422
* args: 
*      out: pointer to data output of idct (macroblocks yyyy u v)
*      pic: pointer to picture buffer (yuyv)
*      stride: picture stride
*/
static void yuv444pto422(int * out,uint8_t *pic,int stride)
{
	int j, k;
	uint8_t *pic0, *pic1;
	int *outy, *outu, *outv;
	int outy1 = 0;
	int outy2 = 8;
	int outu1 = 0;
	int outv1 = 0;

	//yyyyuv
	pic0 = pic;
	pic1 = pic + stride;
	outy = out;
	outu = out + 64 * 4; // Ooops where did i invert ??
	outv = out + 64 * 5;    
	for (j = 0; j < 4; j++) 
	{
		for (k = 0; k < 4; k++) 
		{
			*pic0++ =CLIP( outy[outy1]);        //y1 line 1
			*pic0++ =CLIP( 128 + outu[outu1]);  //u  line 1
			*pic0++ =CLIP( outy[outy1+1]);      //y2 line 1
			*pic0++ =CLIP( 128 + outv[outv1]);  //v  line 1
			*pic1++ =CLIP( outy[outy2]);        //y1 line 2
			*pic1++ =CLIP( 128 + outu[outu1+8]);//u  line 2
			*pic1++ =CLIP( outy[outy2+1]);      //y2 line 2
			*pic1++ =CLIP( 128 + outv[outv1+8]);//v  line 2
			outv1 += 2; outu1 += 2;
			outy1 +=2; outy2 +=2;
		}
		outy += 16;outu +=16; outv +=16;
		outv1 = 0; outu1=0;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (stride -8);
		pic1 += 2 * (stride -8);
	}
}

/*use in utils.c for jpeg decoding 400 planar to 422
* args: 
*      out: pointer to data output of idct (macroblocks yyyy )
*      pic: pointer to picture buffer (yuyv)
*      stride: picture stride
*/
static void yuv400pto422(int * out,uint8_t *pic,int stride)
{
	int j, k;
	uint8_t *pic0, *pic1;
	int *outy ;
	int outy1 = 0;
	int outy2 = 8;
	pic0 = pic;
	pic1 = pic + stride;
	outy = out;

	//yyyy
	for (j = 0; j < 4; j++) 
	{
		for (k = 0; k < 4; k++) 
		{
			*pic0++ = CLIP(outy[outy1]);  //y1 line 1
			*pic0++ = 128 ;               //u
			*pic0++ = CLIP(outy[outy1+1]);//y2 line 1
			*pic0++ = 128 ;               //v
			*pic1++ = CLIP(outy[outy2]);  //y1 line 2
			*pic1++ = 128 ;               //u
			*pic1++ = CLIP(outy[outy2+1]);//y2 line 2
			*pic1++ = 128 ;               //v
			outy1 +=2; outy2 +=2;  
		}
		outy += 16;
		outy1 = 0;
		outy2 = 8;
		pic0 += 2 * (stride -8);
		pic1 += 2 * (stride -8);
	}
}


#define JPG_HUFFMAN_TABLE_LENGTH 0x01A0

static const unsigned char JPEGHuffmanTable[JPG_HUFFMAN_TABLE_LENGTH] = 
{
	// luminance dc - length bits	
	0x00, 
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	// luminance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 
	// chrominance dc - length bits	
	0x01, 
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	// chrominance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 
	// luminance ac - number of codes with # bits (ordered by code length 1-16)
	0x10,
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 
	0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
	// luminance ac - run size (ordered by code length)	
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
	0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32,
	0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
	0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
	0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
	0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94,
	0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
	0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
	0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
	0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
	0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
	0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA, 
	// chrominance ac -number of codes with # bits (ordered by code length 1-16)
	0x11, 
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
	0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
	// chrominance ac - run size (ordered by code length)
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06,
	0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81,
	0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
	0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
	0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
	0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
	0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92,
	0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
	0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
	0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
	0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
	0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
	0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA
}; 

/* Fixed point arithmetic */
//#define FIXED	Sint32
//#define FIXED_BITS 16
//#define TO_FIXED(X) (((Sint32)(X))<<(FIXED_BITS))
//#define FROM_FIXED(X) (((Sint32)(X))>>(FIXED_BITS))

#define ISHIFT 11
#define IFIX(a) ((int)((a) * (1 << ISHIFT) + .5))

/* special markers */
#define M_BADHUFF	-1
#define M_EOF		0x80

struct jpeg_decdata 
{
	int dcts[6 * 64 + 16];
	int out[64 * 6];
	int dquant[3][64];
};

struct in 
{
	uint8_t *p;
	uint32_t bits;
	int left;
	int marker;
	int (*func) (void *);
	void *data;
};

/*********************************/
#define DECBITS 10		/* seems to be the optimum */

struct dec_hufftbl 
{
	int maxcode[17];
	int valptr[16];
	uint8_t vals[256];
	uint32_t llvals[1 << DECBITS];
};

union hufftblp 
{
	struct dec_hufftbl *dhuff;
};

struct scan 
{
	int dc;			/* old dc value */

	union hufftblp hudc;
	union hufftblp huac;
	int next;		/* when to switch to next scan */

	int cid;		/* component id */
	int hv;			/* horiz/vert, copied from comp */
	int tq;			/* quant tbl, copied from comp */
};

/******** Markers *********/
#define M_SOI   0xd8
#define M_APP0  0xe0
#define M_DQT   0xdb
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_DRI   0xdd
#define M_SOS   0xda
#define M_RST0  0xd0
#define M_EOI   0xd9
#define M_COM   0xfe


/*********************************/

#undef PREC
#define PREC int


static int huffman_init(struct ctx* ctx);
static void decode_mcus (struct in *, int *, int, struct scan *, int *);
static int dec_readmarker (struct in *);
static void dec_makehuff (struct dec_hufftbl *, int *, uint8_t *);
static void setinput (struct in *, uint8_t *);
static void idctqtab(uint8_t *, PREC *);
inline static void idct(int *in, int *out, int *quant, long off, int max);
static int fillbits (struct in *, int, unsigned int);
static int dec_rec2 (struct in *, struct dec_hufftbl *, int *, int, int);


typedef void (*ftopict) (int * out, uint8_t *pic, int width) ;

/*********************************/

struct comp 
{
	int cid;
	int hv;
	int tq;
};

#define MAXCOMP 4
struct jpginfo 
{
	int nc;			/* number of components */
	int ns;			/* number of scans */
	int dri;		/* restart interval */
	int nm;			/* mcus til next marker */
	int rm;			/* next restart marker */
};

struct ctx {
	uint8_t *datap;
	struct jpginfo info;
	struct comp comps[MAXCOMP];
	struct scan dscans[MAXCOMP];
	uint8_t quant[4][64];
	struct dec_hufftbl dhuff[4];
	struct in in;
};


static inline int getbyte(struct ctx* ctx)
{
	return *ctx->datap++;
}

static int getword(struct ctx* ctx)
{
	int c1, c2;
	c1 = *ctx->datap++;
	c2 = *ctx->datap++;
	return c1 << 8 | c2;
}

#define dec_huffdc (ctx.dhuff + 0)
#define dec_huffac (ctx.dhuff + 2)

/*read jpeg tables (huffman and quantization)
* args: 
*      till: Marker (frame - SOF0   scan - SOS)
*      isDHT: flag indicating the presence of huffman tables (if 0 must use default ones - MJPG frame)
*/
static int readtables(struct ctx* ctx,int till, int *isDHT)
{
	int m, l, i, j, lq, pq, tq;
	int tc, th, tt;

	for (;;) 
	{
		if (getbyte(ctx)!= 0xff)
			return -1;
		if ((m = getbyte(ctx)) == till)
			break;

		switch (m) 
		{
			case 0xc2:
				return 0;
			/*read quantization tables (Lqt and Cqt)*/
			case M_DQT:
				lq = getword(ctx);
				while (lq > 2) 
				{
					pq = getbyte(ctx);
					/*Lqt=0x00   Cqt=0x01*/
					tq = pq & 15;
					if (tq > 3)
					return -1;
					pq >>= 4;
					if (pq != 0)
					return -1;
					for (i = 0; i < 64; i++)
						ctx->quant[tq][i] = getbyte(ctx);
					lq -= 64 + 1;
				}
				break;
			/*read huffman table*/
			case M_DHT:
				l = getword(ctx);
				while (l > 2) 
				{
					int hufflen[16], k;
					uint8_t huffvals[256];

					tc = getbyte(ctx);
					th = tc & 15;
					tc >>= 4;
					tt = tc * 2 + th;
					if (tc > 1 || th > 1)
					return -1;
					
					for (i = 0; i < 16; i++)
						hufflen[i] = getbyte(ctx);
					l -= 1 + 16;
					k = 0;
					for (i = 0; i < 16; i++) 
					{
						for (j = 0; j < hufflen[i]; j++)
							huffvals[k++] = getbyte(ctx);
						l -= hufflen[i];
					}
					dec_makehuff(ctx->dhuff + tt, hufflen, huffvals);
				}
				/* has huffman tables defined (JPEG)*/
				*isDHT= 1;
				break;
			/*restart interval*/
			case M_DRI:
				l = getword(ctx);
				ctx->info.dri = getword(ctx);
				break;

			default:
				l = getword(ctx);
				while (l-- > 2)
					getbyte(ctx);
				break;
		}
	}
	return 0;
}

static void dec_initscans(struct ctx* ctx)
{
	int i;

	ctx->info.nm = ctx->info.dri + 1;
	ctx->info.rm = M_RST0;
	for (i = 0; i < ctx->info.ns; i++)
		ctx->dscans[i].dc = 0;
}

static int dec_checkmarker(struct ctx* ctx)
{
	int i;

	if (dec_readmarker(&ctx->in) != ctx->info.rm)
		return -1;
	ctx->info.nm = ctx->info.dri;
	ctx->info.rm = (ctx->info.rm + 1) & ~0x08;
	for (i = 0; i < ctx->info.ns; i++)
		ctx->dscans[i].dc = 0;
	return 0;
}

/*jpeg decode
* args: 
*      pic:  pointer to picture data ( decoded image - yuyv format)
*      buf:  pointer to input data ( compressed jpeg )
*      with: picture width 
*      height: picture height
*/
int jpeg_decode(uint8_t *pic, int stride, uint8_t *buf, int width, int height)
{
	struct ctx ctx;
	struct jpeg_decdata *decdata;
	int i=0, j=0, m=0, tac=0, tdc=0;
	int intwidth=0, intheight=0;
	int mcusx=0, mcusy=0, mx=0, my=0;
	int ypitch=0 ,xpitch=0,x=0,y=0;
	int mb=0;
	int max[6];
	ftopict convert;
	int err = 0;
	int isInitHuffman = 0;
	decdata = (struct jpeg_decdata*) calloc(1, sizeof(struct jpeg_decdata));
	
	for(i=0;i<6;i++) 
		max[i]=0;
	
	if (!decdata) 
	{
		err = -1;
		goto error;
	}
	if (buf == NULL) 
	{
		err = -1;
		goto error;
	}
	ctx.datap = buf;
	/*check SOI (0xFFD8)*/
	if (getbyte(&ctx) != 0xff) 
	{
		err = ERR_NO_SOI;
		goto error;
	}
	if (getbyte(&ctx) != M_SOI) 
	{
		err = ERR_NO_SOI;
		goto error;
	}
	/*read tables - if exist, up to start frame marker (0xFFC0)*/
	if (readtables(&ctx,M_SOF0, &isInitHuffman)) 
	{
		err = ERR_BAD_TABLES;
		goto error;
	}
	getword(&ctx);     /*header lenght*/
	i = getbyte(&ctx); /*precision (8 bit)*/
	if (i != 8) 
	{
		err = ERR_NOT_8BIT;
		goto error;
	}
	intheight = getword(&ctx); /*height*/
	intwidth = getword(&ctx);  /*width */

	if ((intheight & 7) || (intwidth & 7)) /*must be even*/
	{
		err = ERR_BAD_WIDTH_OR_HEIGHT;
		goto error;
	}
	ctx.info.nc = getbyte(&ctx); /*number of components*/
	if (ctx.info.nc > MAXCOMP) 
	{
		err = ERR_TOO_MANY_COMPPS;
		goto error;
	}
	/*for each component*/
	for (i = 0; i < ctx.info.nc; i++) 
	{
		int h, v;
		ctx.comps[i].cid = getbyte(&ctx); /*component id*/
		ctx.comps[i].hv = getbyte(&ctx);
		v = ctx.comps[i].hv & 15; /*vertical sampling   */
		h = ctx.comps[i].hv >> 4; /*horizontal sampling */
		ctx.comps[i].tq = getbyte(&ctx); /*quantization table used*/
		if (h > 3 || v > 3) 
		{
			err = ERR_ILLEGAL_HV;
			goto error;
		}
		if (ctx.comps[i].tq > 3) 
		{
			err = ERR_QUANT_TABLE_SELECTOR;
			goto error;
		}
	}
	/*read tables - if exist, up to start of scan marker (0xFFDA)*/ 
	if (readtables(&ctx,M_SOS,&isInitHuffman)) 
	{
		err = ERR_BAD_TABLES;
		goto error;
	}
	getword(&ctx); /* header lenght */
	ctx.info.ns = getbyte(&ctx); /* number of scans */
	if (!ctx.info.ns)
	{
	LOGE("info ns %d/n",ctx.info.ns);
		err = ERR_NOT_YCBCR_221111;
		goto error;
	}
	/*for each scan*/
	for (i = 0; i < ctx.info.ns; i++) 
	{
		ctx.dscans[i].cid = getbyte(&ctx); /*component id*/
		tdc = getbyte(&ctx);
		tac = tdc & 15; /*ac table*/
		tdc >>= 4;      /*dc table*/
		if (tdc > 1 || tac > 1) 
		{
			err = ERR_QUANT_TABLE_SELECTOR;
			goto error;
		}
		for (j = 0; j < ctx.info.nc; j++)
			if (ctx.comps[j].cid == ctx.dscans[i].cid)
				break;
		if (j == ctx.info.nc) 
		{
			err = ERR_UNKNOWN_CID_IN_SCAN;
			goto error;
		}
		ctx.dscans[i].hv = ctx.comps[j].hv;
		ctx.dscans[i].tq = ctx.comps[j].tq;
		ctx.dscans[i].hudc.dhuff = dec_huffdc + tdc;
		ctx.dscans[i].huac.dhuff = dec_huffac + tac;
	}

	i = getbyte(&ctx); /*0 */
	j = getbyte(&ctx); /*63*/
	m = getbyte(&ctx); /*0 */

	if (i != 0 || j != 63 || m != 0) 
	{
		LOGE("hmm FW error,not seq DCT ??\n");
	}
	
	/*build huffman tables*/
	if(!isInitHuffman) 
	{
		if(huffman_init(&ctx) < 0)
			return -ERR_BAD_TABLES;
	}
	/*
	if (ctx->dscans[0].cid != 1 || ctx->dscans[1].cid != 2 || ctx->dscans[2].cid != 3) 
	{
		err = ERR_NOT_YCBCR_221111;
		goto error;
	}

	if (ctx->dscans[1].hv != 0x11 || ctx->dscans[2].hv != 0x11) 
	{
		err = ERR_NOT_YCBCR_221111;
		goto error;
	}
	*/
	/* if internal width and external are not the same or heigth too 
	and pic not allocated realloc the good size and mark the change 
	need 1 macroblock line more ?? */
	if (intwidth > width || intheight > height) 
	{
		return -ERR_BAD_WIDTH_OR_HEIGHT;
#if 0		
		width = intwidth;
		height = intheight;
		// BytesperPixel 2 yuyv , 3 rgb24 
		*pic = (uint8_t*) realloc( *pic, intwidth * (intheight + 8) * 2);
#endif
	}

	switch (ctx.dscans[0].hv) 
	{
		case 0x22: // 411
			mb=6;
			mcusx = width >> 4;
			mcusy = height >> 4;

			xpitch = 16 * 2;

			ypitch = 16 * stride;
			convert = yuv420pto422; //choose the right conversion function
			break;
		case 0x21: //422
			mb=4;
			mcusx = width >> 4;
			mcusy = height >> 3;

			xpitch = 16 * 2;

			ypitch = 8 * stride;
			convert = yuv422pto422; //choose the right conversion function
			break;
		case 0x11: //444
			mcusx = width >> 3;
			mcusy = height >> 3;

			xpitch = 8 * 2;

			ypitch = 8 * stride;
			if (ctx.info.ns==1) 
			{
				mb = 1;
				convert = yuv400pto422; //choose the right conversion function
			}
			else 
			{
				mb=3;
				convert = yuv444pto422; //choose the right conversion function
			}
			break;
		default:
			err = ERR_NOT_YCBCR_221111;
			goto error;
			break;
	}

	idctqtab(ctx.quant[ctx.dscans[0].tq], decdata->dquant[0]);
	idctqtab(ctx.quant[ctx.dscans[1].tq], decdata->dquant[1]);
	idctqtab(ctx.quant[ctx.dscans[2].tq], decdata->dquant[2]);
	setinput(&ctx.in, ctx.datap);
	dec_initscans(&ctx);

	ctx.dscans[0].next = 2;
	ctx.dscans[1].next = 1;
	ctx.dscans[2].next = 0;	/* 4xx encoding */
	for (my = 0,y=0; my < mcusy; my++,y+=ypitch) 
	{
		for (mx = 0,x=0; mx < mcusx; mx++,x+=xpitch) 
		{
			if (ctx.info.dri && !--ctx.info.nm)
				if (dec_checkmarker(&ctx)) 
				{
					err = ERR_WRONG_MARKER;
					goto error;
				}
			switch (mb)
			{
				case 6: 
					decode_mcus(&ctx.in, decdata->dcts, mb, ctx.dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					idct(decdata->dcts + 64, decdata->out + 64,
						decdata->dquant[0], IFIX(128.5), max[1]);
					idct(decdata->dcts + 128, decdata->out + 128,
						decdata->dquant[0], IFIX(128.5), max[2]);
					idct(decdata->dcts + 192, decdata->out + 192,
						decdata->dquant[0], IFIX(128.5), max[3]);
					idct(decdata->dcts + 256, decdata->out + 256,
						decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 320, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;
					
				case 4:
					decode_mcus(&ctx.in, decdata->dcts, mb, ctx.dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					idct(decdata->dcts + 64, decdata->out + 64,
						decdata->dquant[0], IFIX(128.5), max[1]);
					idct(decdata->dcts + 128, decdata->out + 256,
							decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 192, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;
					
				case 3:
					decode_mcus(&ctx.in, decdata->dcts, mb, ctx.dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);    
					idct(decdata->dcts + 64, decdata->out + 256,
						decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 128, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;
					
				case 1:
					decode_mcus(&ctx.in, decdata->dcts, mb, ctx.dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					break;
			} // switch enc411
			convert(decdata->out,pic+y+x,stride); //convert to 422
		}
	}

	m = dec_readmarker(&ctx.in);
	if (m != M_EOI) 
	{
		err = ERR_NO_EOI;
		goto error;
	}
	free(decdata);
	return 0;
error:
	free(decdata);
	return err;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/
static int huffman_init(struct ctx* ctx)
{
	int tc, th, tt;
	uint8_t *ptr= (uint8_t *) JPEGHuffmanTable ;
	int i, j, l;
	l = JPG_HUFFMAN_TABLE_LENGTH ;
	while (l > 0) 
	{
		int hufflen[16], k;
		uint8_t huffvals[256];

		tc = *ptr++;
		th = tc & 15;
		tc >>= 4;
		tt = tc * 2 + th;
		if (tc > 1 || th > 1)
			return -ERR_BAD_TABLES;
		for (i = 0; i < 16; i++)
			hufflen[i] = *ptr++;
		l -= 1 + 16;
		k = 0;
		for (i = 0; i < 16; i++) 
		{
			for (j = 0; j < hufflen[i]; j++)
				huffvals[k++] = *ptr++;
			l -= hufflen[i];
		}
		dec_makehuff(ctx->dhuff + tt, hufflen, huffvals);
	}
	return 0;
}


static void setinput(struct in *in, uint8_t *p)
{
	in->p = p;
	in->left = 0;
	in->bits = 0;
	in->marker = 0;
}

static int fillbits(struct in *in, int le, unsigned int bi)
{
	int b, m;

	if (in->marker) 
	{
		if (le <= 16)
			in->bits = bi << 16, le += 16;
		return le;
	}
	while (le <= 24) 
	{
		b = *in->p++;
		if (b == 0xff && (m = *in->p++) != 0) 
		{
			if (m == M_EOF) 
			{
				if (in->func && (m = in->func(in->data)) == 0)
					continue;
			}
			in->marker = m;
			if (le <= 16)
				bi = bi << 16, le += 16;
			break;
		}
		bi = bi << 8 | b;
		le += 8;
	}
	in->bits = bi;		/* tmp... 2 return values needed */
	return le;
}

static int dec_readmarker(struct in *in)
{
	int m;

	in->left = fillbits(in, in->left, in->bits);
	if ((m = in->marker) == 0)
		return 0;
	in->left = 0;
	in->marker = 0;
	return m;
}

#define LEBI_DCL	int le, bi
#define LEBI_GET(in)	(le = in->left, bi = in->bits)
#define LEBI_PUT(in)	(in->left = le, in->bits = bi)

#define GETBITS(in, n) (					\
  (le < (n) ? le = fillbits(in, le, bi), bi = in->bits : 0),	\
  (le -= (n)),							\
  bi >> le & ((1 << (n)) - 1)					\
)

#define UNGETBITS(in, n) (	\
  le += (n)			\
)


static int dec_rec2(struct in *in, struct dec_hufftbl *hu, int *runp, int c, int i)
{
	LEBI_DCL;

	LEBI_GET(in);
	if (i) 
	{
		UNGETBITS(in, i & 127);
		*runp = i >> 8 & 15;
		i >>= 16;
	}
	else
	{
		for (i = DECBITS;
		(c = ((c << 1) | GETBITS(in, 1))) >= (hu->maxcode[i]); i++);
		if (i >= 16) 
		{
			in->marker = M_BADHUFF;
			return 0;
		}
		i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
		*runp = i >> 4;
		i &= 15;
	}
	if (i == 0)
	{	/* sigh, 0xf0 is 11 bit */
		LEBI_PUT(in);
		return 0;
	}
	/* receive part */
	c = GETBITS(in, i);
	if (c < (1 << (i - 1)))
		c += (-1 << i) + 1;
	LEBI_PUT(in);
	return c;
}

#define DEC_REC(in, hu, r, i)	 (	\
  r = GETBITS(in, DECBITS),		\
  i = hu->llvals[r],			\
  i & 128 ?				\
    (					\
      UNGETBITS(in, i & 127),		\
      r = i >> 8 & 15,			\
      i >> 16				\
    )					\
  :					\
    (					\
      LEBI_PUT(in),			\
      i = dec_rec2(in, hu, &r, r, i),	\
      LEBI_GET(in),			\
      i					\
    )					\
)

static void decode_mcus(struct in *in, int *dct, int n, struct scan *sc ,int *maxp)
{
	struct dec_hufftbl *hu;
	int i = 0, r = 0, t = 0;
	LEBI_DCL;

	memset(dct, 0, n * 64 * sizeof(*dct));
	LEBI_GET(in);
	while (n-- > 0) 
	{
		hu = sc->hudc.dhuff;
		*dct++ = (sc->dc += DEC_REC(in, hu, r, t));

		hu = sc->huac.dhuff;
		i = 63;
		while (i > 0) 
		{
			t = DEC_REC(in, hu, r, t);
			if (t == 0 && r == 0) 
			{
				dct += i;
				break;
			}
			dct += r;
			*dct++ = t;
			i -= r + 1;
		}
		*maxp++ = 64 - i;
		if (n == sc->next)
		sc++;
	}
	LEBI_PUT(in);
}

static void dec_makehuff(struct dec_hufftbl *hu, int *hufflen, uint8_t *huffvals)
{
	int code, k, i, j, d, x, c, v;
	for (i = 0; i < (1 << DECBITS); i++)
		hu->llvals[i] = 0;

	/*
	* llvals layout:
	*
	* value v already known, run r, backup u bits:
	*  vvvvvvvvvvvvvvvv 0000 rrrr 1 uuuuuuu
	* value unknown, size b bits, run r, backup u bits:
	*  000000000000bbbb 0000 rrrr 0 uuuuuuu
	* value and size unknown:
	*  0000000000000000 0000 0000 0 0000000
	*/
	code = 0;
	k = 0;
	for (i = 0; i < 16; i++, code <<= 1)
	{	/* sizes */
		hu->valptr[i] = k;
		for (j = 0; j < hufflen[i]; j++) 
		{
			hu->vals[k] = *huffvals++;
			if (i < DECBITS) 
			{
				c = code << (DECBITS - 1 - i);
				v = hu->vals[k] & 0x0f;	/* size */
				for (d = 1 << (DECBITS - 1 - i); --d >= 0;)
				{
					if (v + i < DECBITS) 
					{	/* both fit in table */
						x = d >> (DECBITS - 1 - v - i);
						if (v && x < (1 << (v - 1)))
							x += (-1 << v) + 1;
						x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
							(DECBITS - (i + 1 + v)) | 128;
					} 
					else 
						x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
							(DECBITS - (i + 1));
					hu->llvals[c | d] = x;
				}
			}
			code++;
			k++;
		}
		hu->maxcode[i] = code;
	}
	hu->maxcode[16] = 0x20000;	/* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#define IMULT(a, b) (((a) * (b)) >> ISHIFT)
#define ITOINT(a) ((a) >> ISHIFT)

#define S22 ((PREC)IFIX(2 * 0.382683432))
#define C22 ((PREC)IFIX(2 * 0.923879532))
#define IC4 ((PREC)IFIX(1 / 0.707106781))

//zigzag order used by idct
static uint8_t zig2[64] = {
    0, 2, 3, 9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
    5, 7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
    1, 4, 8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
    6, 13, 17, 24, 32, 38, 47, 49
};

/*inverse dct for jpeg decoding
* args: 
*      in:  pointer to input data ( mcu - after huffman decoding)
*      out: pointer to data with output of idct (to be filled)
*      quant: pointer to quantization data tables
*      off: offset value (128.5 or 0.5)
*      max: maximum input mcu index?
*/
inline static void idct(int *in, int *out, int *quant, long off, int max)
{
	long t0, t1, t2, t3, t4, t5, t6, t7;	// t ;
	long tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	long tmp[64], *tmpp;
	int i, j, te;
	uint8_t *zig2p;

	t0 = off;
	if (max == 1) //single color mcu
	{
		t0 += in[0] * quant[0];     //only DC available
		for (i = 0; i < 64; i++)    // fill mcu with DC value
			out[i] = ITOINT(t0);
		return;
	}
	zig2p = zig2;
	tmpp = tmp;
	for (i = 0; i < 8; i++) //apply quantization table in zigzag order
	{
		j = *zig2p++;
		t0 += in[j] * (long) quant[j];
		j = *zig2p++;
		t5 = in[j] * (long) quant[j];
		j = *zig2p++;
		t2 = in[j] * (long) quant[j];
		j = *zig2p++;
		t7 = in[j] * (long) quant[j];
		j = *zig2p++;
		t1 = in[j] * (long) quant[j];
		j = *zig2p++;
		t4 = in[j] * (long) quant[j];
		j = *zig2p++;
		t3 = in[j] * (long) quant[j];
		j = *zig2p++;
		t6 = in[j] * (long) quant[j];


		if ((t1 | t2 | t3 | t4 | t5 | t6 | t7) == 0) 
		{
			tmpp[0 * 8] = t0; //DC
			tmpp[1 * 8] = t0;
			tmpp[2 * 8] = t0;
			tmpp[3 * 8] = t0;
			tmpp[4 * 8] = t0;
			tmpp[5 * 8] = t0;
			tmpp[6 * 8] = t0;
			tmpp[7 * 8] = t0;

			tmpp++;
			t0 = 0;
			continue;
		}
		//IDCT;
		tmp0 = t0 + t1;
		t1 = t0 - t1;
		tmp2 = t2 - t3;
		t3 = t2 + t3;
		tmp2 = IMULT(tmp2, IC4) - t3;
		tmp3 = tmp0 + t3;
		t3 = tmp0 - t3;
		tmp1 = t1 + tmp2;
		tmp2 = t1 - tmp2;
		tmp4 = t4 - t7;
		t7 = t4 + t7;
		tmp5 = t5 + t6;
		t6 = t5 - t6;
		tmp6 = tmp5 - t7;
		t7 = tmp5 + t7;
		tmp5 = IMULT(tmp6, IC4);
		tmp6 = IMULT((tmp4 + t6), S22);
		tmp4 = IMULT(tmp4, (C22 - S22)) + tmp6;
		t6 = IMULT(t6, (C22 + S22)) - tmp6;
		t6 = t6 - t7;
		t5 = tmp5 - t6;
		t4 = tmp4 - t5;

		tmpp[0 * 8] = tmp3 + t7;        //t0;
		tmpp[1 * 8] = tmp1 + t6;        //t1;
		tmpp[2 * 8] = tmp2 + t5;        //t2;
		tmpp[3 * 8] = t3 + t4;          //t3;
		tmpp[4 * 8] = t3 - t4;          //t4;
		tmpp[5 * 8] = tmp2 - t5;        //t5;
		tmpp[6 * 8] = tmp1 - t6;        //t6;
		tmpp[7 * 8] = tmp3 - t7;        //t7;
		tmpp++;
		t0 = 0;
	}
	for (i = 0, j = 0; i < 8; i++) 
	{
		t0 = tmp[j + 0];
		t1 = tmp[j + 1];
		t2 = tmp[j + 2];
		t3 = tmp[j + 3];
		t4 = tmp[j + 4];
		t5 = tmp[j + 5];
		t6 = tmp[j + 6];
		t7 = tmp[j + 7];
		if ((t1 | t2 | t3 | t4 | t5 | t6 | t7) == 0) 
		{
			te = ITOINT(t0);
			out[j + 0] = te;
			out[j + 1] = te;
			out[j + 2] = te;
			out[j + 3] = te;
			out[j + 4] = te;
			out[j + 5] = te;
			out[j + 6] = te;
			out[j + 7] = te;
			j += 8;
			continue;
		}
		//IDCT;
		tmp0 = t0 + t1;
		t1 = t0 - t1;
		tmp2 = t2 - t3;
		t3 = t2 + t3;
		tmp2 = IMULT(tmp2, IC4) - t3;
		tmp3 = tmp0 + t3;
		t3 = tmp0 - t3;
		tmp1 = t1 + tmp2;
		tmp2 = t1 - tmp2;
		tmp4 = t4 - t7;
		t7 = t4 + t7;
		tmp5 = t5 + t6;
		t6 = t5 - t6;
		tmp6 = tmp5 - t7;
		t7 = tmp5 + t7;
		tmp5 = IMULT(tmp6, IC4);
		tmp6 = IMULT((tmp4 + t6), S22);
		tmp4 = IMULT(tmp4, (C22 - S22)) + tmp6;
		t6 = IMULT(t6, (C22 + S22)) - tmp6;
		t6 = t6 - t7;
		t5 = tmp5 - t6;
		t4 = tmp4 - t5;

		out[j + 0] = ITOINT(tmp3 + t7);
		out[j + 1] = ITOINT(tmp1 + t6);
		out[j + 2] = ITOINT(tmp2 + t5);
		out[j + 3] = ITOINT(t3 + t4);
		out[j + 4] = ITOINT(t3 - t4);
		out[j + 5] = ITOINT(tmp2 - t5);
		out[j + 6] = ITOINT(tmp1 - t6);
		out[j + 7] = ITOINT(tmp3 - t7);
		j += 8;
	}
}

static uint8_t zig[64] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

//coef used in idct
static PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};

static void idctqtab(uint8_t *qin,PREC *qout)
{
	int i, j;

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
				IMULT(aaidct[i], aaidct[j]);
}

