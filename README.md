# SkyAlt
Accessible database and analytics

![table example image](https://skyalt.com/screens/table.png)

![chart example image](https://skyalt.com/screens/chart.png)


## Installing
There is **no installation**. Extract downloaded *.zip file from the website.

Linux users: *Right-click on skyalt file. Select "Properties". Go into "Permissions" tab. Turn on "Allow executing file as program".*

## Compilation

### Linux
1. cd linux
2. sh install_libs
3. sh build_release
4. Add "languages" and "eula" folders to same folder with binary file


### Windows
1. Visual studio project is in "windows/SkyAlt.sln".
2. Goto project properties
3. Change paths in: C/C++ -> General -> Additional Include Directories
4. Change paths in: Linker -> General -> Additional Library Directories
5. Add "languages" and "eula" folders to same folder with binary file
6. All libraries can be download with Microsoft vcpkg.


## Website
https://www.skyalt.com/

## Issues
Feel free to file any issues in this repository. Before you file an issue, please check if the issue already exists. Please include:
1. Operating system
2. SkyAlt version(in menu->about)
3. If possible, the minimal project(in .zip) to reproduce the bug or screenshot/video

## E-mail
milan@skyalt.com

## License
Bussiness Source License(https://mariadb.com/bsl11/).

More in LICENSE file.
