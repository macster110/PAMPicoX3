#ifndef X3ENCODER_H
#define X3ENCODER_H

#include "x3cmp.h"
#include <stdint.h>

#ifndef X3DEFS
#define X3DEFS
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
#define MIN(a,b)    (((a)<(b)) ? (a) : (b))
#define MAX(a,b)    (((a)>(b)) ? (a) : (b))
#endif


XBuff x3compress(short* data, ushort nChan, ushort soundFrames);

#endif

