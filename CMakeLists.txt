cmake_minimum_required(VERSION 3.10)
project(MGMExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(${X11_INCLUDE_DIR} ${CAIRO_INCLUDE_DIRS})

add_library(mgm STATIC mgm.cpp)
target_link_libraries(mgm ${X11_LIBRARIES} ${CAIRO_LIBRARIES})

add_executable(example example.cpp)
target_link_libraries(example mgm)
