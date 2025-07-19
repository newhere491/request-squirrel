cmake_minimum_required(VERSION 3.11)
project(sqrequest-squirrel)

option(FORCE_32BIT_BIN "Create a 32-bit executable binary if the compiler defaults to 64-bit." OFF)
option(FORCE_DEBUG_BIN "Create debug binary." OFF)

# Set build type
if(FORCE_DEBUG_BIN)
    set(CMAKE_BUILD_TYPE Debug)
    add_definitions(-D_DEBUG)
    message("Configuring Debug builds.")
else()
    set(CMAKE_BUILD_TYPE Release)
    message("Configuring Release builds.")
endif()

# Force 32-bit
if(FORCE_32BIT_BIN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
    message("FORCING 32 BIT BINARIES")
endif()

# Always use PIC for shared libs
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Required dependencies
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
message(STATUS "CURL include dir: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

add_subdirectory(src)
