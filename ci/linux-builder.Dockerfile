# The glibc of this image is the oldest one our Linux builds run on, keep it old on purpose.
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf automake ca-certificates clang cmake file git libc++-14-dev libc++abi-14-dev \
    libgl1-mesa-dev libglu1-mesa-dev libtool libx11-dev libxcursor-dev \
    libxext-dev libxi-dev libxinerama-dev libxrandr-dev m4 make perl pkg-config python3
