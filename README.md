# ORGNET
A wireless networking peripheral for the Psion Organiser II.

## Getting the code
This repository contains Git submodules. To clone this repository, run:

```bash
git clone --recurse-submodules https://github.com/James-Livesey/ORGNET
```

## Building and running
To build ORGNET's firmware and application, run:

```bash
./build.sh
```

This will also upload the build to the RP2350.

To test ORGNET's application in an emulator, run:

```bash
pushd app; ./build.sh --test; popd
```