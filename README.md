# CANdle-SDK

**CANdle-SDK** is the official C++ and Python toolkit from [MAB Robotics](https://www.mabrobotics.pl/) for configuring, testing, and controlling:

- [**MD** motor controllers](https://www.mabrobotics.pl/product-page/md80-motor-controller) — compact brushless servo drives for legged and mobile robots
- [**CANdle** / **CANdle HAT**](https://www.mabrobotics.pl/motion-control/candle-series) — USB/SPI-to-CAN-FD dongles that connect up to 16 devices to a PC or single-board computer
- [**PDS** (Power Distribution System)](https://www.mabrobotics.pl/product-page/pds-power-distribution-system) — modular power management and monitoring for mobile robots

The SDK includes two ready-to-use apps for working with your hardware without writing any code:

- **candletool** — a command-line tool for configuring, calibrating, and diagnosing devices,
- **mdgui** — a graphical tool for tuning MD motion control gains.

It also provides C++ and Python libraries so you can build your own applications on top of the same drivers.

📖 Full documentation: [MD80 x CANdle Documentation](https://mabrobotics.github.io/MD80-x-CANdle-Documentation/CANdle-SDK/intro.html#candlesdk)

## Getting started

Pick the path that matches what you want to do:

| I want to... | Go to |
| --- | --- |
| Configure and test, no coding required | [Install candletool & mdgui](#install-candletool--mdgui) |
| Control MD/PDS from Python | [Python quick start](#python-quick-start) |
| Build C++ MD/PDS control apps using SDK | [Building from source](#building-from-source) |

## Install candletool & mdgui

Prebuilt installers for both apps are published on the [Releases page](https://github.com/mabrobotics/CANdle-SDK/releases/latest) — no build tools or source checkout required.

### Windows

Download the latest `candletool-<version>-Windows-<arch>.exe` and/or `mdgui-<version>-Windows-<arch>.exe` installer from [Releases](https://github.com/mabrobotics/CANdle-SDK/releases/latest) and run it.

#### Swap default USB driver (optional)
This is normally done by the installer automatically, but if for any reason the process fails, it can
be done manually.


Install the WinUSB driver for CANdle (one-time setup per PC):
   1. Download and run [Zadig](https://github.com/pbatard/libwdi/releases/download/v1.5.0/zadig-2.8.exe)
   2. Options -> List All Devices
   3. Select `MD USB-TO-CAN` from the dropdown menu
   4. Change the driver type to `libusb-win32`
   5. Click **Replace Driver** and wait for it to finish

### Linux

Download the `.deb` package matching your architecture (x86_64, arm64, or armhf) from [Releases](https://github.com/mabrobotics/CANdle-SDK/releases/latest), then install it:

```
sudo apt install ./candletool_<version>_<arch>.deb
sudo apt install ./mdgui_<version>_<arch>.deb
```

Once installed, run `candletool --help` to get started, or launch `mdgui` from your applications menu.

## Python quick start

> **Note:** The Python bindings wrap the C++ library and carry some overhead, so they don't match its performance. For high-frequency control loops or other performance-critical applications, use the [C++ library](#building-from-source) directly.
The Python bindings are published on PyPI as `candlesdk` — no compiler or source checkout needed. On some systems (e.g. recent Debian/Ubuntu) `pip` refuses to install system-wide, so installing into a virtual environment may be required:


`venv` setup (optional, if required):
```
python3 -m venv .venv
source .venv/bin/activate   # on Windows: .venv\Scripts\activate
```
Installing CandleSDK Python bindings:
```
pip install candlesdk
```

```python
import pyCandle as pc

candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)
md_id = pc.discoverMDs(candle)[0]
md = pc.MD(md_id, candle)

if md.init() == pc.MD_Error_t.OK:
    md.zero()
    md.setMotionMode(pc.MotionMode_t.IMPEDANCE)
    md.enable()
    ...
    ...
    ...
    md.disable()
```


### Running the Python examples

Ready-to-run scripts covering both MD and PDS control are in [examples/py](examples/py), e.g. `md_impedance.py` and `pds_example_basic.py`. Grab the folder (or the whole repo) with the [git clone](#get-the-source) instructions below, then, with `candlesdk` installed and your hardware connected, run:

```
python3 examples/py/md_impedance.py
```

## Building from source

This section is for developers building the C++ library, candletool or mdgui themselves — e.g. to modify the SDK or to target a platform without prebuilt packages. 
If you just want to configure hardware or write Python code, see [Install candletool & mdgui](#install-candletool--mdgui) or [Python quick start](#python-quick-start) instead.

### Get the source

```
git clone https://github.com/mabrobotics/CANdle-SDK.git
cd CANdle-SDK
git submodule update --init --recursive
```

 > **Note:** A zipped download of the repository will not work, since it relies on git submodules.

### Prerequisites

#### Linux

```
sudo apt install build-essential git cmake libusb-1.0-0-dev
```

#### Windows

Building requires w64devkit, which can be downloaded and configured automatically by the build script below.
You'll also need the WinUSB driver for CANdle — see the driver setup steps in [Install candletool & mdgui](#install-candletool--mdgui) above.

### Build

#### Linux

You can use the script:
```
./launch/buildForLinux.sh
```

Or build manually:
```
mkdir build
cd build
cmake ..
make -j6
```

Or, using Docker (x86_64 only; see [installing Docker on Ubuntu](https://docs.docker.com/engine/install/ubuntu/)):

```
./launch/runDockerForLinux86-64.sh
```

> **Note:** Additionally, you will be required to install udev rules to allow usage of CANdle via libUSB. To do so:
```
cd build
sudo make install_rules
```
This operation is required only once per PC, and may require restarting your PC to take effect.

#### Windows

Natively, from PowerShell:

```
./launch/buildForWindows.bat
```

Or cross-compile from Linux using Docker:

```
./launch/runDockerForWindows.sh
```

Both produce a `build/` directory containing the `candlelib` library and, by default, the `candletool`, `mdgui`, and example binaries.

### C++ examples

Example programs demonstrating MD and PDS usage — impedance control, diagnostics, PDS power stage/braking resistor handling, and more — are in [examples/cpp](examples/cpp). Each file builds into its own executable as part of the normal build above; run one directly from the build output, e.g.:

```
./build/examples/md_example_impedance
```

### Building the Python module from source

Most users should just `pip install candlesdk` — see [Python quick start](#python-quick-start). Build it yourself only if you need an unreleased change or a platform not covered by the published wheels.

Dependencies are listed inside `pyproject.toml`. To build a wheel for the current system, run `python -m build` inside the repository with your preferred Python binary.

To build wheels for multiple libc/Python version combinations at once, use:

```
./launch/pythonBuildWheel.sh
```

Then install the wheel matching your platform, e.g. `python -m pip install ./dist/candlesdk-1.5.0-cp310-cp310-linux_x86_64.whl` for CPython 3.10, glibc, x86-64.

### Use CANdle-SDK as a library in your own project

The recommended way to consume CANdle-SDK from another C++ project is as a git submodule:

```
git submodule add git@github.com:mabrobotics/CANdle-SDK.git
git submodule update --init --recursive
```

Then link against the `candle` target (built from `candlelib`) in your own `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.15)

project(myCandleProject)

add_subdirectory(CANdle-SDK) # added as a submodule
add_executable(myCandleProject main.cpp)
target_link_libraries(myCandleProject candle)
```

## Documentation

For more information check out our documentation page [here](https://mabrobotics.github.io/MD80-x-CANdle-Documentation/CANdle-SDK/intro.html#candlesdk).
