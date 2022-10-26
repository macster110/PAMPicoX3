//
//  main.cpp
//  X3
//
//  Created by Jamie Donald John Macaulay on 19/10/2022.
//

#include <iostream>
#include <fstream>
#include "x3encoder.h" //don't know why I can't user header file here.
#include "x3frame.h" //don't know why I can't user header file here.

/**
 * @brief Encodes wav chunks into X3 data
 *
 * To build in VS code. In top menu go to Terminal-> Run Build Task. Select C/C++ g++ buold active file.
 *
 * To run in VS code navigate to directory in TERMINAL and then run the build file i.e. ./X3Main
 *
 * SudFileX3Test.java decompresses the first chunk of a sud file, prints out the data and saves the wav data in a text file. This
 * can be used to compare the X3 writing capablity here.
 *
 * @return int
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
    //12,14,-86,106,49,54,106,85,106,44,-75,-88,52,86,53,106,69,99- first bytes for X3 from Java

}



//int main(int argc, const char * argv[]) {
//    // insert code here...
//    std::cout << "Hello, World!\n";
//    return 0;
//}
