/* -*- mode: C++; indent-tabs-mode: nil; -*- */
/** \file
 * \brief Code to read variable config file in JSON format.
 *
 * This replaces the XML config file in the original EnergyPlus/BCVTB interface.
 * It helps remove the dependency on Java for ExternalInterface.
 *
 * This file is part of the openBuildNet simulation framework
 * (OBN-Sim) developed at EPFL.
 *
 * \author Truong X. Nghiem (xuan.nghiem@epfl.ch)
 */

#include "configfile.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/reader.h"
#include <cstdio>
#include <string>

using namespace rapidjson;
using namespace std;

////////////////////////////////////////////////////////////////
///  This method frees the local memory allocated
///
///\param strArr 1D string array to be freed
///\param n the size of the 1D string array
////////////////////////////////////////////////////////////////
void
freeResource(char ** strArr, int n)
{
    int i;
    for(i=0; i<n; i++)
        free(strArr[i]);
    free(strArr);
}

/** \brief Handler (callback) structure to process JSON input stream. */
struct ConfigFileHandler {
    // Member variables to store the results
    char * m_OutputVarsName;
    char * m_OutputVarsType;
    int * m_NumOutputVars;
    char const * m_InputKeys;
    int m_NumInputKeys;
    char * m_InputVars;
    int * m_NumInputVars;
    int * m_InputVarsType;
    int const * m_StrLen;
    
    char ** inputKeys;      ///< the string array to store the types of input variable types
    
    string m_currentName, m_currentObject, m_currentKey;      ///< Store the values of the current variable object
    
    enum {
        fieldNONE,
        fieldTYPE,
        fieldNAME,
        fieldKEY,
        fieldOBJECT
    } m_currentObjectField; ///< What is the current field we are reading from an object definition
    
    enum {
        cfgSTART,
        cfgARRAY,           // In the list of variables
        cfgOBJECT,          // A variable object, its type is not yet known
        cfgINPUT,           // In an input variable object
        cfgOUTPUT           // In an output variable object
    } m_state;              ///< current state in parsing the structure of the config file
    
    // Constructor to get the necessary pointers
    ConfigFileHandler(char *	const myOutputVarsName,
                      char *	const myOutputVarsType,
                      int *	const myNumOutputVars,
                      char const *	const myInputKeys,
                      int const *	const myNumInputKeys,
                      char *	const myInputVars,
                      int *	const myNumInputVars,
                      int *	const myInputVarsType,
                      int const *	const myStrLen):
    m_OutputVarsName(myOutputVarsName),
    m_OutputVarsType(myOutputVarsType),
    m_NumOutputVars(myNumOutputVars),
    m_InputKeys(myInputKeys),
    m_NumInputKeys(*myNumInputKeys),
    m_InputVars(myInputVars),
    m_NumInputVars(myNumInputVars),
    m_InputVarsType(myInputVarsType),
    m_StrLen(myStrLen)
    { }
    
    ~ConfigFileHandler() {
        freeResource(inputKeys, m_NumInputKeys);    // Free the allocated memory block
    }
    
    // Function to initialize the handler; must be called before using
    int init();
    
    bool Null() { return false; }
    bool Bool(bool b) { return false; }
    bool Int(int i) { return false; }
    bool Uint(unsigned i) { return false; }
    bool Int64(int64_t i) { return false; }
    bool Uint64(uint64_t i) { return false; }
    bool Double(double d) { return false; }
    bool RawNumber(const char* str, SizeType length, bool copy) { return false; }
    bool String(const char* str, SizeType length, bool copy);
    bool StartObject();
    bool Key(const char* str, SizeType length, bool copy);
    bool EndObject(SizeType memberCount);
    bool StartArray();
    bool EndArray(SizeType elementCount);
};

// This function initializes the handler object, mainly the input keys
// Copied from original code file (BCVTB)
int ConfigFileHandler::init() {
    int i=0, j=0, count=0;
    inputKeys = NULL;
    
    while (true) {
        if (m_InputKeys[count] == '\0') {
            if (inputKeys[i][j] != '\0')
                inputKeys[i][j] = '\0';
            break;
        }
        if (m_InputKeys[count] == ','){
            inputKeys[i][j]='\0';
            i++;
            j=0;
            count++;
        }
        else {
            if (j == 0) {
                char ** tmpInputKeys;
                tmpInputKeys = (char **) realloc(inputKeys, sizeof(char *) * (i+1) );
                if (tmpInputKeys == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed in 'configfile.c'\n");
                    return -1;
                }
                inputKeys = tmpInputKeys;
                inputKeys[i] = NULL;
            }
            
            inputKeys[i] = (char *)realloc(inputKeys[i], sizeof(char) * (j+2) );
            if(inputKeys[i] == NULL) {
                fprintf(stderr, "Error: Memory allocation failed in 'configfile.c'.\n");
                return -1;
            }
            inputKeys[i][j] = m_InputKeys[count];
            j++; count++;
        }
    }
    if ((i+1) != m_NumInputKeys ) {
        fprintf(stderr,
                "Error: Number of input variables keys found does not match:\nFound %d, expected %d\n",
                i+1, m_NumInputKeys);
        freeResource(inputKeys, i+1);
        return -1;
    }
    *m_NumOutputVars = 0;
    *m_NumInputVars = 0;
    m_OutputVarsName[0] = '\0';
    m_OutputVarsType[0] = '\0';
    m_InputVars[0] = '\0';
    m_state = cfgSTART;
    
    return 0;
}

// Process a string value in JSON
bool ConfigFileHandler::String(const char* str, SizeType length, bool copy) {
    // Must be in an object and in reading a field
    if ((m_state != cfgOBJECT && m_state != cfgINPUT && m_state != cfgOUTPUT) || m_currentObjectField == fieldNONE) {
        fprintf(stderr, "Error while parsing the variable configuration file in JSON: unexpected string value encountered.\n");
        return false;
    }
    
    switch (m_currentObjectField) {
        case fieldTYPE:
        {
            string t(str, length);
            if (0 == t.compare("in")) {
                m_state = cfgINPUT;
            } else if (0 == t.compare("out")) {
                m_state = cfgOUTPUT;
            } else {
                fprintf(stderr, "Error while parsing the variable configuration file in JSON: unknown variable type '%s'.\n",
                        t.c_str());
                return false;
            }
            break;
        }
            
        case fieldNAME:
            m_currentName.assign(str, length);
            break;
            
        case fieldKEY:
            m_currentKey.assign(str, length);
            break;
            
        case fieldOBJECT:
            m_currentObject.assign(str, length);
            break;
    }
    
    m_currentObjectField = fieldNONE;
    return true;
}

// Start a variable object
bool ConfigFileHandler::StartObject() {
    // A variable object must be within a list
    if (cfgARRAY != m_state) {
        return false;
    }
    
    m_state = cfgOBJECT;
    m_currentName.clear();
    m_currentObject.clear();
    m_currentKey.clear();
    m_currentObjectField = fieldNONE;
    
    return true;
}

// Start a key (field name)
bool ConfigFileHandler::Key(const char* str, SizeType length, bool copy) {
    string keyname(str, length);
    
    if (0 == keyname.compare("type")) {
        // Get a variable type, must currently in state cfgOBJECT (type not yet defined)
        if (cfgOBJECT != m_state) {
            fprintf(stderr, "Error while parsing the variable configuration file in JSON: unexpected 'type' field encountered.\n");
            return false;
        }
        
        m_currentObjectField = fieldTYPE;
    } else if (0 == keyname.compare("name")) {
        // Get a variable name
        m_currentObjectField = fieldNAME;
    } else if (0 == keyname.compare("key")) {
        m_currentObjectField = fieldKEY;
    } else if (0 == keyname.compare("object")) {
        m_currentObjectField = fieldOBJECT;
    } else {
        fprintf(stderr, "Error while parsing the variable configuration file in JSON: unrecognized field '%s'.\n",
                keyname.c_str());
        return false;
    }
    
    return true;
}

// End a variable object
bool ConfigFileHandler::EndObject(SizeType memberCount) {
    // An object can only end when its type is determined and its required fields are provided
    if (cfgINPUT == m_state) {
        // Input variable => name and key must be defined
        if (m_currentName.empty() || m_currentKey.empty()) {
            fprintf(stderr, "Error while parsing the variable configuration file in JSON: an input variable definition is missing a name or a key.\n");
            return false;
        } else {
            // Register the variable and check its type
            int j;
            for(j=0; j < m_NumInputKeys; j++) {
                if (0 == m_currentKey.compare(inputKeys[j])) {
                    if( (strlen(m_InputVars)+m_currentKey.length()+2) > *m_StrLen){
                        fprintf(stderr, "Error: Memory allocated for parsed E+ input\n"
                                "       variables name is not enough,\n"
                                "       allocated: %d.\n", *m_StrLen);
                        return false;
                    }
                    m_InputVarsType[*m_NumInputVars] = j+1;
                    strcat(m_InputVars, m_currentName.c_str());
                    strcat(m_InputVars, ";");
                    *m_NumInputVars += 1;
                    break;
                }
            }
            if (m_NumInputKeys == j) {
                fprintf(stderr, "Error: Unknown input variable type: %s.\n", m_currentKey.c_str());
                return false;
            }
        }
    } else if (cfgOUTPUT == m_state) {
        // Output variable => name and object must be defined
        if (m_currentName.empty() || m_currentObject.empty()) {
            fprintf(stderr, "Error while parsing the variable configuration file in JSON: an output variable definition is missing a name or an object.\n");
            return false;
        } else {
            // Copy object name
            if( (strlen(m_OutputVarsName)+m_currentObject.length()+2) > (*m_StrLen) ){
                fprintf(stderr, "Error: Not enough memory allocated for EnergyPlus output.\n"
                        "       Allocated: %d.\n", *m_StrLen);
                return false;
            }
            strcat(m_OutputVarsName, m_currentObject.c_str());
            strcat(m_OutputVarsName, (char *) ";");
            
            // Copy variable name
            if( (strlen(m_OutputVarsType)+m_currentName.length()+2) > *m_StrLen ){
                fprintf(stderr, "Error: Not enough memory allocated for EnergyPlus output.\n"
                        "       Allocated: %d.\n", *m_StrLen);
                return false;
            }
            strcat(m_OutputVarsType, m_currentName.c_str());
            strcat(m_OutputVarsType, (char *) ";");
            
            *m_NumOutputVars += 1;
        }
    } else {
        // Error
        fprintf(stderr, "Error while parsing the variable configuration file in JSON: a variable definition is incomplete / invalid.\n");
        return false;
    }
    
    m_state = cfgARRAY; // Go back to the list level
    return true;
}

// Start the list of variables
bool ConfigFileHandler::StartArray() {
    // A list must be at the root level of the config file, and only at the root level
    if (cfgSTART != m_state) {
        return false;
    }
    
    m_state = cfgARRAY;
    return true;
}

// End the list of variables
bool ConfigFileHandler::EndArray(SizeType elementCount) {
    // We allow multiple lists to be concatenated
    m_state = cfgSTART;
    return true;
}


////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int
getepvariables(
               char const *	const  fileName,
               char *	const myOutputVarsName,
               char *	const myOutputVarsType,
               int *	const myNumOutputVars,
               char const *	const myInputKeys,
               int const *	const myNumInputKeys,
               char *	const myInputVars,
               int *	const myNumInputVars,
               int *	const myInputVarsType,
               int const *	const myStrLen
               )
{
    FILE * fd;
    
    // Open the file
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    fd = fopen(fileName, "rb");
#else
    fd = fopen(fileName, "r");
#endif
    
    if (!fd) {
        fprintf(stderr, "Error: Could not open file '%s' when getting EnergyPlus variables.\n", fileName);
        return -1;
    }
    
    char readBuffer[65536]; // Allocate buffer to read the config file
    FileReadStream cfgfile(fd, readBuffer, sizeof(readBuffer)); // The input stream
    
    ConfigFileHandler handler(myOutputVarsName,
                              myOutputVarsType,
                              myNumOutputVars,
                              myInputKeys,
                              myNumInputKeys,
                              myInputVars,
                              myNumInputVars,
                              myInputVarsType,
                              myStrLen);  // The handler to process JSON input stream
    
    int retval = handler.init();
    if (0 != retval) {
        return retval;
    }
    
    Reader cfgreader;
    if (!cfgreader.Parse(cfgfile, handler)) {
        // Error while parsing the config file
        fprintf(stderr, "Error while parsing the variable configuration file in JSON '%s'.\n", fileName);
        fclose(fd);
        return -1;
    }
    
    // If we reach here, the JSON file has been parsed successfully, and variables have been loaded into the member variables of handler.
    
    fclose(fd);
    return 0;
}

////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence. The difference with getepvariables is that it does not
///  validate the configuration file
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int
getepvariablesFMU(
                  char const *	const fileName,
                  char *	const myOutputVarsName,
                  char *	const myOutputVarsType,
                  int *	const myNumOutputVars,
                  char const *	const myInputKeys,
                  int const *	const myNumInputKeys,
                  char *	const myInputVars,
                  int *	const myNumInputVars,
                  int *	const myInputVarsType,
                  int const *	const myStrLen
                  )
{
    return getepvariables(fileName, myOutputVarsName, myOutputVarsType, myNumOutputVars, myInputKeys, myNumInputKeys, myInputVars, myNumInputVars, myInputVarsType, myStrLen);
}
