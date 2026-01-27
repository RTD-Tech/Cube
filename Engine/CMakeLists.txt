# Engine static library
cmake_minimum_required(VERSION 3.16)

# Collect all source files
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/**.cpp"
    "Source/**.c"
    "ThirdParty/Include/**.cpp"
    "ThirdParty/Include/**.c"
)

file(GLOB_RECURSE ENGINE_HEADERS 
    "Source/**.h"
    "Source/**.hpp"
    "ThirdParty/Include/**.h"
    "ThirdParty/Include/**.hpp"
)

# Create the engine static library
add_library(Engine STATIC 
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Set target properties
target_compile_features(Engine PUBLIC cxx_std_17)

# Include directories
target_include_directories(Engine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with dependencies
target_link_libraries(Engine 
    PUBLIC 
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        libzmq-static
        box2d::box2d
)

# Set compile options
if(MSVC)
    target_compile_options(Engine PRIVATE /W4)
else()
    target_compile_options(Engine PRIVATE -Wall -Wextra -Wpedantic -pthread)
endif()

# Create an alias for easier linking
add_library(Engine::Engine ALIAS Engine)
