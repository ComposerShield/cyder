
#include <gtest/gtest.h>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

#include "../source/Utilities.hpp"

//==============================================================================

TEST(UtilitiesFindPluginDescription, ReturnsDescriptionForValidPlugin)
{
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::File currentFile(__FILE__);
    // Cyder.vst3 must have already been built and copied into root directory
    juce::File pluginFile = currentFile.getSiblingFile("../Cyder.vst3");

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

    EXPECT_THROW(
    {
        (void)Utilities::findPluginDescription(nonExistentFile, formatManager);
    }
    , std::runtime_error);
}
