//
//  main.cpp
//  X3
//
//  Created by Jamie Donald John Macaulay on 19/10/2022.
//

#include <iostream>
#include <fstream>
#include "x3encoder.h"
#include "x3frame.h"
#include "SudWrite.h"


/**
 * @brief Test X3 encoding and decoding.
 *
 *  Imports some wav data (the first chunk of a SoundTrap file) encodes into X3 and decodes back into a raw wav file.
 *  The code then checks all the samples between the original and new file are the same.
 *  <p>
 *  Note the purpose of this was to test Mark's C library against the sud files produced by the SoundTrap. The main
 *  sticking point was that the default blklen for a SoundTrap is 16 whilst the blocklen define in x3cmp.h X3_DEF_N i=20
 *
 * @return  - nothing.
 */
#define MAXWAV (10000)

int main()
{
    
    std::cout << "Begin X3 test" << std::endl;

    //open a wav file of data.
    FILE *file;
    file = fopen("/Users/au671271/Library/Mobile Documents/com~apple~CloudDocs/Dev/X3/X3/testWav.txt", "r");
//    int NUM_SAMPLES = 2848;
    
    int printN = 50; //the maximum number of samples to print.

    //create the a new array of samples.
    short wavdata[MAXWAV];

    int i=0;

    while (fscanf(file, "%hd", &wavdata[i]) != EOF)
    {
      i++;
    }
    fclose(file);
    
    int nwav =i;

    wavdata[i] = '\0';

    std::cout << "Imported Wav bytes" << std::endl;
    
    char* wavBytesIn = (char*) wavdata;
    //print the original wav data
    int j;
    for (j = 0; j<MIN(nwav*2, printN); j++){
      printf("%d, ", *(wavBytesIn+j));
    }
    printf("\n");

//    for (i = 0; wavdata[i] != '\0'; i++){
//      printf("%d, ", wavdata[i]);
//    }
//    printf("\n");
    
//    char* wavBytes = (char*) &wavdata;
//
//    int j;
//    for (j = 0; j<i; j++){
//      printf("%d, ", *(wavBytes+j));
//    }
//    printf("\n");
    
//    short* swapBuff = (short*) wavdata;
//    for (i = 0; i < nwav; i++) {
//        swapBuff[i] = htons(swapBuff[i]);
//    }

    ushort nChan = 1; //number of channels
    
    //should be 1009 samples out
    //now we have an array of int16_t which we can compress
    XBuff x3buff = x3compress(wavdata, nChan, nwav);
    
    //std::cout << "Number of outputs: " << x.nsamps << std::endl;
    
//    for (i = 0; i<x.nsamps; i++){
//      printf("%d, ", x.data[i]);
//    }
//    printf("\n");

    std::cout << "X3 compressed bytes: " << std::endl;
    char* x3Bytes = (char*) x3buff.data;
    
    for (j = 0; j<MIN(x3buff.nsamps*2, printN); j++){
      printf("%d, ", *(x3Bytes+j));
    }
    printf("\n");
    
    
    short wavdataout[nwav];
    
    XBuff   obuff = {wavdataout, (ushort) nwav, nChan} ;

    X3_uncompress_def(&obuff,&x3buff);
    
    std::cout << "Wav bytes uncompressed: " << std::endl;

//
    char* wavBytesOut = (char*) obuff.data;
    for (j = 0; j<MIN(obuff.nsamps*2, printN); j++){
      printf("%d, ", *(wavBytesOut+j));
    }
    printf("\n");
    
    //are bytes between the original and wav->x3->wav the same?
    bool equalbytes = true;
    for (j = 0; j<nwav*2; j++){
        if (*(wavBytesOut+j) != *(wavBytesIn+j)){
            equalbytes = false;
            printf("Bytes not equal at %d original %d decompressed %d ", j, *(wavBytesOut+j),  *(wavBytesIn+j));
            printf("\n");
        }
    }
    
    std::cout << "Are all bytes equal after compression? " << equalbytes << std::endl;
    
    std::cout << "Finish X3 test" << std::endl;
    //12,14,-86,106,49,54,106,85,106,44,-75,-88,52,86,53,106,69,99- first bytes for X3 from Jav
    
    //Begin the sud test
    
    std::cout << "Begin .sud test" << std::endl;
    
    std::ofstream wf("/Users/au671271/Library/Mobile Documents/com~apple~CloudDocs/Dev/X3/X3/test_sud.sud", std::ios::out | std::ios::binary);
    
    if(!wf) {
        std::cout << "Cannot open file!" << std::endl;
       return 1;
    }
    
    char buf[10000];
    int samplerate = 576000;
    int nchan = 1;
    int blockSize = 16;
    
    char *bufptr = &buf[0];
    
    //get the current system time.
    time_t unixtime;
    unixtime = time(NULL);
    
    SudHeader sudHeader;
    sudHeader.unixtime = unixtime;
    
    std::cout << "Current unix time: " << unixtime << std::endl;
    
    bufptr += writeSudHeader(bufptr, &sudHeader);
    
    
    //write the X3 header to the file
    bufptr += writeX3Header(bufptr,   samplerate,  nchan,  blockSize, unixtime);
    
    
    std::cout << "Write " << bufptr -&buf[0] << " bytes to file" << std::endl;

    wf.write(&buf[0], (bufptr -&buf[0])+1);
    
//    int CHUNK_DATA_LEN = 2000;
//    char chunkData[CHUNK_DATA_LEN];
//    
//    SudChunk sudChunk;
//    sudChunk.unixtime = seconds;
//    sudChunk.data = chunkData;
//    sudChunk.datalen = CHUNK_DATA_LEN;
//    
//    bufptr += writeSudChunk(bufptr, &sudChunk);
//    
//    openxmlfield(chunkData,"1", "2");

    
    wf.close();
       if(!wf.good()) {
          std::cout << "Error occurred at writing time!" << std::endl;
          return 1;
    }
    
    std::cout << "Finish .sud test" << std::endl;
    
    


}
