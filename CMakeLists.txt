cmake_minimum_required(VERSION 3.6)

project(isolate-network LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
# For historical reasons, the libnl pkg-config target uses version 3.0 even for later versions
pkg_check_modules(libnl REQUIRED IMPORTED_TARGET libnl-3.0 libnl-route-3.0)

add_executable(isolate-network isolate-network.cpp)
target_compile_features(isolate-network PRIVATE cxx_std_17)
target_link_libraries(isolate-network PRIVATE PkgConfig::libnl)
