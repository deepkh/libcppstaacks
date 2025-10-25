#!/bin/bash
set -e

# Go to the script's directory
cd "$(dirname "$0")"

BUILD_DIR=build

# Clean previous build directory (optional)
if [ "$1" == "clean" ]; then
    echo "Cleaning build and Debug directories..."
    rm -rf "$BUILD_DIR" Debug
    exit 0
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake and build
echo "Configuring project..."
cmake -DCMAKE_BUILD_TYPE=Debug ..

echo "Building project..."
make -j"$(nproc)" VERBOSE=1

echo ""
echo "✅ Build complete!"
echo "Binaries and libraries are located in:"
echo "  $(realpath ../Debug)"
