cmake_minimum_required(VERSION 3.16)
project(C_OS VERSION 4.0.2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

message(STATUS "C-OS 4.0.2 uses the Makefile as the single build source of truth.")
message(STATUS "Available targets: kernel, iso, run, clean")

add_custom_target(kernel
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} kernel
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building C-OS kernel via project Makefile"
)

add_custom_target(iso
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} iso
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS kernel
    COMMENT "Building C-OS ISO via project Makefile"
)

add_custom_target(run
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} run
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS iso
    COMMENT "Running C-OS in QEMU via project Makefile"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} clean
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Cleaning C-OS build artifacts via project Makefile"
)
