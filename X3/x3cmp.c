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
//	x3cmp.c  Frame-level compression and uncompression functions
//	Last modified: March 2013

#include <stdio.h>
#include "x3cmp.h"
#include "bpack.h"
#include <stdlib.h>     /* abs */

int	x3blkencode(BPack *b, short *ip, int *T, int n, int nch) ;
int	x3blkdecode(short *op, BPack *b, short last, int n, int nch) ;
int   sdiffmaxs(short *op, short *ip, int n, int nch) ;
short integrate(short *op, short last, int n, int nch) ;

int      DEF_T[] = X3_DEF_T ;
short    DBUFF[N_MAX] ;

/**
 * Compression function. this can be called with relatively large blocks (e.g. 1000 samples) of
 * data. These then get divided up into smaller blocks (20 samples), each of which get's
 * it's own wee header to say which encoding it's used.
 * obuff is output buffer of compressed data
 * ibuff is input buffer of raw data
 * N is generally defaulted to 20 and seems to be the default block length
 * T defaults to NULL, then gets set to {3,8,20} are the thresholds for the different RICE encodings.
 * returns the number of words in the compressed buffer.
 */
int		X3_compress(XBuff *obuff, XBuff *ibuff, int N, int *T)
{
	// compress a frame of audio data
	ushort *ip;
	int    k, ns, nch ;
	BPack  b[1] ;

	if(T==NULL)
		T = DEF_T ;

	// initializations
	ns = ibuff->nsamps-1 ;			// number of samples per channel to pack after first sample
	nch = ibuff->nch ;				// number of channels
	ip = (ushort*) ibuff->data ;	         // retrieve the input buffer pointer
	packinit(b,(ushort*) obuff->data,obuff->nsamps) ;	         // initialize the bit packer

	// pack the first sample for each channel - it is pass-thru coded.
	packn(b,(short*) ip,nch,16,1) ;
	ip += nch ;

	// Now pack the frame block-by-block. Blocks for each channel are interleaved
	// in the packed data.
	while(ns>0) {			// while there is still data to compress
		int n = MIN(N,ns) ;			// compute block length
		for(k=0;k<nch;k++)  // decode n samples from each channel
			x3blkencode(b,(short*) ip+k,T,n, nch) ;
		ns -= n ;           // update number of samples remaining and input pointer
		ip += n*nch ;
	}

	obuff->nsamps = packflush(b) ;
	return(obuff->nsamps) ;
}


int    X3_uncompress(XBuff *obuff, XBuff *ibuff, int N)
{
	// decode an X3 encoded data chunk
	ushort *op ;
	int    k, ns, nch ;
	BPack  b[1] ;

	// initializations
	unpackinit(b,(ushort*) ibuff->data,ibuff->nsamps) ;	      // initialize the bit unpacker
	ns = obuff->nsamps ;			// number of samples per channel to unpack
	nch = obuff->nch ;				// number of channels
	op = (ushort*) obuff->data ;	         // retrieve the output buffer pointer

	// unpack the first sample for each channel - it is pass-thru coded.
	unpackn((short*) op,b,nch,16,1) ;
	// fixsign(op,nch,16,1) ;
	--ns ;
	op += nch ;

	// Now unpack the frame block-by-block. Blocks for each channel are interleaved
	// in the packed data.
	while(ns>0) {			// while there is still data to decode
		int n = MIN(N,ns) ;			// compute block length
		for(k=0;k<nch;k++)  // decode n samples from each channel
			n = x3blkdecode((short*)  op+k,b,*(op-nch+k),n,nch) ;

		ns -= n ;           // update number of samples remaining and output pointer
		op += n*nch ;
	}

	return(0) ;
}

/**
 * The main worker which compresses a short buffer of n samples of
 * data. A single channel is compresses, though the buffer ip may
 * point to a multi channel buffer of interleaved data.
 * b = buffer that data get packed into
 * ip pointer to first sample of the data to be packed for the correct channel
 * T Rice thresholds
 * n number of samples to pack
 * nch number of channels of interleaved data.
 */
int		x3blkencode(BPack *b, short *ip, int *T, int n, int nch)
{
	int  nb, ma ;
	// filter the n incoming samples to a temporary buffer and find the maximum
	// magnitude. Depending on the magnitude, code the frame with a variable-
	// length code, a block floating point code, or pack it as an uncoded key
	// frame, i.e., if ma = max(abs(diff(x)))
	//   if ma>=BIG
	//			raw_encode blk			- raw encode undifferentiated stream
	//   elseif ma < X3_THRESH
	//			rice_encode blk		- rice encode filtered stream
	//   else
	//			bfp_encode blk       - block floating point encode filtered stream

	ma = sdiffmaxs(DBUFF,ip,n,nch) ;	 // apply filter and get max block magnitude
//	if (ma < 256) {
//		ma = 256; // force no compression.
//	}
//	if (ma != 104) {
//		printf("Max diff signal = %d\n", ma);
//		fflush(stdout);
//	}
	// sdiffmaxs retrieved a single channel of data.

	if(ma <= T[2]) {			 // encode frame with variable-length Rice code
		int c = (ma > *T) + (ma > *(T+1)) ; // get the code index, 0, 1 or 2;
		pack1(b,c+1,2) ;         // add 2 bit Rice header to the bit stream
		packr(b,DBUFF,n,c,1) ;     // code n words to the bit stream
		return(0) ;
	}

	// block-floating point or pass-through encode the block
	for(nb=0; ma>0; nb++, ma>>=1) ;  // find the number of bits needed to code ma
	if(nb>=15) {
		pack1(b,15,6) ;                 // add 6 bit BFP header to the bit stream
		packn(b,ip,n,16,nch) ;		     // pack 16 bit integers from source
	}
	else {
		pack1(b,nb,6) ;                 // add 6 bit BFP header to the bit stream
		packn(b,DBUFF,n,nb+1,1) ;	  // pack nb-bit filtered integers with an extra bit for a sign.
	}

	return(0) ;
}


int	x3blkdecode(short *op, BPack *b, short last, int n, int nch)
{
	// unpack and decode one block from one channel
	short	d ;
	int		err, nb ;

	// unpack the n samples in the block
	d = unpack1(b,2) ;	   // first get the code select header (2 bits)
	if(d == 0)	{					   // it is a bfp or pass thru block
		nb = (int)unpack1(b,4) ;	 // get the 4-bit bfp exponent
		if(nb>0)                  // it is a valid bfp header
			++nb ;                       // number of bits/word is one more than the exponent

		else {
			n = (int)(unpack1(b,6)+1) ;	// number of words in the frame
			// now get the block type bit
			d = unpack1(b,2) ;	   // first get the code select header (2 bits)
			if(d == 0)	{					   // if it is a bfp or pass thru block
				nb = (int)(unpack1(b,4)+1) ;	 // get the 4-bit bfp exponent
			}
		}
	}
	// now we have the code type, number of bits and block length

	if(d>0)           	          // the block is rice encoded
		err = unpackr(op,b,n,d-1,nch) ;

	else {										// the block is bfp encoded with word length of nb
		err = unpackn(op,b,n,nb,nch) ;
		if(nb==16)                // no filtering needed if pass through coded
			return(n) ;          // no sign fix needed either IF short is 16 bits
		// If there are portability concerns, do the fixsign call
		fixsign(op,n,nb,nch) ;
	}

	integrate(op,last,n,nch) ;       // run deemphasis filter
	return(n) ;

}

int   sdiffmaxs(short *op, short *ip, int n, int nch)
{
	// De-emphasis filter to reverse the diff in the compressor.
	// Filters operates in-place.
	int   c, k, ma=0 ;

	for(k=0; k<n; k++, ip+=nch) {
		c = *ip - *(ip-nch) ;
		*op++ = (short) c ;
		ma = MAX(ma,abs(c)) ;
	}

	return(ma) ;
}


short   integrate(short *op, short last, int n, int nch)
{
	// De-emphasis filter to reverse the diff in the compressor.
	// Filters operates in-place.
	int    k ;

	for(k=0; k<n; k++, op+=nch) {
		last += *op ;
		*op = last ;
	}

	return(last) ;
}
