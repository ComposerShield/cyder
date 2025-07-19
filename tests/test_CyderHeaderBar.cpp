
#include <gtest/gtest.h>

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../source/CyderAudioProcessor.hpp"
#include "../source/CyderAudioProcessorEditor.hpp"
#include "../source/CyderHeaderBar.hpp"
#include "../source/HotReloadThread.hpp"

#include <gtest/gtest.h>

//==============================================================================

TEST(CyderHeaderBarGetCurrentStatusString, CheckStatusStringAfterReloadingPlugin)
{
    CyderAudioProcessor cyderProcessor;
    std::unique_ptr<CyderAudioProcessorEditor> editor;
    editor.reset(dynamic_cast<CyderAudioProcessorEditor*>(cyderProcessor.createEditor()));
    ASSERT_TRUE(editor != nullptr);
    
    const auto& headerBar = editor->getHeaderBar();
    
    constexpr auto numChannels = 2;
    constexpr auto sampleRate  = 44100.0;
    constexpr auto blocksize   = 1024;
    
    // Set stereo
    cyderProcessor.setPlayConfigDetails(numChannels, numChannels, sampleRate, blocksize);
    
    juce::File currentFile(__FILE__);
    // ExamplePlugin.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getParentDirectory() // "tests"
        .getParentDirectory() // root dir
        .getChildFile("ExamplePlugin")
        .withFileExtension("vst3");
    
    // Load example plugin
    {
        auto result = cyderProcessor.loadPlugin(pluginFile.getFullPathName());
        ASSERT_TRUE(result);
        EXPECT_EQ(cyderProcessor.getCurrentStatus(), CyderStatus::successfullyLoadedPlugin);
    }
    
    // Check status string after a short wait for successfullyLoadedPlugin
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(500);
        
        const auto statusString = headerBar.getCurrentStatusString().toStdString();
        const auto expectedString = headerBar.getStatusAsString(CyderStatus::successfullyLoadedPlugin);
        
        jassert(statusString == expectedString);
        EXPECT_EQ(statusString, expectedString);
    }
    
    // Trigger plugin modified callback
    {
        auto* thread = dynamic_cast<HotReloadThread*>(cyderProcessor.getHotReloadThread());
        thread->onPluginChangeDetected();
    }
    
    // Detect Change and Automatically Reload Plugin
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(200);
        EXPECT_EQ(cyderProcessor.getCurrentStatus(), CyderStatus::successfullyReloadedPlugin);
    }
    
    // Check status string after a short wait for successfullyReloadedPlugin
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(500);
        
        const auto statusString = headerBar.getCurrentStatusString().toStdString();
        const auto expectedString = headerBar.getStatusAsString(CyderStatus::successfullyReloadedPlugin);
        
        jassert(statusString == expectedString);
        EXPECT_EQ(statusString, expectedString);
    }
}
