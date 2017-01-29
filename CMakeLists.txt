cmake_minimum_required(VERSION 2.8)
project(mods C)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 3)

# make の出力を強制 (デバッグ用)
set(CMAKE_VERBOSE_MAKEFILE 1)


# サブディレクトリ (ソースファイル格納) を追加
add_subdirectory(src)
add_subdirectory(man)

