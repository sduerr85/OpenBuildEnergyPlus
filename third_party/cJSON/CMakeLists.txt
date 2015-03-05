
set(SRC 
  cJSON.h
  cJSON.c
)

if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()


add_library( cJSON ${SRC} )

