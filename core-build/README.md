
# miaoWM-core 编译指南

## 安装 Emscripten
```bash
# Get the emsdk repo
$ git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
$ cd emsdk

# Download and install the SDK tools.
$ ./emsdk install 1.39.4

# 确保一定使用这个版本 不然会编译不成功
$ ./emsdk activate 1.39.4

# Activate PATH and other environment variables in the current terminal
$ source ./emsdk_env.sh

# Check installation
$ emcc -v
```
## 编译 Freetype
```bash
# Get freetype package
$ wget https://nchc.dl.sourceforge.net/project/freetype/freetype2/2.13.3/freetype-2.13.3.tar.xz
$ tar -xJvf freetype-2.13.3.tar.xz

# Enter that directory
$ cd freetype-2.13.3

# Build
$ mkdir build && cd build
$ emcmake cmake .. -DCMAKE_CXX_FLAGS="-s ENVIRONMENT=web"
$ emmake make -j16
```

## 在安装OpenCV之前，需要处理一下

```bash
$ git clone --depth=1 -b 4.5.1 https://github.com/opencv/opencv.git opencv/sources
```

在构建OpenCV之前，必须自定义一些构建设置。
在编辑器中打开 `opencv/sources/platforms/js/build_js.py` 并做如下修改：

```plain
# In def get_cmake_cmd(self)

# 将这些设置打开，这是构建所必需的
-DWITH_JPEG=OFF               -> ON
-DWITH_PNG=OFF                -> ON
-DBUILD_opencv_imgcodecs=OFF  -> ON

# （可选）将这些设置设置为OFF以加速构建
-DWITH_QUIRC=ON               -> OFF
-DBUILD_ZLIB=ON               -> OFF
-DBUILD_opencv_calib3d=ON     -> OFF
-DBUILD_opencv_dnn=ON         -> OFF
-DBUILD_opencv_features2d=ON  -> OFF
-DBUILD_opencv_flann=ON       -> OFF
-DBUILD_opencv_photo=ON       -> OFF
-DBUILD_EXAMPLES=ON           -> OFF
-DBUILD_TESTS=ON              -> OFF
-DBUILD_PERF_TESTS=ON         -> OFF
```

## 构建 OpenCV

```bash
# change directory to opencv
$ cd opencv

$ emcmake python ./sources/platforms/js/build_js.py build_wasm --build_wasm
```

### 完成依赖构建

```bash
$ cd build_wasm
$ emmake make
```

### 验证依赖项工件

确保在你的OpenCV构建目录中存在以下文件：

`opencv/build_wasm/lib/libopencv_imgcodecs.a`  
`opencv/build_wasm/3rdparty/lib/liblibjpeg-turbo.a`  
`opencv/build_wasm/3rdparty/lib/liblibpng.a`

## 构建 miaoWM-core

### 修改 `CMakeLists.txt` 文件


打开 `CMakeLists.txt` 文件, 将 `OPENCV_DIR` 改为你自己的 `opencv` 目录.

```cmake
set(OPENCV_DIR "/path/to/opencv")
set(EMSDK_DIR "/path/to/emsdk")
```

### 生成构建文件

```bash
# change directory to penguin-recognizer
$ cd core-build

$ mkdir build && cd build
$ emcmake cmake ..

# Should print the following output if success:
# -- Configuring done
# -- Generating done
# -- Build files have been written to: ./watermask_wasm/build
```

### 完事了，构建~

```bash
$ emmake make -j16
# make: ['make']
# -- Configuring done
# -- Generating done
# -- Build files have been written to: ./watermask_wasm/build
# Consolidate compiler generated dependencies of target watermark
# [ 50%] Linking CXX executable watermark.js
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# no symbols
# [100%] Built target watermark
```
