set (ci_source_files
   aaOCVConsumer.cpp
   aaCamCapture.cpp
   Thread.cpp
  )

#include_directories(${ARGUS_INCLUDE_DIR} ${EGL_INCLUDE_DIR} ${OPENGLES_INCLUDE_DIR} /home/ubuntu/Downloads/samples/utils ${OpenCV_INCLUDE_DIRS} /usr/local/include /usr/local/include/xlswriter/ /opt/intel/ipp/include )

#include_directories(./ ../algorithms )
add_compile_options(-std=c++14)
add_library(ci ${ci_source_files})

