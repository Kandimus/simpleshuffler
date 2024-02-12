cmake_minimum_required(VERSION 3.5)

project(simpleshuffler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB files_cpp "sources/*.cpp")
file(GLOB files_h "sources/*.h")

file(GLOB su_cpp "external/smallUtils/stringex.cpp")

add_executable(${PROJECT_NAME}
    ${files_cpp} ${files_h}
    ${su_cpp}
)

target_include_directories(${PROJECT_NAME} PRIVATE "external/smallUtils")

include(GNUInstallDirs)
install(TARGETS simpleshuffler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
