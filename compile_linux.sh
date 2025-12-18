#!/bin/bash
set -e

# Update and install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build libcurl4-openssl-dev libssl-dev zlib1g-dev libsqlite3-dev rustc cargo

# Create build directory
mkdir -p build_linux_cross
cd build_linux_cross

# Configure
cmake -G Ninja -DCLIENT=OFF -DSERVER=ON -DCMAKE_BUILD_TYPE=Release ..

# Build
ninja DDNet-Server
