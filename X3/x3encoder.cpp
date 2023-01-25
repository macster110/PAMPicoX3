#include "x3encoder.h"
#include "x3frame.h"
#include "crc16v3.h"
#include "x3cmp.h"
#include <iostream>


short  PICOBUFF[MAXFRAME+X3_HDRLEN] ;

int16_t htons2(int16_t hostshort) {
  return ((hostshort & 0xff) << 8) | (hostshort >> 8);
}


XBuff x3compress(short* data, ushort nChan, ushort soundFrames){
    
    
	XBuff ibuff[1];
	ibuff->data = data;
	ibuff->nch = nChan;
	ibuff->nsamps = soundFrames;

	int     ns, nw, k ;
    ns = ibuff->nsamps * ibuff->nch ;
    if(ns>MAXFRAME) {
        printf("input buffer too large for a single frame\n)") ;
        XBuff   null;
        return null;
    }

    short   *pb = PICOBUFF, cd;
	XBuff   pbuff = {PICOBUFF+X3_HDRLEN,MAXFRAME, nChan} ;
    
	nw = X3_compress_def(&pbuff,ibuff) ; // compresses a multi channel buffer, returns len of compressed data.

    cd = crc16(pbuff.data,nw) ; // get a crc code for the compressed buffer.
    // write the header into the X3_HDRLEN (10) bytes at the start of the buffer
    int id = 2;
    nw += x3frameheader(PICOBUFF,id,ibuff->nch,ibuff->nsamps,nw,0,cd);
    
//    // good to go and write to file.
//    // need to byte swap here - not 100% sure what Mark was up to in his code.
//    short* swapBuff = (short*) PICOBUFF;
//    for (int i = 0; i < nw; i++) {
//        swapBuff[i] = htons2(swapBuff[i]);
//    }
    
   // std::cout << "Number of samples: " << ibuff->nsamps << " nw " << nw << std::endl;
    pbuff.nsamps = nw;
    
    return pbuff;
}
