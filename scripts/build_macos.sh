#!/bin/bash
# Build ResumeBuilder for macOS
# Requires: Xcode Command Line Tools

echo "========================================"
echo "Building ResumeBuilder for macOS"
echo "========================================"

cd "$(dirname "$0")/.."

# Create build directory
mkdir -p build_macos
cd build_macos

# Configure with CMake
echo "Configuring CMake..."
cmake .. -G Xcode \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

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
echo "Output: build_macos/Release/ResumeBuilder.app"
echo "========================================"
