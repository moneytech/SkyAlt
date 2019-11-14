# SkyAlt
A private place for organizing life & work data

![alt text](https://raw.githubusercontent.com/MilanSuk/SkyAlt_web/master/screenshots/high_1.png)


## Installing
There is **no installation**. Extract downloaded *.zip file from the website.

Linux users: *Right-click on skyalt file. Select "Properties". Go into "Permissions" tab. Turn on "Allow executing file as program".*

## Compilation

### Linux
cd linux
sh build_release
Add "languages" and "licenses" folders to same folder with binary file

### Windows
Visual studio project is in "windows/SkyAlt.sln".
Goto project properties
Change paths in: C/C++ -> General -> Additional Include Directories
Change paths in: Linker -> General -> Additional Library Directories
Add "languages" and "licenses" folders to same folder with binary file
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

## Disclaimer
SkyAlt is currently at an public beta stage of development. This means that some important features are missing and current features can be changed as well, which can break your project. Please don’t use it for anything important!

## Contributor License Agreement
As a Contributor, you need to accept the Contributor License Agreement(CLA). We will ask you to sign the CLA after you make your first pull request. Any work intentionally submitted for inclusion in SkyAlt shall be licensed under the CLA.
