# Base image: Ubuntu 24.04 with C++ toolchain pre-installed.
# Includes: gcc/g++, make, cmake 3.28, clangd, build-essential, git, pkg-config.
FROM mcr.microsoft.com/devcontainers/cpp:2-ubuntu24.04

# Install libsodium — the only dependency the base image is missing.
# libsodium-dev  → headers + static lib (needed at compile time)
# libsodium28    → shared lib (needed at runtime, Ubuntu 24.04 ships this version)
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        libsodium-dev \
    && rm -rf /var/lib/apt/lists/*
