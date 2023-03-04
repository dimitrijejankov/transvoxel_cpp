cmake_minimum_required(VERSION 3.18.1)
project(Voxel VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp transvoxel/transition_sides.cpp 
                                        transvoxel/voxel_coordinates.cpp
                                        transvoxel/implementation/aux_tables.cpp 
                                        transvoxel/implementation/rotation.cpp)