# SkyAlt
Accessible database and analytics

![table example image](https://skyalt.com/screens/table.png)

![chart example image](https://skyalt.com/screens/chart.png)


## Installing
There is **no installation**. Extract downloaded *.zip file from the website.

Linux users: *Right-click on skyalt file. Select "Properties". Go into "Permissions" tab. Turn on "Allow executing file as program".*

## Compilation

### Linux
cd linux
sh install_libs
sh build_release
Add "languages" and "eula" folders to same folder with binary file

### Windows
Visual studio project is in "windows/SkyAlt.sln".
Goto project properties
Change paths in: C/C++ -> General -> Additional Include Directories
Change paths in: Linker -> General -> Additional Library Directories
Add "languages" and "eula" folders to same folder with binary file
All libraries can be download with Microsoft vcpkg.

## Website
https://www.skyalt.com/

## Issues
Feel free to file any issues in this repository. Before you file an issue, please check if the issue already exists. Please include:
1. Operating system
2. SkyAlt version(in menu->about)
3. If possible, the minimal project(in .zip) to reproduce the bug or screenshot/video

## E-mail
milan@skyalt.com

##License
Bussiness Source License(https://mariadb.com/bsl11/)
More in LICENSE file.
