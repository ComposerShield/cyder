cmake_minimum_required(VERSION 3.15)

project(Cyder VERSION 0.1.0 LANGUAGES C CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE from the extern directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern/juce")

# Create a plugin target
juce_add_plugin(Cyder_Plugin
    PRODUCT_NAME "Cyder"
    FORMATS VST3 Standalone
    
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_COPY_DIR "${CMAKE_SOURCE_DIR}"

    COMPANY_NAME "Cyder"
    COMPANY_COPYRIGHT "Copyright (c) 2025"
    COMPANY_WEBSITE "https://github.com/ComposerShield/cyder"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    DISABLE_AAX_BYPASS FALSE
    DISABLE_AAX_MULTI_MONO FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE

    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx

    BUNDLE_ID "com.Antares.AutoTuneCore"
    PLUGIN_MANUFACTURER_CODE Cydr               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Cydr                            # A unique four-character plugin id with exactly one upper-case character
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
target_sources(Cyder_Plugin
    # public so it can be used by unit test executable
    PUBLIC
        ${SOURCE_FILES}
)

# Expose the extern folder for header includes
target_include_directories(Cyder_Plugin
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

set(CYDER_JUCE_LIBS
    juce::juce_audio_processors
    juce_audio_devices
    juce_audio_utils
    juce::juce_core
    juce::juce_graphics
)

# Link against JUCE core module
target_link_libraries(Cyder_Plugin
     # public so it can be used by unit test executable
    PUBLIC
        ${CYDER_JUCE_LIBS}
)

set(CYDER_COMPILE_DEFS
    # Copied from JUCE AudioPluginHost
    JUCE_ALSA=1
    JUCE_CONTENT_SHARING=1
    JUCE_DIRECTSOUND=1
    JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_ARA=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CAMERA=0
    JUCE_USE_CDBURNER=0
    JUCE_USE_CDREADER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=1
    JUCE_VST3_HOST_CROSS_PLATFORM_UID=1
    JUCE_WASAPI=1
    JUCE_WEB_BROWSER=0
    PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples"
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS/iOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
)

target_compile_definitions(Cyder_Plugin 
    # public so it can be used by unit test executable
    PUBLIC
        ${CYDER_COMPILE_DEFS}
)

# Disable macOS App Sandbox and Hardened Runtime for the standalone
if (APPLE)
    set_target_properties(Cyder_Plugin PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_APP_SANDBOX "NO"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "NO"
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

  # Enable most warnings, upgrade to errors
  target_compile_options(Cyder_Plugin 
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
  )

endif()

# Download and configure GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Unit test executable
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

source_group("tests" FILES ${TEST_SOURCES})

add_executable(Cyder_Tests ${TEST_SOURCES})

target_link_libraries(Cyder_Tests
    PRIVATE
        GTest::gtest_main
        Cyder_Plugin
)

target_include_directories(Cyder_Tests 
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/source"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

add_test(NAME Cyder_Tests COMMAND Cyder_Tests)