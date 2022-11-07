// Sud C-API v1.0.
// Copyright (C) 2022 Jamie Macaulay
//
// This file is part of a C++ sud file library. Sud files are data files that
// contain both audio and auxiliary data. They compress audio data using an X3
// compression algorothm and
//
// The Sud C-API is free software: you can redistribute it
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
//  SudWrite.hpp
//  Write a sud file.

//  Created by Jamie Donald John Macaulay on 03/11/2022.
#include "SudWrite.h"
#include <iostream>
#include "crc16v3.h"



/**
 * @brief write a little endian integer.

 */
int writeInt(char* buf, int x){

    *buf  = (x>>16) & 0xFF;
    buf++;
    *buf  = (x>>24)  & 0xFF;
    buf++;
    *buf  = (x>>0) & 0xFF;
    buf++;
    *buf  = (x>>8) & 0xFF;
    buf++;
    
//    int i;
//    for (i=0; i<4; i++){
//        *buf = (x >> (8 * (i))) & 0xFF;
//        buf++;
//    }

   return 4;
}

/**
 *
 * @brief write a short
 */
int writeShort(char* buf, short x){
    
    int i;
    for (i=0; i<2; i++){
        *buf = (x >> 8 * (i)) & 0xFF;
        buf++;
    }

    return 2;
}

/**
 *
 * @brief write a byte.
 */
int writeByte(char* buf, char x){
    *buf = x;
    buf++;
    return 1;
}

/**
 * Write the XML X3 header to char array.
 * @brief - create the C++ header.
 */
int writeX3Xml(char* s, int sampleRate, int nChan, int blockSize){
        strcpy(s, "<X3ARCH PROG=\"x3new.m\" VERSION=\"2.0\">"); // DG remove \ from end
        char sfs[20] ;

        //assemble the metadata message
//        addxmlfield(s,"CFG","ID=\"0\" FTYPE=\"XML\"",NULL) ;
//        openxmlfield(s,"CFG","ID=\"1\" FTYPE=\"WAV\"") ;
//        snprintf(sfs, 20, "%d",sampleRate) ;
//        addxmlfield(s,"FS","UNIT=\"Hz\"",sfs) ;
//        snprintf(sfs,20, "%d",nChan) ;
//        addxmlfield(s,"NCHAN",NULL,sfs);
//        addxmlfield(s,"SUFFIX",NULL,"wav") ;
//        openxmlfield(s,"CODEC","TYPE=\"X3\" VERS=\"2\"") ;      // name of the encoder
//        snprintf(sfs, 20, "%d",b§lockSize);
//        addxmlfield(s,"BLKLEN",NULL,sfs) ;
//        addxmlfield(s,"CODES","N=\"4\"","RICE0,RICE1,RICE3,BFP") ;
//        addxmlfield(s,"FILTER",NULL,"DIFF") ;
//        addxmlfield(s,"NBITS",NULL,"16") ;
//        addxmlfield(s,"T","N=\"3\"","3,8,20") ;
//        closexmlfield(s,"CODEC") ;
//        closexmlfield(s,"CFG") ;
//        closexmlfield(s,"X3ARCH") ;
    
    if (strlen(s)%2 == 1){
        //need to make sure the length is even bnecause saud files don;t like odd character numbers...
        strcat(s," ") ;
    }
    


    return (int) strlen(s);
}


int writeX3Header(char* buf,  int samplerate, int nchan, int blockSize, time_t time){
    
    char s[500]; //must make sure this is large enough our we will cause a segmentation fault.
    int n = writeX3Xml(s,  samplerate,  nchan,  blockSize);
    
    std::cout << "XML " <<  n << " data: "  << s << std::endl;
    
    SudChunk chunk;
    
    chunk.chunkID = 0;
    chunk.unixtime = time;
    chunk.datalen = n;
    chunk.data = s;
    
    int nchunk = writeSudChunk(buf, &chunk, true);
    
    return nchunk;
}


int writeSudChunk(char* buf, SudChunk *chunk, bool swapendian){
        
    char* bufstart = buf;
    
    uint32_t unixtimeint = (uint32_t) chunk->unixtime;
    
    //the magic number
    buf+=writeShort(buf, MAGIC_NUMBER);
    
    //the chunk ID
    buf+=writeShort(buf, chunk->chunkID);
    
    //the data length
    buf+=writeShort(buf, chunk->datalen);
    
    //the sample count
    buf+=writeShort(buf, chunk->samplecount);
    
    //the unix time
    buf+=writeInt(buf, unixtimeint);
    
    //the time offset in micro seconds
    buf+=writeInt(buf, chunk->timeoffsetus);
    
    // the crc for the data.
    int crcdata = crc16((short*)  chunk->data,  chunk->datalen/2);
    
    buf+=writeShort(buf, crcdata); //todo
    
    //the crc for the header
    int crc = (int) crc16((short*) bufstart, (int) (buf - bufstart)/2);
    
    //the crc for the
    buf+=writeShort(buf, crc); //todo
    
    char* databuf = chunk->data;
    int i;
    
    if (!swapendian) {
        for (i=0; i<chunk->datalen; i++){
            buf+=writeByte(buf, *databuf);
            databuf ++;
        }
    }
    else {
        //swap endian
        for (i=0; i<chunk->datalen; i+=2){
            buf+=writeByte(buf, *(databuf+i+1));
            buf+=writeByte(buf, *(databuf+i));
        }
        if (chunk->datalen%2==1){
            //std::cout << "Last character "  << *(databuf+chunk->datalen-1) << std::endl;
            buf+=writeByte(buf, *(databuf+chunk->datalen-1));
        }
    }

    return CHUNK_HEADER_LEN +  chunk->datalen;
}


int writeSudHeader(char* buf, SudHeader *sudHeader){
    
    char* bufstart = buf;
    
    std::cout << "Current unix time 2: " <<  sudHeader->unixtime << std::endl;
    
    uint32_t unixtimeint = (uint32_t) sudHeader->unixtime;

    //Host code version//
    buf+=writeShort(buf, sudHeader->hostversion);
    
    //Host time
    buf+=writeInt(buf, unixtimeint);
    
    //Device Type
    buf+=writeByte(buf, sudHeader->devicetype);
    
    //Device Code Version
    buf+= writeByte(buf, sudHeader->deviceversion);
    
    //Device Time
    buf+=writeInt(buf, unixtimeint);

    //DeviceIdentifier
    buf+=writeInt(buf, sudHeader->deviceIdentifier);
    
    //Block Length
    buf+=writeInt(buf, sudHeader->blocklen); //not sure what this is
    
    //start block
    buf+=writeShort(buf, sudHeader->startblock); //not sure what this is

    //end block
    buf+=writeShort(buf, sudHeader->endblock); //not sure what this is

    //n mber of blocks
    buf+=writeInt(buf, sudHeader->nblocks); //not sure what this is
    
    //std::cout << "CRC: " << (bufstart-buf) << std::endl;

    //cyclic reduncacny check for the header
    int crc =  (int) crc16((short*) bufstart, (int) (buf - bufstart)/2);

    //crc
    buf+=writeShort(buf, crc);

    return SUD_HEADER_LEN;
    
};





