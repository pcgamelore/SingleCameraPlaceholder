set (common_source_files
  aaDebug.cpp
  aaCircularBuffer.cpp
  )
add_compile_options(-std=c++14)
include_directories(./)
add_library(common ${common_source_files})
