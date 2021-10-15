cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -s" )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -s" )

add_library( # Sets the name of the library.
        NlDownload

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
#        src/main/jni/jni_usePosNdk.c

        src/main/jni/arddownload.cpp
        src/main/jni/arq.cpp
        src/main/jni/codetools.cpp
        src/main/jni/des3des.cpp
        src/main/jni/downloader.cpp
        src/main/jni/function.cpp
        src/main/jni/inirw.cpp
        src/main/jni/lzss.cpp
        src/main/jni/nlcdownload.cpp
        src/main/jni/nld.cpp
        src/main/jni/nlddownload.cpp
        src/main/jni/nn.cpp
        src/main/jni/rsa.cpp
        src/main/jni/sha1.cpp
        src/main/jni/usbserial.cpp
        src/main/jni/nlpdownload.cpp
        src/main/jni/native-lib.cpp
        src/main/jni/ndk/jni_usePosNdk.c
        )


find_library(log-lib log)
find_library(z-lib z)

target_link_libraries( NlDownload
        ${log-lib}
        ${z-lib})