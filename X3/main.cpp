//
//  main.cpp
//  X3
//
//  Created by Jamie Donald John Macaulay on 19/10/2022.
//

#include <iostream>

#include <iostream>
#include <fstream>
#include "x3encoder.h" //don't know why I can't user header file here.

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

int main()
{
    FILE *file;
    file = fopen("testWav.txt", "r");
    int NUM_SAMPLES = 2848;
        
    //create the a new array of samples.
    short wavdata[NUM_SAMPLES];
    
    int i=0;

    while (fscanf(file, "%hd", &wavdata[i]) != EOF)
    {
      i++;
    }
    fclose(file);

    wavdata[i] = '\0';

    std::cout << "Raw wav data" << std::endl;

    for (i = 0; wavdata[i] != '\0'; i++){
      printf("%d, ", wavdata[i]);
    }
    printf("\n");

    int chan = 1;
    //now we have an array of int16_t which we can compress
    int x = x3compress(&wavdata[0], chan, NUM_SAMPLES);

    std::cout << "Finished X3 test" << std::endl;
    
    std::cout << x << std::endl;

}



//int main(int argc, const char * argv[]) {
//    // insert code here...
//    std::cout << "Hello, World!\n";
//    return 0;
//}
