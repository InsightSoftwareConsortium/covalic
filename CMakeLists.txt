project( covalic )
cmake_minimum_required(VERSION 2.8)

option( BUILD_DOCUMENTATION "Generate Documentation for COVALIC" NO )

find_package( ITK )

add_subdirectory( Documentation )
add_subdirectory( Code )