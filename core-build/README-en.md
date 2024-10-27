
# miaoWM-core build guide

## install Emscripten
```bash
# Get the emsdk repo
$ git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
$ cd emsdk

# Download and install the SDK tools.
$ ./emsdk install 1.39.4

# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
$ ./emsdk activate 1.39.4

# Activate PATH and other environment variables in the current terminal
$ source ./emsdk_env.sh

# Check installation
$ emcc -v
```

## Before Building Opencv

```bash
$ git clone --depth=1 -b 4.5.1 https://github.com/opencv/opencv.git opencv/sources
```

Before building OpenCV, some build settings have to be customized.  
Open `opencv/sources/platforms/js/build_js.py` by an editor, and change the following settings.

```plain
# In def get_cmake_cmd(self)

# Turn these settings to ON, which are necessary for building
-DWITH_JPEG=OFF               -> ON
-DWITH_PNG=OFF                -> ON
-DBUILD_opencv_imgcodecs=OFF  -> ON

# (Optional) Turn these settings to OFF to accelerate building
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

## Build OpenCV

```bash
# change directory to opencv
$ cd opencv

$ emcmake python ./sources/platforms/js/build_js.py build_wasm --build_wasm
```

### Finalize Dependency Builds

```bash
$ cd build_wasm
$ emmake make
```

### Verify Dependency Artifacts

Now, the following files should exist

`opencv/build_wasm/lib/libopencv_imgcodecs.a`  
`opencv/build_wasm/3rdparty/lib/liblibjpeg-turbo.a`  
`opencv/build_wasm/3rdparty/lib/liblibpng.a`

## Build miaoWM-core

### Modify `CMakeLists.txt` if necessary

> If your `opencv` directory is at the same level as the `core-build` directory, like so:
>
> ```
> .
> ├── opencv
> └── core-build
> ```
>
> then you can skip this step as it is unnecessary.

Open `CMakeLists.txt`, modify the value of `OPENCV_DIR` to the path to your `opencv` directory.

```cmake
set(OPENCV_DIR "/path/to/opencv")
set(EMSDK_DIR "/path/to/emsdk")
```

### Make build files

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

### Finalize

```bash
$ emmake make
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