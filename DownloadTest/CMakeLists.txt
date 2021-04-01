cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

add_library( DownloadFromPc
             SHARED
             src/main/jni/jni_download_from_pc.c
             src/main/jni/sdtpNlupdate.c
             src/main/jni/posNdk.c
 )

find_library(log-lib log )


target_link_libraries( DownloadFromPc
                       ${log-lib})