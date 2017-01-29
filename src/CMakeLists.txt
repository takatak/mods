cmake_minimum_required(VERSION 2.8)

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})


add_definitions("-Wall -Os -march=native") # <- gcc専用?　

# add_definitions("-Wall -O") # <- 一般的?
add_executable(mods
  mods.c
)

# Link the target to the GTK+ libraries
target_link_libraries(mods ${GTK3_LIBRARIES})

INSTALL(TARGETS mods RUNTIME DESTINATION bin)
