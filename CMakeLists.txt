cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(coffee-scale)

target_sources(app PRIVATE 
   src/main.c 
   src/nau7802.c 
   src/button.c
   src/nvs.c)


