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
    
    int i=0;
    int nwav;
    //create the a new array of samples.
    short wavdata[MAXWAV];

    int printN = 50; //the maximum number of samples to print.
    
    
    
    /**** Import data from a text file ****/
//    //open a wav file of data.
//    FILE *file;
//    file = fopen("/Users/au671271/Library/Mobile Documents/com~apple~CloudDocs/Dev/X3/X3/testWav.txt", "r");
////    int NUM_SAMPLES = 2848;
//
//    //import the wavdata into an array from text file
//    while (fscanf(file, "%hd", &wavdata[i]) != EOF)
//    {
//      i++;
//    }
//    nwav =i
//    fclose(file);
    
    /**** Create a saw tooth wave ****/
    int sawmod = 30;
    for (int i=0; i<MAXWAV; i++){
        wavdata[i] =  i%sawmod -  (sawmod/2);
    }
    
    nwav =MAXWAV;
    
    printf("NWAV %d, ", nwav);


    wavdata[i] = '\0';

    std::cout << "Imported "<< 2*nwav << " wav bytes" << std::endl;
    
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

    std::cout << "X3 compressed to " << x3buff.nsamps*2 << " bytes: " << std::endl;
    char* x3Bytes = (char*) x3buff.data;
    
    for (j = 0; j<MIN(x3buff.nsamps*2, printN); j++){
      printf("%d, ", *(x3Bytes+j));
    }
    printf("\n");
    
    
    short wavdataout[nwav];
    
    XBuff   obuff = {wavdataout, (ushort) nwav, nChan} ;

    X3_uncompress_def(&obuff,&x3buff);
    
    std::cout << "Decompressed "<< obuff.nsamps*2 << " wav bytes" << std::endl;

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
    std::cout << "----------------------------" << std::endl;
    std::cout << "" << std::endl;

    //12,14,-86,106,49,54,106,85,106,44,-75,-88,52,86,53,106,69,99- first bytes for X3 from Jav
    
    //Begin the sud test
    
    std::cout << "Begin .sud test" << std::endl;
    
    std::ofstream wf("/Users/au671271/Library/Mobile Documents/com~apple~CloudDocs/Dev/X3/X3/test_sud.sud", std::ios::out | std::ios::binary);
    
    if(!wf) {
        std::cout << "Cannot open file!" << std::endl;
       return 1;
    }
    
    char buf[1000000];
    int samplerate = 576000;
    int nchan = 1;
    int blockSize = 16;
    int nBits = 16;
    short bitShift = 0;
    
    char *bufptr = &buf[0];
    
    //get the current system time.
    time_t unixtime;
    unixtime = time(NULL);
    
    SudHeader sudHeader;
    sudHeader.unixtime = unixtime;
    
    std::cout << "Current unix time: " << unixtime << std::endl;
    
    bufptr += writeSudHeader(bufptr, &sudHeader);
    
    std::cout << "X3 XML Header" << std::endl;
    //write the X3 header to the file
    bufptr += writeX3Header(bufptr,  2, 1, nchan,  blockSize, nBits, unixtime);
    
    
    std::cout << "Wav XML Header" << std::endl;
    //next write the wav header to the file - need to know what to do with the X3 decompressed data.
    bufptr += writeWavHeader(bufptr, 3, 2, samplerate, nchan, nBits, bitShift, "wav", unixtime);

    //now write all the raw bits to a binary file i.e. the sud file.
    
    int nw;
    for (int i=0; i<10; i++){
        //write 10 wav chunks to a file.    Note that the XML chunk has to be wav
        nw = x3Chunk(bufptr, 3, wavdata, nwav, nchan, unixtime, 0);
        bufptr +=nw;
        std::cout << "nw " << nw << " nwav "<< nwav << std::endl;

    }

    wf.write(&buf[0], (bufptr -&buf[0])+1);
    
    std::cout << "Write " << bufptr -&buf[0] << " bytes to file" << std::endl;

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
