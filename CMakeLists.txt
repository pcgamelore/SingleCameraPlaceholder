cmake_minimum_required(VERSION 2.8)
project( hardwareOpt )

IF (NOT DEFINED PLATFORM)
    SET(PLATFORM TX2)
ENDIF()
ADD_DEFINITIONS(-DPLATFORM=${PLATFORM})

if( ${PLATFORM} MATCHES "TX1")
set(TEGRA_MULTIMEDIA_API_DIR /home/ubuntu)
set (CMAKE_CXX_FLAGS "-DENABLETX1=1")
ENDIF()

if( ${PLATFORM} MATCHES "TX2")
set(TEGRA_MULTIMEDIA_API_DIR /home/nvidia)
set (CMAKE_CXX_FLAGS "-DENABLETX2=1")
ENDIF()

#set(TEGRA_MULTIMEDIA_API_DIR /home/nvidia)


set(CMAKE_MODULE_PATH "/usr/lib/aarch64-linux-gnu/"  "/home/ubuntu/Downloads/argus/cmake" "/home/nvidia/tegra_multimedia_api/argus/cmake"  "~/Downloads" "${CMAKE_MODULE_PATH}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set(CMAKE_C_FLAGS "-std=c++11")

set(CMAKE_PREFIX_PATH /)
find_package(Argus      REQUIRED)
find_package(OpenGLES   REQUIRED)
find_package(X11        REQUIRED)
find_package(EGL        REQUIRED)
find_package(OpenCV     REQUIRED )
find_package(OpenVX     REQUIRED )

find_library( ARGUSSAMPLEUTILS libargussampleutils.a HINTS "/home/")

PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
PKG_CHECK_MODULES(GDK3 REQUIRED gdk-3.0)

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
                               gstreamer-sdp-1.0>=1.4
                               gstreamer-video-1.0>=1.4
                               gstreamer-app-1.0>=1.4)

include_directories( ${ARGUS_INCLUDE_DIR} ${EGL_INCLUDE_DIR} ${OPENGLES_INCLUDE_DIR} ${GST_INCLUDE_DIRS}  ${X11_INCLUDE_DIR} /home/ubuntu/Downloads/argus/samples/utils ${OpenCV_INCLUDE_DIRS} /usr/include/ /usr/local/include  ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/include/  ./ci /usr/local/cuda-8.0/targets/aarch64-linux/include/ ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/argus/include ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/argus/samples/utils ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/algorithm/cuda/ ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/include/ )

add_subdirectory(ci)
add_subdirectory(common)

add_compile_options(-std=c++14)
cuda_add_executable( hardwareOpt main.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/algorithm/cuda/NvCudaProc.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/algorithm/cuda/NvAnalysis.cu   ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvV4l2Element.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvV4l2ElementPlane.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvVideoConverter.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvApplicationProfiler.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvBuffer.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvEglRenderer.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvElement.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvElementProfiler.cpp  ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvLogging.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvUtils.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvVideoDecoder.cpp ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/samples/common/classes/NvVideoEncoder.cpp)

target_link_libraries( hardwareOpt ci common ${GST_LIBRARIES}  ${OpenCV_LIBS}  ${ARGUS_LIBRARIES} ${OPENGLES_LIBRARIES} ${X11_LIBRARIES} ${EGL_LIBRARIES}  /usr/lib/aarch64-linux-gnu/tegra/libnvbuf_utils.so  cuda /usr/lib/aarch64-linux-gnu/libv4l2.so  ${TEGRA_MULTIMEDIA_API_DIR}/tegra_multimedia_api/argus/build/samples/utils/libargussampleutils.a  ${GTK3_LIBRARIES} ${GDK3_LIBRARIES})
