# wdrvr
wdrvr is a lightweight, open-source tool designed to visualize war-driving data using POIs exported from [WiGLE.net](https://wigle.net/). This first beta release introduces a focused utility for hobbyists and security researchers who want a clean, local way to view their collected data.

![wdrvr](https://github.com/DigitalArtifex/wdrvr/blob/main/images/wdrvr.png)

# Usage

Obtaining the latest release is typically done by visiting the [releases](https://github.com/DigitalArtifex/wdrvr/releases) page where we offer pre-built binaries for Windows 7+ and Linux (glibc 2.35+). Alternatively, if you already have an up-to-date version of Qt installed or would just like to use a nightly build, follow the instructions for building below.

# Building

wdrvr was built using Qt6 and can be compiled by opening the project in QtCreator or with CMake directly.

```
git clone git@github.com:DigitalArtifex/wdrvr.git
mkdir wdrvr/build
cd wdrvr/build
/opt/Qt/6.8.1/gcc_64/bin/qt-cmake -S ../../ -B ./
cmake --build ./
```

Optionally, you can also install it
```
cmake --install ./
```


# 3rd Party Credits
"White Textured Wallpaper" by wwarby is licensed under CC BY 2.0.

"win11" icons have been donated by Icons8.com
