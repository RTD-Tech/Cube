# Vendor dependencies
cmake_minimum_required(VERSION 3.16)

# Check if SDL directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/CMakeLists.txt")
    message(STATUS "Building SDL3...")
    
    # Disable SDL3 examples, tests, etc. to speed up build
    set(SDL_SHARED OFF CACHE BOOL "Build SDL3 as shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL3 as static library" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL3 test library" FORCE)
    
    add_subdirectory(SDL)
    
    # Ensure SDL_image builds as static to match our SDL3 static build
    set(BUILD_SHARED_LIBS OFF)
    
    # Disable heavy dependencies that require additional tools/libraries
    set(SDLIMAGE_AVIF OFF CACHE BOOL "Enable AVIF image support" FORCE)
    set(SDLIMAGE_JXL OFF CACHE BOOL "Enable JPEG XL image support" FORCE) 
    set(SDLIMAGE_WEBP OFF CACHE BOOL "Enable WebP image support" FORCE)
    set(SDLIMAGE_TIF OFF CACHE BOOL "Enable TIFF image support" FORCE)
    
    # Keep basic formats enabled
    set(SDLIMAGE_BMP ON CACHE BOOL "Enable BMP image support" FORCE)
    set(SDLIMAGE_PNG ON CACHE BOOL "Enable PNG image support" FORCE)
    set(SDLIMAGE_JPG ON CACHE BOOL "Enable JPEG image support" FORCE)
    
    add_subdirectory(SDL_image)

    # Disable heavy dependencies
    set(SDLTTF_HARFBUZZ OFF CACHE BOOL "Enable HarfBuzz support" FORCE)
    set(SDLTTF_FREETYPE_VENDORED ON CACHE BOOL "Use vendored FreeType" FORCE)
    
    add_subdirectory(SDL_ttf)
else()
    message(FATAL_ERROR "SDL subdirectory not found. Please add SDL3 source to Vendor/SDL.")
endif()

# Check if libzmq directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libzmq/CMakeLists.txt")
    message(STATUS "Building libzmq...")
    
    # Configure libzmq build options
    set(BUILD_SHARED OFF CACHE BOOL "Build libzmq as shared library" FORCE)
    set(BUILD_STATIC ON CACHE BOOL "Build libzmq as static library" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Build libzmq tests" FORCE)
    set(ENABLE_CPACK OFF CACHE BOOL "Enable libzmq packaging" FORCE)
    set(WITH_DOCS OFF CACHE BOOL "Build libzmq documentation" FORCE)
    
    add_subdirectory(libzmq)
else()
    message(FATAL_ERROR "libzmq subdirectory not found. Please add libzmq source to Vendor/libzmq.")
endif()

# Check if box2d directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/box2d/CMakeLists.txt")
    message(STATUS "Building Box2D...")

    # Configure box2d build options
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Box2D as shared library" FORCE)
    set(BOX2D_SAMPLES OFF CACHE BOOL "Build Box2D samples" FORCE)
    set(BOX2D_BENCHMARKS OFF CACHE BOOL "Build Box2D benchmarks" FORCE)
    set(BOX2D_DOCS OFF CACHE BOOL "Build Box2D documentation" FORCE)
    set(BOX2D_UNIT_TESTS OFF CACHE BOOL "Build Box2D unit tests" FORCE)
    set(BOX2D_VALIDATE OFF CACHE BOOL "Enable heavy validation" FORCE)

    add_subdirectory(box2d)
else()
    message(FATAL_ERROR "box2d subdirectory not found. Please add Box2D source to Vendor/box2d.")
endif()
