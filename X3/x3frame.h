// X3 C-API v2.0
#ifndef X3FRAME_H_
#define X3FRAME_H_
//	Copyright (C) 2012,2013 Mark Johnson
//
// This file is part of the X3 C-API, a low-complexity lossless
// audio compressor for underwater sound.
//
// The X3 C-API is free software: you can redistribute it 
// and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation, either version 3 of 
// the License, or any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file. If not, see <http://www.gnu.org/licenses/>.
//
//	x3.h  Definitions and prototypes for the X3 API
//	Last modified: March 2013

#include <stdio.h>
#include <string.h>
#include "x3cmp.h"


#ifndef X3DEFS
#define X3DEFS
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
#define MIN(a,b)    (((a)<(b)) ? (a) : (b))
#define MAX(a,b)    (((a)>(b)) ? (a) : (b))
#endif

// Maximum frame size - this is needed to pre-define a temporary buffer
// for processing frames.
#define MAXFRAME    (10000)

// length of string to allocate for standard header - can probably be a lot shorter
// but this will do.
#define X3HEADLEN 500

/*
 * Number of samples for X3 mini blocks.
 * NB. this is hard coded in many places at the moment - needs sorting out.
 */
#define X3BLOCKSIZE 20

// 16 bit start of frame key
#define X3_KEY      (30771)     // frame header key for X3 v2.0
// 64 bit start of file key for X3 archive files
#define X3_FILE_KEY ("X3ARCHIV")

// number of 16-bit words in the frame header
#define X3_HDRLEN   (10)

typedef struct {
	short KEY;    // pointer to the data vector
	short ID;     // source id
	short NCH;    // number of channels
	short NS;     // number of samples per channel
	short NBY;    // number of bytes in the data payload
	ulong T0;     // 64 bits of time code
	ulong T1;
	ushort CH;     // CRC-16 over the first 16 bytes of the header
	ushort CD;     // CRC-16 over the NB bytes of data
} XHdr;

#ifdef __cplusplus
extern "C" {
#endif

extern int x3frameheader(short *pb, int id, int nch, int ns, int nw, short *T,
		short cd);
extern int fwrite_short(FILE *fp, ushort *buff, int n);
extern int X3_writeaudioframe(FILE *fid, XBuff *ibuff, int id,
		ushort *timecode);
extern int X3_writemetaframe(FILE *fid, char *s, ushort *timecode);
extern int X3_readaudioframe(XBuff *obuff, FILE *fid, XHdr *hdr);
extern int X3_getnextframehdr(XHdr *h, FILE *fid);
extern FILE *X3_new(char *fname, int fs);

// xml message creation functions
extern int X3_prepareXMLheader(char* s, int sampleRate, int nChan, int blockSize);
extern int openxmlfield(char *s, const char *field, const char *arg);
extern int addxmlfield(char *s, const char *field, const char *arg,
		const char *value);
extern int closexmlfield(char *s, const char *field);


#ifdef __cplusplus
}
#endif
#endif
