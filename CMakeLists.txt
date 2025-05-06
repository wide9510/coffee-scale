cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(coffee-scale)

target_sources(app PRIVATE 
   src/main.c
   src/nau7802.c 
   src/button.c
   src/nvs.c
   src/display.c
   src/scale.c)

target_include_directories(app
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)


