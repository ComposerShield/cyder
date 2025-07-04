
#include <gtest/gtest.h>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

#include "../source/CyderAssert.hpp"
#include "../source/Utilities.hpp"

//==============================================================================

TEST(UtilitiesFindPluginDescription, ReturnsDescriptionForValidPlugin)
{
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::File currentFile(__FILE__);
    // ExamplePlugin.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getSiblingFile("../ExamplePlugin.vst3");

    // Ensure the plugin file exists before testing
    ASSERT_TRUE(pluginFile.exists());

    juce::PluginDescription desc;
    EXPECT_NO_THROW(
    {
        desc = Utilities::findPluginDescription(pluginFile, formatManager);
    }
    );

    // The description identifier should match the plugin file path
    EXPECT_EQ(desc.fileOrIdentifier, pluginFile.getFullPathName());
}

TEST(UtilitiesFindPluginDescription, ThrowsWhenPluginNotFound)
{
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::File nonExistentFile("/path/to/nonexistent.vst3");

    // Turn OFF jasserts for testing bad input
    ScopedDisableCyderAssert disableJasserts;
    
    EXPECT_THROW(
    {
        (void)Utilities::findPluginDescription(nonExistentFile, formatManager);
    }
    , std::runtime_error);
}

TEST(UtilitiesCreateInstance, ReturnsInstanceForValidDescription)
{
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::File currentFile(__FILE__);
    juce::File pluginFile = currentFile.getSiblingFile("../ExamplePlugin.vst3");

    ASSERT_TRUE(pluginFile.exists());

    juce::PluginDescription description;
    EXPECT_NO_THROW(
    {
        description = Utilities::findPluginDescription(pluginFile, formatManager);
    });
    
    std::unique_ptr<juce::AudioPluginInstance> instance;
    EXPECT_NO_THROW(
    {
        instance = Utilities::createInstance(description, formatManager, 44100.0, 512);
    });

    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(instance->getName(), description.name);
}

TEST(UtilitiesCreateInstance, ThrowsWhenInvalidDescription)
{
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::PluginDescription invalidDesc;
    invalidDesc.fileOrIdentifier = "/path/to/nonexistent.vst3";
    invalidDesc.pluginFormatName = "VST3";
    invalidDesc.name = "NonexistentPlugin";
    invalidDesc.version = "1.0.0";

    // Turn OFF jasserts for testing bad input
    ScopedDisableCyderAssert disableJasserts;
    
    EXPECT_THROW(
    {
        (void)Utilities::createInstance(invalidDesc, formatManager, 44100.0, 512);
    }
    , std::runtime_error);
}

TEST(UtilitiesCopyPluginToTemp, CopiesFileSuccessfully)
{
    // Create mock vst3 (which is a directory/bundle)
    auto tempSource = juce::File::createTempFile("testPlugin.vst3");
    tempSource.createDirectory();
    
    // Create mock binary to stick inside the bundle, give us something within bundle to copy
    auto mockBinaryFile = tempSource.getChildFile("Contents").getChildFile("MacOS").getChildFile("testPlugin");
    [[maybe_unused]] auto result = mockBinaryFile.create();
    jassert(result);
    
    ASSERT_TRUE(tempSource.exists());

    juce::File copiedFile;
    EXPECT_NO_THROW(
    {
        copiedFile = Utilities::copyPluginToTemp(tempSource);
    });

    ASSERT_TRUE(copiedFile.exists());
    EXPECT_NE(copiedFile.getFullPathName(), tempSource.getFullPathName());
    EXPECT_EQ(copiedFile.getFileExtension(), tempSource.getFileExtension());
}

TEST(UtilitiesCopyPluginToTemp, ThrowsWhenCopyFails)
{
    juce::File nonExistentFile("/path/to/nonexistent.dll");

    // Turn OFF jasserts for testing bad input
    ScopedDisableCyderAssert disableJasserts;
    
    EXPECT_THROW(
    {
        (void)Utilities::copyPluginToTemp(nonExistentFile);
    }
    , std::runtime_error);
}
