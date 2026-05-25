#!/bin/bash
# Build ResumeBuilder for Android
# Requires: Android NDK r26+

echo "========================================"
echo "Building ResumeBuilder for Android"
echo "========================================"

cd "$(dirname "$0")/.."

# Check NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME not set"
    echo "Please set ANDROID_NDK_HOME to your NDK installation path"
    exit 1
fi

# Create build directory
mkdir -p build_android
cd build_android

# Configure with CMake using NDK toolchain
echo "Configuring CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build
echo "Building..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build successful!"
echo "Output: build_android/libresume_builder.so"
echo "========================================"
