

cmake_minimum_required(VERSION 3.1)

project(hcho)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

SET(OpenCV_DIR /Repos/rtums.com/rtcvs/00-神经网络/smartium/3rdparty/opencv/build/dist/lib/cmake/opencv4)

find_package(OpenCV REQUIRED)

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV Not Found!")
endif(NOT OpenCV_FOUND)

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories(${PROJECT_SOURCE_DIR})


set(hcho_SRCS
    main.cpp
    Color.cpp
    ColorUtil.cpp
    MathUtil.cpp
    HoughCircle.cpp
    FindSquares.cpp
)

add_executable(${PROJECT_NAME} ${hcho_SRCS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
