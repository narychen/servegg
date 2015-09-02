cmake_minimum_required(VERSION 2.6)
PROJECT(libevent_test)
SET(_ ${PROJECT_SOURCE_DIR})


ADD_DEFINITIONS( -g -O0 -W -Wno-deprecated -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -fdiagnostics-color=always -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11 -DENCRYPT)

INCLUDE_DIRECTORIES(${_} ${_}/../../src/base ${_}/../../src/include ${_}/../../src/include/hiredis ${_}/../../src/include/event2)
LINK_DIRECTORIES(${_} ${_}/../../src/lib)

ADD_EXECUTABLE(evt example-libevent.cpp)

TARGET_LINK_LIBRARIES(evt base log4cxx event hiredis)
 
