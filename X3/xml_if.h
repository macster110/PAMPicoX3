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
//	x3.h  Definitions and prototypes for the X3 API
//	Last modified: March 2013

#ifndef XML_IF_H_
#define XML_IF_H_

#ifndef X3DEFS
#define X3DEFS
  typedef unsigned long	   ulong ;
  typedef unsigned short	ushort ;
  typedef unsigned char	   uchar ;
  #define MIN(a,b)    (((a)<(b)) ? (a) : (b))
  #define MAX(a,b)    (((a)>(b)) ? (a) : (b))
#endif

 #define  XMLSTARTMESS   "<?xml version=\"1.0\" encoding=\"US-ASCII\" ?>\n"

 // xml i/o functions
 FILE     *xml_open(char* fname, char* s);

 // xml message creation functions
  int   openxmlfield(char *s, const char *field, const char *arg) ;
 extern int   addxmlfield(char *s, const char *field, const char *arg, const char *value) ;
 extern int   closexmlfield(char *s, const char *field) ;

#endif
