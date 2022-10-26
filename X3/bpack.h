// X3 C-API v2.0
#ifndef BPACK_H_
#define BPACK_H_
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
//	bpack.h  bit packing and unpacking functions
//	Last modified: March 2013

#ifndef X3DEFS
#define X3DEFS
  typedef unsigned short	ushort ;
  typedef unsigned char	uchar ;
  #define MIN(a,b)    (((a)<(b)) ? (a) : (b))
#endif  

typedef struct {
	ushort	*buff ;      // pointer to next 16-bit word in the buffer
   long     curr ;       // word currently being packed or unpacked
	int	   nb ;         // number of bits remaining in current word
   int		nw ;         // number of words remaining in buffer
   int      nbuff ;      // total number of words in the buffer
	} BPack ;

extern void  packinit(BPack *b, ushort *buff, int nwords) ;
extern int   pack1(BPack *b, short in, int nbits) ;
extern int   packn(BPack *b, short *ibuff, int n, int nbits, int stride) ;
extern int   packr(BPack *b, short *ibuff, int n, int code, int stride) ;
extern int   packflush(BPack *b) ;

extern void  unpackinit(BPack *b, ushort *buff, int nwords) ;
extern short unpack1(BPack *b, int nbits) ;
extern int   unpackn(short *obuff, BPack *b, int n, int nbits, int stride) ;
extern int   unpackr(short *obuff, BPack *b, int n, int code, int stride) ;
extern void  fixsign(short *buff, int n, int nbits, int stride) ;

#endif
