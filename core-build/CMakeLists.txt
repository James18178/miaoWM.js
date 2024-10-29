cmake_minimum_required(VERSION 3.1)
project(watermark)

# Use C++ 17
set(CMAKE_CXX_STANDARD 17)

# Set Release as default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# change the path to opencv if necessary
# if(NOT OPENCV_DIR)
set(OPENCV_DIR "./opencv")
# endif(NOT OPENCV_DIR)

# change the path to freetype if necessary
# if(NOT FREETYPE_DIR)
set(FREETYPE_DIR "./freetype")
# endif(NOT FREETYPE_DIR)

# change the path to emsdk if necessary
# if(NOT EMSDK_DIR)
set(EMSDK_DIR "~/Documents/emsdk")
# endif(NOT EMSDK_DIR)

# Does not work
# find_package(OpenCV REQUIRED PATHS "${OPENCV_DIR}/build_wasm" NO_DEFAULT_PATH)

# Add Freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
target_link_libraries(watermark ${FREETYPE_LIBRARIES})

# Needed for opencv2/opencv.hpp
include_directories("${OPENCV_DIR}/sources/include")

# Needed by opencv.hpp for opencv2/opencv_modules.hpp
include_directories("${OPENCV_DIR}/build_wasm")

# Needed by opencv_modules.hpp for every module
file(GLOB opencv_include_modules "${OPENCV_DIR}/sources/modules/*/include")
include_directories(${opencv_include_modules})

# Needed by emscripten.hpp
include_directories("${EMSDK_DIR}/upstream/emscripten/system/include")

# Needed by sources
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# Needed by 3rdparty
file(GLOB 3rdparty_include_modules "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/*/include")
include_directories(${3rdparty_include_modules})

# Recognize executable
add_executable(watermark 
"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/CvxFont.cpp")

# Add Freetype
include_directories("${FREETYPE_DIR}/include")
file(GLOB freetype_libs "${FREETYPE_DIR}/build/*.a")
target_link_libraries(watermark ${freetype_libs})

# Link to opencv.js precompiled libraries
file(GLOB opencv_libs "${OPENCV_DIR}/build_wasm/lib/*.a")
target_link_libraries(watermark ${opencv_libs})

file(GLOB opencv_3rdlibs "${OPENCV_DIR}/build_wasm/3rdparty/lib/*.a")
target_link_libraries(watermark ${opencv_3rdlibs})

# Specify linker arguments
set_target_properties(watermark PROPERTIES LINK_FLAGS " --bind -O3 -s ALLOW_MEMORY_GROWTH=1 -s EXTRA_EXPORTED_RUNTIME_METHODS=['FS'] -s EXPORT_ES6=1 -s SINGLE_FILE=1  -s MODULARIZE=1 -s EXPORT_NAME=watermark_Module")
