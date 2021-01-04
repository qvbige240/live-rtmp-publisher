include_directories(./include)
aux_source_directory(. PROJECT_SOURCES)
set(PROJECT_LIBS x264 rtmp asound faac pthread)

include_directories(./util)
aux_source_directory(./util UTIL_SOURCES)
link_directories(${UTIL_LIB_DIR})
set(PROJECT_LIBS ${PROJECT_LIBS} ${UTIL_LIBS})
set(PROJECT_SOURCES ${PROJECT_SOURCES} ${UTIL_SOURCES})

# find opencv
find_package(OpenCV)
if(OPENCV_FOUND)
    include_directories(./include/cv)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIR})
    aux_source_directory(./cv CV_SOURCES)
    set(PROJECT_LIBS ${PROJECT_LIBS} ${OpenCV_LIBS})
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${CV_SOURCES})
endif()

# gen config.h
include_directories(${PROJECT_BINARY_DIR})
configure_file(
    ${PROJECT_SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/config.h
)

add_executable(publisher ${PROJECT_SOURCES})
target_link_libraries(publisher ${PROJECT_LIBS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -D_DEBUG -D_SAVE_H264 -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -D_DEBUG -D_SAVE_H264 -DH264_AVCC -Wall")
# Annex_B format file and to AVCC format stream, record_annexb_01.h264
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -D_DEBUG -DUSE_H264_READER -DSUBNALU_TO_AVCC_FORMAT -DH264_AVCC -Wall")
# Annex_B file stream, record_annexb_01.h264
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -D_DEBUG -DUSE_H264_READER -Wall")
# AVCC file stream, record_avcc.h264
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -D_DEBUG -DUSE_H264_READER -DH264_AVCC -Wall")
