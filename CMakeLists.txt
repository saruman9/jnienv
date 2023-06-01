cmake_minimum_required(VERSION 3.6.0)
project(jnienv)

add_library(jnienv SHARED src/jnienv.c)
target_include_directories(jnienv PUBLIC include)

target_link_libraries(jnienv log)

add_executable(jnienv_test src/main.c)
set_property(TARGET jnienv_test APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath=$ORIGIN")
target_link_libraries(jnienv_test jnienv)
