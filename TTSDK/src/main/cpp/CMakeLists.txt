
cmake_minimum_required(VERSION 3.22.1)

project("pet")

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp)

find_library(log-lib log)
find_library(android-lib android)
find_library(jnigraphics-lib jnigraphics)


target_link_libraries(${CMAKE_PROJECT_NAME}
        ${log-lib}
        ${android-lib}
        ${jnigraphics-lib})