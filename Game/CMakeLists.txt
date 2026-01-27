# Game console application
cmake_minimum_required(VERSION 3.16)

# Collect all source files for the game
file(GLOB_RECURSE GAME_SOURCES 
    "Source/*.cpp"
    "Source/*.c"
)

file(GLOB_RECURSE GAME_HEADERS 
    "Source/*.h"
    "Source/*.hpp"
)

# Create the game executable
add_executable(Game 
    ${GAME_SOURCES}
    ${GAME_HEADERS}
)

# Set target properties
target_compile_features(Game PRIVATE cxx_std_20)

# Include directories
target_include_directories(Game 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Link with the engine library
target_link_libraries(Game 
    PRIVATE 
        Engine::Engine
)

# Set compile options
if(MSVC)
    target_compile_options(Game PRIVATE /W4)
    # Ensure console subsystem for MSVC
    set_target_properties(Game PROPERTIES 
        WIN32_EXECUTABLE FALSE
    )
else()
    target_compile_options(Game PRIVATE -Wall -Wextra -Wpedantic -pthread)
endif()

# Set the executable name
set_target_properties(Game PROPERTIES 
    OUTPUT_NAME "Square"
    DEBUG_POSTFIX "_d"
)
