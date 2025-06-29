cmake_minimum_required(VERSION 3.15)

project(Cyder VERSION 0.1.0 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE from the extern directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern/juce")

# Create a console application target
juce_add_console_app(Cyder_App
    PRODUCT_NAME "Cyder"
)

# Collect all source files in the source directory
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
)

# Group all source files under an Xcode folder named 'source'
source_group("source" FILES ${SOURCE_FILES})
  
# Group JUCE core module implementation files under 'JUCE Modules'
file(GLOB_RECURSE JUCE_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/juce/modules/juce_core/*.cpp"
)
source_group("JUCE Modules" FILES ${JUCE_CORE_SOURCES})

# Specify your source file(s)
target_sources(Cyder_App
    PRIVATE
        ${SOURCE_FILES}
)

# Expose the extern folder for header includes
target_include_directories(Cyder_App
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

# Link against JUCE core module
target_link_libraries(Cyder_App
    PRIVATE
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_graphics
)