project(bmp)
cmake_minimum_required(VERSION 3.2)

add_library(bmp STATIC
	source/bmp.cpp
)

target_include_directories(bmp PUBLIC include)

add_executable(binprint
	source/binprint.cpp
)

target_link_libraries(binprint PRIVATE bmp)
