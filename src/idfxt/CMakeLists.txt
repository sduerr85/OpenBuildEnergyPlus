
set(SRC 
  JSONDataInterface.cpp
  JSONDataInterface.h
  idd-full.h
)
add_definitions(-std=c++11)

if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()

add_library(JSONDataAPI ${SRC})
add_executable(idfxt main.cpp)
target_link_libraries(idfxt JSONDataAPI cJSON)

install( TARGETS idfxt DESTINATION ./ )

