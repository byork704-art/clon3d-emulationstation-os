# Building clon3d-emulationstation-os on Raspberry Pi

This document describes how to compile and install the custom EmulationStation engine
for the Clon3D project on a Raspberry Pi.

## Prerequisites

### Raspberry Pi OS (Bullseye or later)

Install build dependencies:

```bash
sudo apt-get update
sudo apt-get install -y \
  cmake \
  build-essential \
  libsdl2-dev \
  libfreeimage-dev \
  libfreetype6-dev \
  libcurl4-openssl-dev \
  rapidjson-dev \
  libasound2-dev \
  libgles2-mesa-dev \
  libvlc-dev \
  libvlccore-dev \
  vlc-bin \
  fonts-droid-fallback \
  git
```

### Submodules

This repository uses git submodules. After cloning, initialize them:

```bash
git clone https://github.com/byork704-art/clon3d-emulationstation-os.git
cd clon3d-emulationstation-os
git checkout clon3d-engine-import
git submodule update --init --recursive
```

## Build Commands

### Raspberry Pi 4 / Pi 5 (Mesa VC4/V3D driver — Bullseye and later)

```bash
mkdir build && cd build
cmake .. -DUSE_MESA_GLES=On -DRPI=On
make -j$(nproc)
```

### Raspberry Pi 3 / earlier (legacy Broadcom driver)

```bash
# Install legacy driver support package first:
sudo apt-get install libraspberry-dev

mkdir build && cd build
cmake .. -DRPI=On
make -j$(nproc)
```

### Debug build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_MESA_GLES=On -DRPI=On
make -j$(nproc)
```

### OMX player support (Pi 3 and earlier, 32-bit OS only)

Add `-DOMX=On` to the cmake command if you need omxplayer video preview support:

```bash
cmake .. -DRPI=On -DOMX=On
make -j$(nproc)
```

> **Note:** omxplayer support is not available on 64-bit Raspberry Pi OS or on Bullseye+ by default.

## Installation

After building, install the binary and resources:

```bash
sudo make install
```

Or manually copy:

```bash
sudo cp emulationstation /usr/local/bin/
sudo cp -r resources /etc/emulationstation/
```

## Configuration

On first run, EmulationStation creates a default configuration at `~/.emulationstation/es_systems.cfg`.

Edit that file to point to your ROMs directories and emulator commands.

## Updating from Upstream

To pull in changes from the RetroPie upstream:

```bash
git remote add upstream https://github.com/RetroPie/EmulationStation
git fetch upstream
git merge upstream/master
```

Resolve any conflicts, then rebuild:

```bash
cd build && make -j$(nproc)
```
