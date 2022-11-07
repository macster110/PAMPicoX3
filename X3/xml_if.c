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
//	xml_if.c  XML input/output functions
//	Last modified: March 2013

#include <stdio.h>
#include <string.h>
//#include "xml_if.h"

#define  XML_START   "<X3>"
#define  XML_END     "</X3>"

#define METANAME "x3metafile"
#define XMLSTARTMESS "X"
//FILE     *xml_open(fname,s)
//{
// char    s1[100] = XML_START ;
//
// // add an encapsulating field to the metadata message to make it correct
// strcat(s1,s) ;
//
// // parse the xml message in s and open output files for each defined configuration
// FILE* xid = fopen(METANAME,"wt") ;
// fprintf(xid,XMLSTARTMESS) ;
// fwrite(xid,s,sizeof(char),0) ;
// strcat(s1,XML_END) ;
// return(xid) ;
//}


void     xml_close(FILE *xid)
{
 fprintf(xid,XML_END) ;
 fclose(xid) ;
}


int   openxmlfield(char *s, const char *field, const char *arg)
{
 strcat(s,"<") ;
 strcat(s,field) ;
 if(arg!=NULL) {
    strcat(s," ") ;
    strcat(s,arg) ;
    }
 strcat(s,">") ;
 return(0) ;
}


int   addxmlfield(char *s, const char *field, const char *arg, const char *value)
{
 strcat(s,"<") ;
 strcat(s,field) ;
 if(arg!=NULL) {
    strcat(s," ") ;
    strcat(s,arg) ;
    }

 if(value!=NULL) {
    strcat(s,">") ;
    strcat(s,value) ;
    strcat(s,"</") ;
    strcat(s,field) ;
    strcat(s,">") ;
    }
 else {
	 /*
	  * Changed by DG from single line ending with /> to the four following lines
	  * or my xml readers don't seem to understand it.
	  */
	 //    strcat(s," />") ; // Marks
	 strcat(s,">") ; // DG four lines.
	 strcat(s,"</") ;
	 strcat(s,field) ;
	 strcat(s,">") ;
 }

 return(0) ;
}


int   closexmlfield(char *s, const char *field)
{
 sprintf(s,"%s</%s>",s,field) ;
 return(0) ;
}
