# minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
SET(CMAKE_CXX_STANDARD 14)

PROJECT(Nyx)

SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# headers and source files shall remain forever separate
INCLUDE_DIRECTORIES(inc)

LIST(
	APPEND
 	NYX_LIB_SOURCES
 	src/syntax/token.cpp
 	src/syntax/parser.cpp
	src/syntax/abstract.cpp
	src/syntax/concrete.cpp
	src/syntax/tokenizer.cpp
)

ADD_LIBRARY(nyx-static STATIC ${NYX_LIB_SOURCES})
SET_TARGET_PROPERTIES(nyx-static PROPERTIES OUTPUT_NAME nyx)

ADD_LIBRARY(nyx-shared STATIC ${NYX_LIB_SOURCES})
SET_TARGET_PROPERTIES(nyx-shared PROPERTIES OUTPUT_NAME nyx)

ADD_EXECUTABLE(nyx-bin src/nyx.cpp)
TARGET_LINK_LIBRARIES(nyx-bin nyx-static)
SET_TARGET_PROPERTIES(nyx-bin PROPERTIES OUTPUT_NAME nyx)

