cmake_minimum_required(VERSION 3.0)
project(jnienv)

add_library(jnienv SHARED jnienv.c jnienv.h)

target_link_libraries(jnienv log)

add_executable(jnienv_test main.c)
set_property(TARGET jnienv_test APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath=$ORIGIN")
target_link_libraries(jnienv_test jnienv)
