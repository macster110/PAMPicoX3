// X3 C-API v2.0
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

#ifndef X3CMP_H_
#define X3CMP_H_

#ifndef X3DEFS
#define X3DEFS
typedef unsigned long	   ulong ;
typedef unsigned short  	ushort ;
typedef unsigned char	   uchar ;

#define MIN(a,b)    (((a)<(b)) ? (a) : (b))
#define MAX(a,b)    (((a)>(b)) ? (a) : (b))
#endif

// X3 loss-less audio compression default parameters */
#define	X3_DEF_N	      (16) //the block length - note that SoundTraps use 16 whilst the default here is 20
#define X3_DEF_NT      (3)
#define	X3_DEF_T	      {3,8,20}

// The Rice codes corresponding to each threshold. This definition is only
// informational. The actual codes used are defined in bpack.c
#define	X3_DEF_CODES	{0,1,3}

// Maximum block size - this is needed to pre-define a temporary buffer
// for filtering the input vector in x3compress. If larger block sizes may
// be used, a dynamic allocation might be preferable.
#define N_MAX       50

typedef struct {
	short    *data ;     // pointer to the data vector
	ushort   nsamps ;    // number of samples per channel
	ushort   nch ;       // number of channels
} XBuff ;

#ifdef __cplusplus
extern "C" {
#endif

extern int    X3_compress(XBuff *obuff, XBuff *ibuff, int N, int *T) ;
extern int    X3_uncompress(XBuff *obuff, XBuff *ibuff, int N) ;

#ifdef __cplusplus
}
#endif

#define X3_compress_def(out,in)    X3_compress(out,in,X3_DEF_N,NULL) ;
#define X3_uncompress_def(out,in)  X3_uncompress(out,in,X3_DEF_N) ;

#endif
