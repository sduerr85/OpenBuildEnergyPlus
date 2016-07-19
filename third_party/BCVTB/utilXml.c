// Methods for parsing XML files.

/*
********************************************************************
Copyright Notice
----------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy). All rights reserved.

If you have questions about your rights to use or distribute this
software, please contact Berkeley Lab's Technology Transfer Department
at TTD@lbl.gov

NOTICE.  This software was developed under partial funding from the U.S.
Department of Energy.  As such, the U.S. Government has been granted for
itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable, worldwide license in the Software to reproduce, prepare
derivative works, and perform publicly and display publicly.  Beginning
five (5) years after the date permission to assert copyright is obtained
from the U.S. Department of Energy, and subject to any subsequent five
(5) year renewals, the U.S. Government is granted for itself and others
acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
license in the Software to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so.


Modified BSD License agreement
------------------------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy).  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. Neither the name of the University of California, Lawrence
      Berkeley National Laboratory, U.S. Dept. of Energy nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes,
patches, or upgrades to the features, functionality or performance of
the source code ("Enhancements") to anyone; however, if you choose to
make your Enhancements available either publicly, or directly to
Lawrence Berkeley National Laboratory, without imposing a separate
written license agreement for such Enhancements, then you hereby grant
the following license: a non-exclusive, royalty-free perpetual license
to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

********************************************************************
*/

///////////////////////////////////////////////////////////
/// \file    utilXml.h
/// \brief   Methods for getting xml values
///          using the expat libray
///
/// \author  Rui Zhang
///          Carnegie Mellon University
///          ruiz@cmu.edu
/// \date    2009-08-11
///
/// \version $Id: utilXml.c 55724 2009-09-16 17:51:58Z mwetter $
///
/// This file provides methods to get general xml values \c getxmlvalue
/// using simple xpath expressions
/// values will be in the same order as they are in the xml file
///
/// This file also provides methods to get the EnergyPlus \c getepvariables.
/// The variables returned will be in the same order as they are in the
/// configuration file.
/// \sa getxmlvalue()
/// \sa getepvariables()
///
//////////////////////////////////////////////////////////

#include "utilXml.h"


#define BUFFSIZE        8192

char Buff[BUFFSIZE]; ///< Local buffer for reading in the xml file

////////////////////////////////////////////////////////////////
///\struct A simple stack structure to keep track of the parent elements
////////////////////////////////////////////////////////////////
typedef struct Stack2 {
    char ** head;
    int top;
    int cur;
} Stack2;


Stack2 expStk; ///< Variables for getxmlvalue function

char * att; ///< Local global variable for function \c getxmlvalue
char * vals;  ///< Local global variable for function \c getxmlvalue
int * numVals; ///< Local global variable for function \c getxmlvalue
int PARSEVALUE; ///< flag for parsing xml values 1 if parse, 0 if not parse

int const * strLen;     ///< the length of string parsed to this function

////////////////////////////////////////////////////////////////
/// Stack operation, this function will pop one element from stack
/// and will free the resource unused
////////////////////////////////////////////////////////////////
int
stackPopBCVTB()
{
    if(0==expStk.top)
        return -1;
    free((expStk.head)[expStk.top]);
    expStk.head = (char **) realloc(expStk.head, sizeof(char *) * (expStk.top));
    if(expStk.head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in 'utilXml.c'.\n");
        return -1;
    }
    expStk.top--;
    return expStk.top;
}

////////////////////////////////////////////////////////////////
/// Stack operation, will push one element into the stack
/// and will allocate memory for the new element, hence is deep copy
////////////////////////////////////////////////////////////////
int
stackPushBCVTB(char const * str)
{
    if(!str) return -1;
    expStk.top++;
    expStk.head = (char **) realloc(expStk.head, sizeof(char *) * (expStk.top+1));
    if(expStk.head == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in 'utilXml.c'");
        return -1;
    }
    expStk.head[expStk.top] = (char *)malloc(sizeof(char) * (strlen(str)+1) );
    if(expStk.head[expStk.top] == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in 'utilXml.c'");
        return -1;
    }
    strcpy(expStk.head[expStk.top], str);
    return expStk.top;
}

////////////////////////////////////////////////////////////////
/// Call back functions that will be used by the expat xml parser
//
/// This function is used for \c getxmlvalues
////////////////////////////////////////////////////////////////
static void XMLCALL
start(void * data, char const * el, char const ** attr)
{
  int i;
  if(0 == strcmp(el, expStk.head[expStk.cur]) && expStk.cur < expStk.top )
    expStk.cur++;
  if(expStk.cur == expStk.top){
    for(i=0; attr[i]; i += 2) {
      if( 0 == strcmp(attr[i], att) ){
        if(1 == PARSEVALUE){
          if( (strlen(vals)+strlen(attr[i+1])+2) > *strLen){
            fprintf(stderr, "Error: Memory allocated for parsed attribute\n"
                             "      values is not enough, allocated: %d.\n",
                             *strLen);
            *numVals = strlen(vals) + strlen(attr[i+1])+2;
            return;
          }
          if(vals[0] != '\0')
            strcat(vals, ";");
          strcat(vals, attr[i+1]);
        }
        *numVals = *numVals + 1;
      }
    }
  }
}

////////////////////////////////////////////////////////////////
/// Call back functions that will be used by the expat xml parser
//
/// This function is used for \c getxmlvalues
////////////////////////////////////////////////////////////////
static void XMLCALL
end(void * data, char const * el)
{
  if(!strcmp(el, expStk.head[expStk.cur])&& expStk.cur>0)
    expStk.cur--;
}


////////////////////////////////////////////////////////////////
/// This is a general function that returns the value according to \c exp
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the xml file name.
///\param exp the xPath expression.
///\param myVals string to store the found values, semicolon separated.
///\param mynumVals number of values found.
///\param myStrLen length of the string that is passed.
////////////////////////////////////////////////////////////////
int
getxmlvalues(
 char const * const fileName,
 char const * const exp,
 char * const myVals,
 int * const myNumVals,
 int const myStrLen
)
{
  char * temp;
  int i,j;
  FILE * fd;
  XML_Parser p;
  vals = myVals;
  numVals = myNumVals;
  *numVals = 0;
  strLen = &myStrLen;
  att = NULL;
  expStk.head = NULL;
  expStk.top = -1;
  expStk.cur = -1;
  fd = fopen(fileName, "r");
  if(!fd) {
    fprintf(stderr, "Error: Could not open file '%s'.\n", fileName);
    return -1;
  }
  p = XML_ParserCreate(NULL);
  if (!p) {
    fprintf(stderr, "Error: Could not allocate memory for parser in function 'getxmlvalue'.\n");
    fclose(fd);
    return -1;
  }
  i=2; j=0;
  if(!exp || '\0' == exp[0]) {
    fclose(fd);
    return -1; }
  if( exp[0] != '/' || exp[1] != '/') {
    fclose(fd);
    return -1; }

  temp = NULL;
  while(exp[i] != '\0'){
    if( exp[i] == '/' || exp[i] == '[' || exp[i] == ']') {
      if(0==j && 0==expStk.top) {
        fprintf(stderr, "Error when parsing expression in 'utilXml.c'.\n");
        return -1;
      }
      i++;
      if(strchr(temp, '@'))
        break;
      stackPushBCVTB(temp);
      free(temp);
      temp = NULL;
      j=0;
    }
    else {
      j++;
      char * thisTemp;
      thisTemp = (char *) realloc(temp, sizeof(char)*(j+1));
      if(thisTemp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in 'utilXml.c'.\n");
        return -1;
      }
      temp = thisTemp;
      temp[j-1]=exp[i];
      temp[j]='\0';
      i++;
    }
  }
  if(temp[0] == '@'){
    att = (char *) malloc(sizeof(char) * (strlen(temp) ) );
    if(att == NULL) {
      fprintf(stderr, "Error: Memory allocation failed in 'utilXml.c'.\n");
	  free(temp);
      return -1;
    }
    for(i=1; i<strlen(temp); i++)
      att[i-1] = temp[i];
    att[i-1]='\0';
    free(temp);
  }
  else {
    fprintf(stderr, "Error when parsing expression in 'utilXml.c'.\n");
	free(temp);
	free(att);
	while(i!= -1) stackPopBCVTB();
    return -1;
  }
  expStk.cur = 0;
  if(1 == PARSEVALUE)
    vals[0]='\0';
  *numVals = 0;
  XML_SetElementHandler(p, start, end);

  for (;;) {
    int done;
    int len;

    len = (int)fread(Buff, 1, BUFFSIZE, fd);
    if (ferror(fd)) {
      fprintf(stderr, "Error when reading xml variables in '%s'.\n", fileName);
      return -1;
    }
    done = feof(fd);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr, "Error: Parse error in file '%s':\n%s\n",
	      fileName,
              XML_ErrorString(XML_GetErrorCode(p)));
      return -1;
    }

    if (done)
      break;
  }
  if( 0 == *numVals ){
	  fprintf(stderr, "Error: Did not find xml value\n       for expression '%s'.\n       in file '%s'\n",
		  exp, fileName);
  }
  while( i != -1 )
    i = stackPopBCVTB();
  att = NULL;
  XML_ParserFree(p);
  fclose(fd);
  return 0;

}


////////////////////////////////////////////////////////////////
/// This method returns one xmlvalue for a given xPath expressions.
/// The function will call the function \c getxmlvalues to get the variables
/// without ";" at the end of the parsed string
///
/// Return values: 0 normal; -1 error
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the xml file name.
///\param exp the xPath expression.
///\param str string to store the found values, semicolon separated.
///\param nVals number of values found.
///\param strLen the string length allocated.
////////////////////////////////////////////////////////////////
int
getxmlvalue(
 char const * const fileName,
 char const * const exp,
 char * const str,
 int * const nVals,
 int const strLen
)
{
  int ret;
  PARSEVALUE = 1;
  ret = getxmlvalues(fileName,
	                   exp,
	                   str,
	                   nVals,
	                   strLen);

  if(ret != 0){
    fprintf(stderr,"Error: Error when attempting to parse file '%s'\n",fileName);
    return -1;
  }
  if(*nVals == 0){
    fprintf(stderr,"Error: No xml value parsed in file '%s'\n",fileName);
    return -1;
  }
  if(*nVals > 1){
    fprintf(stderr, "Error: More than one xml values parsed, \n"
                    "       while expecting one value. \n"
                    "       number of xml values parsed is: %d\n"
                    "       xPath: '%s'\n",
                    *nVals, exp);
    return -1;
  }
  return 0;
}
