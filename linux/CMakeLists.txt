cmake_minimum_required(VERSION 2.6)
project(skyalt)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0)

add_executable(skyalt ../src/main.c ../src/os.c ../src/std.c ../src/ui.c ../src/gui.c ../src/io.c ../src/db.c ../src/media.c ../src/map.c ../src/file.c ../src/log.c ../src/language.c ../src/license.c)

include_directories (${GTK3_INCLUDE_DIRS})
link_directories (${GTK3_LIBRARY_DIRS})
add_definitions (${GTK3_CFLAGS_OTHER})

target_include_directories(skyalt PRIVATE /usr/include/freetype2 /usr/include/freetype2/freetype /usr/include/opus /usr/include/ffmpeg)

target_link_libraries(skyalt z png curl ssl crypto X11 m freetype pthread asound swscale avformat avcodec avutil odbc xml2 ${GTK3_LIBRARIES})

install(TARGETS skyalt RUNTIME DESTINATION bin)
