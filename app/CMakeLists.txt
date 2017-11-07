# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

set(AppRoot D:/androidDemo/FFMPEG/app)
include_directories(${AppRoot}/src/main/jni/header/include)
include_directories(${AppRoot}/src/main/jni/header/libyuv)

add_library( # Sets the name of the library.
             wzf

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/jni/wzf.c
             src/main/jni/player.c
             src/main/jni/queue.c)

 add_library(
             avcodec
             SHARED
             IMPORTED
             )
 add_library(
               avdevice
               SHARED
               IMPORTED
               )
add_library(
               avfilter
               SHARED
               IMPORTED
               )
 add_library(
                 avformat
                 SHARED
                 IMPORTED
                 )
 add_library(
                   avutil
                   SHARED
                   IMPORTED
                   )
 add_library(
                   postproc
                   SHARED
                   IMPORTED
                   )
 add_library(
                   swresample
                   SHARED
                   IMPORTED
                   )
 add_library(
                    swscale
                    SHARED
                    IMPORTED
                    )
 add_library(
                     yuv
                     SHARED
                     IMPORTED
                     )

if(${ANDROID_ABI} STREQUAL "armeabi")
 set_target_properties(
           avcodec
           PROPERTIES IMPORTED_LOCATION
           ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavcodec-56.so
           )
 set_target_properties(
       avdevice
       PROPERTIES IMPORTED_LOCATION
       ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavdevice-56.so
       )
 set_target_properties(
       avfilter
       PROPERTIES IMPORTED_LOCATION
       ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavfilter-5.so
       )
 set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavformat-56.so
        )
 set_target_properties(
         avutil
         PROPERTIES IMPORTED_LOCATION
         ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavutil-54.so
         )
  set_target_properties(
         postproc
         PROPERTIES IMPORTED_LOCATION
         ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libpostproc-53.so
         )

  set_target_properties(
           swresample
           PROPERTIES IMPORTED_LOCATION
           ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libswresample-1.so
           )

 set_target_properties(
           swscale
           PROPERTIES IMPORTED_LOCATION
           ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libswscale-3.so
           )
 set_target_properties(
          yuv
          PROPERTIES IMPORTED_LOCATION
          ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libyuv.so
          )
 endif(${ANDROID_ABI} STREQUAL "armeabi")


 if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
  set_target_properties(
            avcodec
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavcodec-56.so
            )
  set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavdevice-56.so
        )
  set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavfilter-5.so
        )
  set_target_properties(
         avformat
         PROPERTIES IMPORTED_LOCATION
         ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavformat-56.so
         )
  set_target_properties(
          avutil
          PROPERTIES IMPORTED_LOCATION
          ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libavutil-54.so
          )
   set_target_properties(
          postproc
          PROPERTIES IMPORTED_LOCATION
          ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libpostproc-53.so
          )

   set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libswresample-1.so
            )

  set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libswscale-3.so
            )
  set_target_properties(
           yuv
           PROPERTIES IMPORTED_LOCATION
           ${CMAKE_SOURCE_DIR}/src/main/libs/armeabi/libyuv.so
           )
  endif(${ANDROID_ABI} STREQUAL "armeabi-v7a")
# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              android-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       wzf
                       avcodec
                       avdevice
                       avfilter
                       avformat
                       avutil
                       postproc
                       swresample
                       swscale
                       yuv

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib} )
