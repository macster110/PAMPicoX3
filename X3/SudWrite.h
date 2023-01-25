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


#ifndef SudWrite_h
#define SudWrite_h

#include <stdio.h>
#include <stdint.h>
#include <stdint.h>
//#include "x3encoder.h"
#include <time.h>
#include "xml_if.h"
#include "x3cmp.h"


//A "magic" number - this is contianed in all chunk headers and would
//potnetially allow for the extraction of chunks from a corrupted sud file.
#define MAGIC_NUMBER  (0xA952)

//The length of the sud header in bytes.
#define SUD_HEADER_LEN  (30)

//The length of chunk header in bytes.
#define CHUNK_HEADER_LEN  (20)

//the default chunk ID for XML header chunks.
#define XML_CHUNK_ID (0)

//the X3 block size to use

#define SUD_BLOCK_SIZE  (16)
#ifndef X3_DEF_N
#define SUD_BLOCK_SIZE  (X3_DEF_N)
#endif

/**
 * The data contained in a single chunk.
 */
typedef struct {
    short chunkID;
    // the chunk ID
    int datalen;     // the length of data associated with the chunk in number of bytes
    char* data; //pointer to the data.
    int samplecount; //the number of SAMPLES in the data - 0 if not audio data.
    time_t unixtime; // the unix time with a 1 second resolution.
    int timeoffsetus; // the time offset in microseconds.
} SudChunk;


/**
 * The header data contained in a sud file.
 */
typedef struct {
    time_t unixtime; // the unix time at start of file with a 1 second resolution.
    short hostversion;
    time_t hostime; //the time the file was downloaded  e.g. from a SoundTrap
    uint8_t devicetype; //the type of device used. 0 for SoundTrap
    uint8_t deviceversion; // the code version of the device used - often 0.
    int deviceIdentifier; // the serial number of the device.
    int blocklen; //?
    short startblock; //?
    short endblock; //?
    int nblocks; //?
    
} SudHeader;


/**
 * @brief Writes a sud header to a buffer.
 * @param buf - the buffer to write to.
 * @param sudHeader - tthe sud header struct.
 */
int writeSudHeader(char* buf, SudHeader *sudHeader);

/**
 * @brief write a chunk header to a buffer.
 * @param buf - the buffer to write to.
 * @param chunk - struct containing the chunk header.
 * @param swapendian true to swap the endian of the data. There seems little logical reason for this other tha  this is the way sud files work...
 */
int writeSudChunk(char* buf, SudChunk *chunk, bool swapendian);

/**
 * @brief writes the X3 header to a sud file. Note that this must be written before any X3 data in the file
 * or sud decompression will not work.
 * @param buf - the buffer to write to.
 * @param chunkID - the chunk identifier.
 * @param sourceID - the ID of the parent chunk - usuallty this is just 1.
 * @param nchan - the number of channels.
 * @param nBits - the number of bits per sample.
 * @param blockSize - the block size. `
 */
int writeX3Header(char* buf, short chunkID, short sourceID, int nchan, int blockSize, int nBits, time_t time);

/**
 * @brief writes the X3 header to a sud file. Note that this must be written before any X3 data in the file
 * or sud decompression will not work.
 * @param buf - the buffer to write to.
 * @param chunkID - the chunk identifier.
 * @param sourceID - the ID of the parent chunk - usuallty this is the chunkID of the X3 handler.
 * @param sampleRate - the number of samples per second.
 * @param nchan - the number of channels.
 * @param nBits - the number of bits per sample.
 * @param bitshift - usually 0, but, if using say a 15 bit number the ADC then the wav file might be bitshifted left to optomise
 * @param suffix - the type of file to save to - usually this is "wav", however, could be "dwv" for saving click files or other bespoke endings.

 * X3 compression. In this case bitshift would be 1 so that the sud unpacker knows to bitshift each sample by one.
 */
int writeWavHeader(char* buf, short chunkID, short sourceID, int sampleRate, int nchan, int nBits, short bitshift, const char *suffix, time_t time);

/**
 * @brief Convenience function to write an X3 chunks.
 * @param buf - the buffer to write to.
 * @param chunkID - the chunk identifier i.e. which audio stream tow rite to (sud files can have multiple audio streams)
 * @param data pointer to UNCOMPRESSED data - the data will be compressed using an X3 algorithm. Note this assumes 16 bit data.
 * @param datalen - the length of the data .
 */
int x3Chunk(char* buf, short chunkID, short* data, int datalen, uint16_t nChan, time_t unixtime, uint32_t micros);

/**
 * @brief Convenience function to add CSV data to a sud file.
 * @param data ponter to chunk data.
 *
 */
int csvChunk(char* buf, short chunkID, char* data, int datalen, time_t unixtime, uint32_t micros);

/**
 * @brief Convenience function to add XML chunks to a sud file.
 */
int xmlChunk(char* buf, char* data, int dataLen, time_t unixtime, uint32_t micros);


#endif /* SudWrite_h */
