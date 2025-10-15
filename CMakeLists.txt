cmake_minimum_required(VERSION 3.10)

project(acrostream C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{BUILDROOT})
    message(FATAL_ERROR "BUILDROOT environment variable not set. Please specify the path to your Buildroot toolchain.")
endif()

set(TOOLCHAIN_PREFIX "$ENV{BUILDROOT}/bin/")
set(SYSROOT_PATH "$ENV{BUILDROOT}/arm-openipc-linux-gnueabihf/sysroot")

set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}arm-openipc-linux-gnueabihf-gcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT_PATH} -Wall -Wextra -O3")

file(GLOB SOURCE_FILES "*.c")

add_executable(acrostream ${SOURCE_FILES})
