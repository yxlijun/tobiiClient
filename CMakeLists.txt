# face recognition
project(master)
cmake_minimum_required(VERSION 2.8)

# 配置OpenCV,设置环境变量`OpenCV_DIR`指向OpenCV的build文件夹路径
# `OpenCV_DIR` --> `/path/to/opencv/build`
# 保证`OpenCV_DIR/(x86|x64)/(vc10|vc11|vc12)/bin`路径在系统Path中
find_package(OpenCV REQUIRED)

if(MSVC)
    #set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    ADD_DEFINITIONS(-D_AFXDLL)
    add_definitions(-DUNICODE -D_UNICODE)
    SET(CMAKE_MFC_FLAG 2)
endif()

INCLUDE_DIRECTORIES("E:/code/tobiiProject/ZeroMQ 4.0.4/include")
LINK_DIRECTORIES("E:/code/tobiiProject/ZeroMQ 4.0.4/lib")
INCLUDE_DIRECTORIES("E:/code/tobiiProject/zguide/examples/C")
INCLUDE_DIRECTORIES("E:/code/tobiiProject/tobii-eyex-sdk/include/eyex")
LINK_DIRECTORIES("E:/code/tobiiProject/tobii-eyex-sdk/lib/x86")

# add c++11 flags for gcc
if (CMAKE_COMPILER_IS_GNUCXX)
    set(ENABLE_CXX11 "-std=c++11")
    if (GCC_VERSION VERSION_LESS 4.7)
        set(ENABLE_CXX11 "-std=c++0x")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ENABLE_CXX11}")
endif()

include_directories(.)
file(GLOB SOURCE *.h* *.c* *.rc)
#INCLUDE_DIRECTORIES(SYSTEM ${SimpleAmqpClient_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} libzmq-v120-mt-gd-4_0_4 )
