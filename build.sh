#!/bin/bash

# Copyright (c) 2025, 2026 Gary Huang (deepkh@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
