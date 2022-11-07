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
//	x3frame.c  Frame handling functions
//	Last modified: March 2013

#include <stdio.h>
#include <string.h>
#include "x3cmp.h"
#include "x3frame.h"

extern   unsigned int crc16(short *vec, int n) ;

#define  fread_short(fp,buff,n)   (fread((void *)(buff),sizeof(short),(n),(fp)))

int   x3frameheader(short *pb,int id, int nch, int ns, int nw, short *T, short cd) ;
int   fwrite_short(FILE *fp, ushort *buff, int n) ;

// Allocate a local buffer for handling frames. This should be larger than the
// largest allowable frame.
short    PBUFF[MAXFRAME+X3_HDRLEN] ;

/**
 * fid is an open output file, ibuff is a buffer of uncompressed data
 * id is 1, but I've no idea what it is - it's added to the frame header
 * of the output. timecode always NULL in this implementation, but if it were'nt
 * it would be added into the frame header somehow.
 */
int		X3_writeaudioframe(FILE *fid, XBuff *ibuff, int id, ushort *timecode)
{
	// write a frame of compressed audio data to the open file
	short   *pb = PBUFF, cd ;
	XBuff   pbuff = {PBUFF+X3_HDRLEN,MAXFRAME,1} ;
	int     ns, nw, k ;

	ns = ibuff->nsamps * ibuff->nch ;
	if(ns>MAXFRAME) {
		printf("input buffer too large for a single frame\n)") ;
		return(1) ;
	}

	nw = X3_compress_def(&pbuff,ibuff) ; // compresses a multi channel buffer, returns len of compressed data.
	cd = crc16(pbuff.data,nw) ; // get a crc code for the compressed buffer.
	// write the header into the X3_HDRLEN (10) bytes at the start of the buffer
	nw += x3frameheader(PBUFF,id,ibuff->nch,ibuff->nsamps,nw,timecode,cd) ;
	// good to go and write to file.
	fwrite_short(fid,PBUFF,nw) ;

	/**
	 * So, if we wanted to create packets which could be written to file or to a network port, we'd
	 * need to wrap the compress, crc and frameheader functions. Could then happily send either to file
	 * or to a Network socket.
	 */
	return(0) ;
}


int		X3_writemetaframe(FILE *fid, char *s, ushort *timecode)
{
	// write a frame of text metadata to the open file
	short   cd, *pb = PBUFF+X3_HDRLEN ;
	int     nw, k ;

	nw = (strlen(s)+1)>>1 ;      // number of words needed for packed string
	if(nw>MAXFRAME) {
		printf("string too large for a single frame\n)") ;
		return(1) ;
	}

	// pack the message into words
	// note that this version of the code currently byte swaps here, then
	// in fwrite_short it byte swaps again ! May not need to do this after checking endienness
	// of different platofrms.
	for(k=0;k<nw;k++,s+=2)
		*pb++ = (*s<<8) | *(s+1) ;

	cd = crc16(PBUFF+X3_HDRLEN,nw) ;
	nw += x3frameheader(PBUFF,0,0,0,nw,timecode,cd) ;
	fwrite_short(fid,PBUFF,nw) ;
	return(0) ;
}


int		X3_readaudioframe(XBuff *obuff, FILE *fid, XHdr *hdr)
{
	// Read a frame of compressed audio data from the open file and decode it.
	// The header has already been read from the stream and is in the third argument.
	// Output buffer obuff must have room for hdr->NS * hdr->NCH 16 bit words.

	XBuff   pbuff = {PBUFF,0,1} ;
	int     nw = hdr->NBY>>1 ;

	if(nw > MAXFRAME) {
		printf("frame too large for buffer\n)") ;
		return(1) ;
	}

	fread_short(fid,PBUFF,nw) ;
	obuff->nsamps = hdr->NS ;
	obuff->nch = hdr->NCH ;
	pbuff.nsamps = nw ;
	X3_uncompress_def(obuff,&pbuff) ;
	return(crc16(PBUFF,nw) != hdr->CD) ;
}


int		X3_readmetaframe(char *s, FILE *fid, XHdr *hdr)
{
	// Read a frame of metadata from the open file and unpack it.
	// The header has already been read from the stream and is in the third argument
	// String s must have room for hdr->NW *2 + 1 characters.

	short   *pb = PBUFF ;
	int     k, nw ;

	nw = hdr->NBY>>1 ;
	if(nw>MAXFRAME) {
		printf("frame too large for buffer\n)") ;
		return(1) ;
	}

	fread_short(fid,PBUFF,nw) ;
	for(k=0;k<nw;k++,pb++) {
		*s++ = *pb >> 8 ;
		*s++ = *pb & 0x0ff ;
	}

	*s = 0 ;
	return(crc16(PBUFF,nw) != hdr->CD) ;
}


int    X3_getnextframehdr(XHdr *h, FILE *fid)
{
	short   H[10] ;

	fread_short(fid,H,sizeof(XHdr)>>1) ;
	h->KEY = H[0] ;
	h->ID = H[1]>>8 ;
	h->NCH = H[1]&0x0ff ;
	h->NS = H[2] ;
	h->NBY = H[3] ;
	h->T0 = ((ulong)H[4]<<16) | (ulong)H[5] ;
	h->T1 = ((ulong)H[6]<<16) | (ulong)H[7] ;
	h->CH = H[8] ;
	h->CD = H[9] ;
	if(h->KEY != X3_KEY)
		return(2) ;

	if(crc16(H,8) != h->CH)
		return(3) ;

	return(0) ;
}

/**
 * Write an x3 file header, complete with it's own smaller header (10 words)
 * and return the total number of bytes written.
 */
int X3_prepareXMLheader(char* s, int sampleRate, int nChan, int blockSize) {
	strcpy(s, "<X3ARCH PROG=\"x3new.m\" VERSION=\"2.0\">"); // DG remove \ from end
	char sfs[20] ;

	// assemble the metadata message
	addxmlfield(s,"CFG","ID=\"0\" FTYPE=\"XML\"",NULL) ;
	openxmlfield(s,"CFG","ID=\"1\" FTYPE=\"WAV\"") ;
	sprintf(sfs,"%d",sampleRate) ;
	addxmlfield(s,"FS","UNIT=\"Hz\"",sfs) ;
	sprintf(sfs,"%d",nChan) ;
	addxmlfield(s,"NCHAN",NULL,sfs);
	addxmlfield(s,"SUFFIX",NULL,"wav") ;
	openxmlfield(s,"CODEC","TYPE=\"X3\" VERS=\"2\"") ;      // name of the encoder
	sprintf(sfs,"%d",blockSize);
	addxmlfield(s,"BLKLEN",NULL,sfs) ;
	addxmlfield(s,"CODES","N=\"4\"","RICE0,RICE1,RICE3,BFP") ;
	addxmlfield(s,"FILTER",NULL,"DIFF") ;
	addxmlfield(s,"NBITS",NULL,"16") ;
	addxmlfield(s,"T","N=\"3\"","3,8,20") ;
	closexmlfield(s,"CODEC") ;
	closexmlfield(s,"CFG") ;
	closexmlfield(s,"X3ARCH") ;
    
	return strlen(s);
}

FILE  *X3_new(char *fname, int fs)
{
	FILE *fid ;
	char s[X3HEADLEN];
	fid = fopen(fname,"wb") ;
	fprintf(fid,X3_FILE_KEY) ; // "X3ARCHIV" could be used to compare / identify difference between Marks and my files.

	// // assemble the metadata message
	// addxmlfield(s,"CFG","ID=\"0\" FTYPE=\"XML\"",NULL) ;
	// openxmlfield(s,"CFG","ID=\"1\" FTYPE=\"WAV\"") ;
	// sprintf(sfs,"%d",fs) ;
	// addxmlfield(s,"FS","UNIT=\"Hz\"",sfs) ;
	// addxmlfield(s,"SUFFIX",NULL,"wav") ;
	// openxmlfield(s,"CODEC","TYPE=\"X3\" VERS=\"2\"") ;      // name of the encoder
	// addxmlfield(s,"BLKLEN",NULL,"20") ;
	// addxmlfield(s,"CODES","N=\"4\"","RICE0,RICE1,RICE3,BFP") ;
	// addxmlfield(s,"FILTER",NULL,"DIFF") ;
	// addxmlfield(s,"NBITS",NULL,"16") ;
	// addxmlfield(s,"T","N=\"3\"","3,8,20") ;
	// closexmlfield(s,"CODEC") ;
	// closexmlfield(s,"CFG") ;

	// create the meta frame in a separate function.
	int xmlLen = X3_prepareXMLheader(s, fs, 8, 20);
	// pack the metadata frame and write it to the file
	X3_writemetaframe(fid,s,NULL) ;
	return(fid) ;
}


FILE  *X3_open(char *fname)
{
	FILE *fid ;
	char s[500] = "=" ;

	fid = fopen(fname,"rb") ;
	if(fid<0) {
		printf(" Unable to open file %s\n", fname) ;
		return(NULL) ;
	}

	// check there is an X3 file header at the start of the file
	fread(fid,8,sizeof(char),s) ;
	s[8] = '\0' ;

	if(strcmp(s,X3_FILE_KEY)!=0) {
		fclose(fid) ;
		printf(" File is not an X3 archive\n") ;
		return(NULL) ;
	}

	return(fid) ;
}

/**
 * Write an x3 header into data,
 * return the number of word16's written (always 10 = 20 bytes).
 * These data will later be flipped with htons, so need to be
 * wary that this may swap the byte ordering - i.e. is this cross endian compatible ?
 */
int     x3frameheader(short *pb, int id, int nch, int ns, int nw, short *T, short cd)
{
	short   *pt = pb ;
	int     k ;

	*pb++ = X3_KEY ;
	*pb++ = (id<<8) | (nch&0x0ff) ; // possibly NOT cross endian compatible to do this !
	*pb++ = ns ;
	*pb++ = nw << 1 ;
	if(T != NULL)
		for(k=0;k<4;k++)
			*pb++ = T[k] ;
	else
		for(k=0;k<4;k++)
			*pb++ = 0 ;

	*pb++ = crc16(pt,8) ;
	*pb++ = cd ;
	return(X3_HDRLEN) ;
}


/**
 * write shorts to a file, byte swapping prior to
 * writing. Which platform this is needed for I'm not
 * sure ...
 */
int   fwrite_short(FILE *fp, ushort *buff, int n)
{
	int     k ;
	ushort  c, *bp = buff ;

	for(k=0;k<n;k++) {
		c = *buff ;
		*buff++ = (c>>8) | ((c&0x0ff)<<8) ;
	}

	fwrite((void *)(bp),sizeof(short),n,fp) ;
	return n;
}

