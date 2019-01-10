cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME test)

project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH test.ino)

#### Uncomment below additional settings as needed.
 set(${CMAKE_PROJECT_NAME}_BOARD mega)
 set(${CMAKE_PROJECT_NAME}_PORT COM5)
 set(mega.build.mcu atmega2560)
 set(mega.upload.protocol wiring)
 set(mega.upload.speed 115200)

include_directories("C:\\Users\\Bruger\\Documents\\Arduino\\libraries")


generate_arduino_firmware(${CMAKE_PROJECT_NAME})
