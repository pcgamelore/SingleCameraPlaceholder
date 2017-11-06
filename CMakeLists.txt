cmake_minimum_required(VERSION 2.8)
project( hardwareOpt )


set(CMAKE_MODULE_PATH "/usr/lib/aarch64-linux-gnu/"  "/home/ubuntu/Downloads/argus/cmake" "/home/ubuntu/tegra_multimedia_api/argus/cmake"  "~/Downloads" "${CMAKE_MODULE_PATH}")

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

include_directories( ${ARGUS_INCLUDE_DIR} ${EGL_INCLUDE_DIR} ${OPENGLES_INCLUDE_DIR} ${GST_INCLUDE_DIRS}  ${X11_INCLUDE_DIR} /home/ubuntu/Downloads/argus/samples/utils ${OpenCV_INCLUDE_DIRS} /usr/include/ /usr/local/include  /home/ubuntu/tegra_multimedia_api/include/  ./ci /usr/local/cuda-8.0/targets/aarch64-linux/include/ /home/ubuntu/tegra_multimedia_api/argus/include /home/ubuntu/tegra_multimedia_api/argus/samples/utils /home/ubuntu/tegra_multimedia_api/include/ )

add_subdirectory(ci)
add_subdirectory(common)

add_compile_options(-std=c++14)
cuda_add_executable( hardwareOpt main.cpp /home/ubuntu/tegra_multimedia_api/samples/common/algorithm/cuda/NvCudaProc.cpp /home/ubuntu/tegra_multimedia_api/samples/common/algorithm/cuda/NvAnalysis.cu   /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvV4l2Element.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvV4l2ElementPlane.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvVideoConverter.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvApplicationProfiler.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvBuffer.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvEglRenderer.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvElement.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvElementProfiler.cpp  /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvLogging.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvUtils.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvVideoDecoder.cpp /home/ubuntu/tegra_multimedia_api/samples/common/classes/NvVideoEncoder.cpp)

target_link_libraries( hardwareOpt ci common ${GST_LIBRARIES}  ${OpenCV_LIBS}  ${ARGUS_LIBRARIES} ${OPENGLES_LIBRARIES} ${X11_LIBRARIES} ${EGL_LIBRARIES}  /usr/lib/aarch64-linux-gnu/tegra/libnvbuf_utils.so  cuda /usr/lib/aarch64-linux-gnu/libv4l2.so  /home/ubuntu/Downloads/argus/build/samples/utils/libargussampleutils.a ${GTK3_LIBRARIES} ${GDK3_LIBRARIES})
