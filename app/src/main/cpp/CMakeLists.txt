cmake_minimum_required(VERSION 3.10.2)


project("calculate_color_change")

set(opencvsourcedir /home/orbianta/Desktop/Texts/AI/OpenCV-android-sdk/sdk/native)

include_directories(${opencvsourcedir}/jni/include)
add_library(libopencv SHARED IMPORTED)
set_target_properties(libopencv PROPERTIES IMPORTED_LOCATION ${opencvsourcedir}/libs/${ANDROID_ABI}/libopencv_java4.so)


add_library(app
             SHARED
             color_change_calc.cpp)

find_library(log-lib
              log )

target_link_libraries(app
                        libopencv
                       ${log-lib})