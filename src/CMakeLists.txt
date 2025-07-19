add_library(SqRequest SHARED
    main.hpp
    main.cpp
    includes.hpp
    request.hpp
    request.cpp
)

# Define _SQ64 on 64-bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(SqRequest PRIVATE _SQ64)
    message("DEFAULTING TO 64 BIT BINARIES")
endif()

# Name plugin without lib prefix
set_target_properties(SqRequest PROPERTIES PREFIX "")

# Determine plugin output name
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
    set(PLUGIN_NAME "request04rel64")
else()
    set(PLUGIN_NAME "request04rel32")
endif()

set_target_properties(SqRequest PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})

# Include headers
target_include_directories(SqRequest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/readerwriterqueue
        ${CURL_INCLUDE_DIRS}
)

# Platform-specific linking
if(UNIX)
    # Linux: only depend on libcurl (which itself pulls OpenSSL)
    target_link_libraries(SqRequest
        PRIVATE
            ${CURL_LIBRARIES}
            -ldl
            -lpthread
    )
elseif(WIN32)
    # Windows: statically link everything
    target_link_libraries(SqRequest
        PRIVATE
            ${CURL_LIBRARIES}
            ${OPENSSL_LIBRARIES}
            -static
    )
    target_compile_definitions(SqRequest PRIVATE CURL_STATICLIB)
endif()
