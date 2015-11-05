# Include CMake code for OBN node.C++
SET(OBN_MAIN_DIR ~/work/working/obn)

# Include the common CMake code for node.C++
INCLUDE(${OBN_MAIN_DIR}/nodecpp/CMakeCommon.txt)

SET(SRC
defines.h
utilSocket.c
utilSocket.h
utilXml.c
utilXml.h
openbuildnet.h
openbuildnet.cpp
) 

IF( WIN32 )
  LIST( APPEND SRC bcvtb.def )
ENDIF()

INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/third_party/Expat/lib")

ADD_LIBRARY( bcvtb STATIC ${SRC} ${OBNNODE_CORE_SRCFILES})

IF( WIN32 )
  TARGET_LINK_LIBRARIES( bcvtb wsock32 )
ENDIF()

IF( MSVC )
  ADD_DEFINITIONS( "/w" )
ELSE()
  ADD_DEFINITIONS( "-w -fPIC" )
ENDIF()

set_target_properties(
  bcvtb
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path"
)

## Make sure that C++ 11 is used (for thread, mutex...)
# if(APPLE)
# list( APPEND CMAKE_CXX_FLAGS "-stdlib=libc++ -std=c++11 ${CMAKE_CXX_FLAGS}")
# else()
# set_property(TARGET bcvtb PROPERTY CXX_STANDARD 11)
# set_property(TARGET bcvtb PROPERTY CXX_STANDARD_REQUIRED ON)
# endif()
