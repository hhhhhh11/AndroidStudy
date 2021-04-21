cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

add_library( NDKTestApi
             SHARED
             src/main/jni/posNdk.c
        src/main/jni/jni_usePosNdk.c
 )

find_library(log-lib log)


target_link_libraries(
                        NDKTestApi
                       ${log-lib})