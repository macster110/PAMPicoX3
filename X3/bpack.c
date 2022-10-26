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
//	bpack.c  bit packing and unpacking functions
//	Last modified: March 2013

#include "bpack.h"

// right-hand masks for 0 to 16 bits
ushort  MASK[17] = {0,1,3,7,0x0f,0x1f,0x3f,0x7f,0x0ff,0x1ff,0x3ff,0x7ff,
               0x0fff,0x1fff,0x3fff,0x7fff,0xffff} ;

//ushort  MASK[65]  ={0, // extra zero at the start to deal with the case of 0 bits !
//        0x1, 0x3, 0x7, 0xF,   // should never be called with zero bits, but need this to pack out the rest of the codes.
//        0x1F, 0x3F, 0x7F, 0xFF,
//        0x1FF, 0x3FF, 0x7FF, 0xFFF,
//        0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
//        0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF,
//        0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
//        0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
//        0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF};

//// Rice code lookup tables. The number of bits and the codeword are interleaved
ushort  RTAB0[] = {12,1,10,1,8,1,6,1,4,1,2,1,1,1,3,1,5,1,7,1,9,1,11,1,13,1,15,1} ;
ushort  RTAB1[] = {12,3,11,3,10,3,9,3,8,3,7,3,6,3,5,3,4,3,3,3,2,3,
                  2,2,3,2,4,2,5,2,6,2,7,2,8,2,9,2,10,2,11,2,12,2} ;
ushort  RTAB3[] = {10,15,10,13,10,11,10,9,9,15,9,13,9,11,9,9,8,15,8,13,8,11,8,9,
                  7,15,7,13,7,11,7,9,6,15,6,13,6,11,6,9,5,15,5,13,5,11,5,9,
                  4,15,4,13,4,11,4,9,4,8,4,10,4,12,4,14,5,8,5,10,5,12,5,14,
                  6,8,6,10,6,12,6,14,7,8,7,10,7,12,7,14,8,8,8,10,8,12,8,14,
                  9,8,9,10,9,12,9,14,10,8,10,10,10,12,10,14} ;


// Rice code lookup tables. The number of bits and the codeword are interleaved - THESE ARE THE CODES USED IN
//X3cmpv2.c on the SoundTrap.
//ushort  RTAB0[] = {12,1,10,1,8,1,6,1,4,1,2,1,1,1,3,1,5,1,7,1,9,1,11,1,13,1} ;
//ushort  RTAB1[] = {12,3,11,3,10,3,9,3,8,3,7,3,6,3,5,3,4,3,3,3,2,3,
//                  2,2,3,2,4,2,5,2,6,2,7,2,8,2,9,2,10,2,11,2,12,2, 13,2} ;
//ushort  RTAB3[] = {10,13,10,11,10,9,9,15,9,13,9,11,9,9,8,15,8,13,8,11,8,9,
//                  7,15,7,13,7,11,7,9,6,15,6,13,6,11,6,9,5,15,5,13,5,11,5,9,
//                  4,15,4,13,4,11,4,9,4,8,4,10,4,12,4,14,5,8,5,10,5,12,5,14,
//                  6,8,6,10,6,12,6,14,7,8,7,10,7,12,7,14,8,8,8,10,8,12,8,14,
//                  9,8,9,10,9,12,9,14,10,8,10,10,10,12,10,14} ;

// pointers to the nbits entry corresponding to an integer 0 for each code
//ushort  *RTABS[] = {RTAB0+12,RTAB1+22,RTAB3+56} ; //Original
//ushort  *RTABS[] = {RTAB0+11,RTAB1+17,RTAB3+27} ; //SoundTraps
ushort  *RTABS[] = {RTAB0+12,RTAB1+22,RTAB3+56} ;

// number of suffix bits in each code
#define  RSUFF0   0
#define  RSUFF1   1
#define  RSUFF3   3

ushort   RSUFFS[] = {RSUFF0,RSUFF1,RSUFF3} ;

// inverse Rice table - same table for all Rice codes
//short   IRT[] = {0,-1,1,-2,2,-3,3,-4,4,-5,5,-6,6,-7,7,-8,8,-9,9,-10,10,
//					  -11,11,-12,12,-13,13,-14,14,-15,15,-16,16,-17,17,-18,18,
//                 -19,19,-20,20,-21,21,-22,22,-23,23,-24,24,-25,25,-26,26} ;

short IRT[] = {0,-1,1,-2,2,-3,3,-4,4,-5,5,-6,6,-7,7,-8,8,-9,9,-10,10,-11,11,-12,12,-13,13,-14,14,-15,15,-16,16,-17,17,-18,18,-19,19,-20,20};

// Un-packing functions:
// unpackn is a general nword x nbit unpacker
// unpackr() is a specialised unpacker for rice-coded blocks where each
// sample may have a different bit length

void  packinit(BPack *b, ushort *buff, int nwords)
{
 b->nw = 0 ;               // number of complete words produced
 b->nbuff = nwords ;       // buffer size in 16-bit words
 b->curr = 0 ;             // current word
 b->nb = 0 ;               // no valid bits in the current word
 b->buff = buff ;          // pointer to buffer of completed words
}


int   packflush(BPack *b)
{
 if(b->nb==0)              // if the current word is empty, the buffer is flushed
    return(b->nw) ;
    
 *b->buff = b->curr << (16-b->nb) ; // left justify the current word and append to the buffer
 return(b->nw+1) ;
}


int  pack1(BPack *b, short in, int nbits)
{
 int      bitcnt ;
 long     ow ;

 // pack 1 word of nbits to bit stream b. Mask off any sign bits. 
 ow = ((b->curr)<<nbits) | (in & MASK[nbits]) ;
 bitcnt = b->nb + nbits ;

 if(bitcnt>=16) {
    bitcnt -= 16 ;
    *(b->buff)++ = ow>>bitcnt ;
    ow &= MASK[bitcnt] ;
	 ++(b->nw) ;
	 }

 b->curr = ow ;
 b->nb = bitcnt ;
 return(0) ;
}


int  packn(BPack *b, short *in, int n, int nbits, int stride)
{
 int      k, nw, bitcnt ;
 ushort   *buff, msk ;
 long     ow ;

 // pack n words of nbits each to bit stream b. Mask off any sign bits beyond
 // nb bits. The input words are at *in, *(in+stride), *(in+2*stride), ... 

 buff = b->buff ;
 ow = b->curr ;
 bitcnt = b->nb ;
 nw = b->nw ;
 msk = MASK[nbits] ;
 if(((b->nbuff - nw+1)<<4)-bitcnt < nbits*n)     // check there are enough bits in the stream
    return(1) ;

 for(k=0; k<n; k++, in+=stride) {
//	 printf("Pack %d in %d bits, ow 0x%x bitcnt %d\n", *in, nbits, ow, bitcnt);
	 ow = (ow<<nbits) | (*in & msk) ;
	 bitcnt += nbits ;

	 if(bitcnt>=16) {
//		 printf("Wind on buff array\n");
		 bitcnt -= 16 ;
		 *buff++ = ow>>bitcnt ;
		 ow &= MASK[bitcnt] ;
		 ++nw ;
	 }
 }
// if (n == 20) exit(0);

 b->curr = ow ;
 b->nb = bitcnt ;
 b->nw = nw ;
 b->buff = buff ;
 return(0) ;
}


int  packr(BPack *b, short *in, int n, int code, int stride)
{
 // packer for variable length Rice codes
 int      k, bitcnt, nw, nb ;
 long     ow ;
 ushort   *buff, *codeoffs, *cp ;

 buff = b->buff ;
 bitcnt = b->nb ;
 nw = b->nw ;
 ow = b->curr ;
 codeoffs = RTABS[code] ;

 for(k=0; k<n; k++, in+=stride) {      // Do for n words
    cp = codeoffs + (*in<<1) ;         // make the table lookup address
    nb = *cp ;                         // get the number of bits
    ow = (ow<<nb) | *(cp+1) ;    // shift the accumulator and append the codeword
    bitcnt += nb ;

    if(bitcnt >= 16) {           // if this completes a 16-bit word
    	bitcnt -= 16 ;
    	*buff++ = ow>>bitcnt ;          // add it to the buffer
    	ow &= MASK[bitcnt] ;           // and mask off the saved bit in the accumulator
    	++nw ;
    }
 }

 b->curr = ow ;
 b->nb = bitcnt ;
 b->nw = nw ;
 b->buff = buff ;
 return(nw <= b->nbuff) ;
}


void  unpackinit(BPack *b, ushort *buff, int nwords)
{
 b->nb = 16 ;                 // 16 bits are available in the current word
 b->nw = nwords-1 ;           // number of words remaining in the buffer
 b->curr = *buff ;            // current word
 b->buff = buff+1 ;           // pointer to the next word
}


short  unpack1(BPack *b, int nbits)
{
 int      ntogo ;
 long     ow ;

 // unpack 1 word of nbits from bit stream b.
 ow = b->curr ;
 ntogo = b->nb ;
 if(ntogo<nbits) {      // nbits is <=16 so only 1 extra word is required from the stream
	 ow = (ow<<16) | *(b->buff)++ ;
	 ntogo += 16 ;
	 --(b->nw) ;
	 }

 ntogo -= nbits ;
 b->curr = ow & MASK[ntogo] ;
 b->nb = ntogo ;
 return((short)(ow>>ntogo)) ;
}


int  unpackn(short *out, BPack *b, int n, int nbits, int stride)
{
 int      k, ntogo, nw ;
 ushort   *buff ;
 long     ow ;

 // unpack n words of nbits each from bit stream b. Store the result in
 // *out, *(out+stride), *(out+2*stride), ... 

 buff = b->buff ;
 ow = b->curr ;
 ntogo = b->nb ;
 nw = b->nw ;
 if((nw<<4)+ntogo < nbits*n)     // check there are enough bits in the stream
    return(1) ;

 for(k=0; k<n; k++, out+=stride) {
    if(ntogo<nbits) {      // nbits is <=16 so only 1 extra word is required from the stream
		 ow = (ow<<16) | (*buff++) ;
		 ntogo += 16 ;
		 --nw ;
		 }

    ntogo -= nbits ;
	 *out = ow>>ntogo ;
    ow = ow & MASK[ntogo] ;
	 }

 b->curr = ow ;
 b->nb = ntogo ;
 b->nw = nw ;
 b->buff = buff ;
 return(0) ;
}


int  unpackr(short *out, BPack *b, int n, int code, int stride)
{
 // unpacker for variable length Rice codes
 int      k, ntogo, nw, lev, nsuffix, suff, ns ;
 long     ow, msk ;
 ushort   *buff ;

 buff = b->buff ;
 ntogo = b->nb ;
 nw = b->nw ;
 ow = b->curr ;
 nsuffix = RSUFFS[code] ;
 lev = 1<<nsuffix ;

 for(k=0; k<n; k++, out+=stride) {      // Do for n words
	 // First find the end of the variable length section.
    // If there is an end and a complete suffix in the current word, it will
    // have a value of at least 1<<nsuffix. If not, append the next word from
    // the stream
    if(ow < lev) {
		 ow = (ow<<16) | *buff++ ;
		 nw-- ;
		 ntogo += 16 ;
		 }

    // ow is now guaranteed to have a start and a suffix.
    // Find the start (i.e., the first 1 bit from the left)
	 for(ns=1, msk = 1<<(ntogo-1); (ow & msk)==0; ns++, msk>>=1);
    ntogo -= ns+nsuffix ;
	 suff = (ow >> ntogo) & (lev-1) ;
    ow &= (1<<ntogo)-1 ;
	 *out = IRT[lev*(ns-1)+suff] ;
	 }

 b->curr = ow ;
 b->nb = ntogo ;
 b->nw = nw ;
 b->buff = buff ;
 return(nw<-1) ;
}


void  fixsign(short *buff, int n, int nbits, int stride)
{
 int   k ;
 short w, offs, half ;

 half = 1<<(nbits-1) ;
 offs = half<<1 ;

 /* sign correct in-place n words, skipping nch words between each word */
 for(k=0; k<n; k++, buff+=stride) {
	 w = *buff ;
	 *buff = (w>=half) ? w-offs : w ;
	 }
}
