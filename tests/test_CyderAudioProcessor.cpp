
#include <gtest/gtest.h>

#if ENABLE_QITI
#include <qiti_include.hpp>

#include "../example/plugin/source/ExamplePluginProcessor.h"
#endif

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../source/CyderAudioProcessor.hpp"

#include <gtest/gtest.h>

//==============================================================================

TEST(CyderAudioProcessorGetAndSetStateInformation, SaveAndRestoreData)
{    
    CyderAudioProcessor cyderProcessor;
    
    juce::File currentFile(__FILE__);
    // ExamplePlugin.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getParentDirectory() // "tests"
                                       .getParentDirectory() // root dir
                                       .getChildFile("ExamplePlugin")
                                       .withFileExtension("vst3");

    {
        auto result = cyderProcessor.loadPlugin(pluginFile.getFullPathName());
        ASSERT_TRUE(result);
    }
    
#if ENABLE_QITI
    qiti::ScopedQitiTest qitiTest; // TODO: figure out a way to move this up
    
    auto examplePluginGetStateInformation =
        qiti::FunctionData::getFunctionData<&ExamplePluginAudioProcessor::setStateInformation>();
    auto examplePluginSetStateInformation =
        qiti::FunctionData::getFunctionData<&ExamplePluginAudioProcessor::getStateInformation>();
#endif
    
    auto* exampleProcessor = cyderProcessor.getWrappedPluginProcessor();

    // Insert generic plugin state XML into saveState
    {
        juce::MemoryBlock rawSaveState;
        
        juce::XmlElement pluginStateXml("ExamplePluginState");
        pluginStateXml.setAttribute("gain", 0.5f);
        pluginStateXml.setAttribute("mix", 0.8f);
        pluginStateXml.setAttribute("pluginVersion", "1.0.0");
        juce::AudioProcessor::copyXmlToBinary(pluginStateXml, rawSaveState);
        
        // Set the state directly into the plugin processor
        // without having it wrapped by the VST3PluginInstance
        exampleProcessor->setStateInformation(rawSaveState.getData(),
                                              static_cast<int>(rawSaveState.getSize()));
    }
    
    // Session data is wrapped by VST3PluginInstance::getStateInformation()
    juce::MemoryBlock saveState;
    exampleProcessor->getStateInformation(saveState);
    
    // Retreive session data from Cyder
    {
    #if ENABLE_QITI
        ASSERT_EQ(examplePluginGetStateInformation->getNumTimesCalled(), 0);
    #endif
        
        juce::MemoryBlock retreivedState;
        cyderProcessor.getStateInformation(retreivedState);
        
    #if ENABLE_QITI
        // Cyder getStateInformation() calls wrapped plugin's getStateInformation()
        ASSERT_EQ(examplePluginGetStateInformation->getNumTimesCalled(), 1);
    #endif
        
        void* data = retreivedState.getData();
        auto sizeInBytes = static_cast<int>(retreivedState.getSize());
        
        auto xmlString = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
        std::unique_ptr<juce::XmlElement> xml { juce::XmlDocument::parse(xmlString) };
        
        ASSERT_TRUE(xml != nullptr);
        ASSERT_TRUE(xml->hasTagName("Cyder"));
        
        // Restore plugin file path
        {
            auto savedPathString = xml->getStringAttribute("pluginFilePath");
            
            ASSERT_TRUE(savedPathString == pluginFile.getFullPathName());
        }
        
        // Decode and restore wrapped plugin state
        auto* stateElem = xml->getChildByName("WrappedPluginState");
        ASSERT_TRUE(stateElem != nullptr);
        
        auto base64Data = stateElem->getAllSubText();
        juce::MemoryBlock pluginData;
        {
            juce::MemoryOutputStream stream(pluginData, false);
            juce::Base64::convertFromBase64(stream, base64Data);
        }
        ASSERT_TRUE(pluginData == saveState);
        
    #if ENABLE_QITI
        ASSERT_EQ(examplePluginGetStateInformation->getNumTimesCalled(), 0);
        
        cyderProcessor.setStateInformation(retreivedState.getData(),
                                           static_cast<int>(retreivedState.getSize()));
        
        // Cyder setStateInformation() calls wrapped plugin's setStateInformation()
        ASSERT_EQ(examplePluginGetStateInformation->getNumTimesCalled(), 1);
    #endif
    }
}

TEST(CyderAudioProcessorUnloadPlugin, HotReloadThreadIsStoppedAfterUnload)
{
    CyderAudioProcessor cyderProcessor;
    
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
        EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::successfullyLoadedPlugin);
    }
    
    // Ensure HotReloadThread exists and is running
    {
        auto* hotReloadThread = cyderProcessor.getHotReloadThread();
        EXPECT_TRUE(hotReloadThread != nullptr);
        if (hotReloadThread != nullptr)
            EXPECT_TRUE(hotReloadThread->isThreadRunning());
    }
    
    // Unload Plugin
    cyderProcessor.unloadPlugin();
    EXPECT_TRUE(cyderProcessor.getWrappedPluginProcessor() == nullptr);
    EXPECT_TRUE(cyderProcessor.getWrappedPluginEditor() == nullptr);
    EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::idle);
    
    // Ensure HotReloadThread no longer exists (we wouldn't want to reload after unloading!)
    {
        auto* hotReloadThread = cyderProcessor.getHotReloadThread();
        EXPECT_EQ(hotReloadThread, nullptr);
    }
}

TEST(CyderAudioProcessorLoadPlugin, WrappedPluginMatchesIOMono)
{
    CyderAudioProcessor cyderProcessor;
    
    constexpr auto numChannels = 1;
    constexpr auto sampleRate  = 44100.0;
    constexpr auto blocksize   = 1024;
    
    // Set mono
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
        EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::successfullyLoadedPlugin);
    }
    
    // Expect wrapped processor to be mono
    auto wrappedProcessor = cyderProcessor.getWrappedPluginProcessor();
    EXPECT_EQ(1, wrappedProcessor->getTotalNumInputChannels());
    EXPECT_EQ(1, wrappedProcessor->getTotalNumOutputChannels());
}

TEST(CyderAudioProcessorLoadPlugin, WrappedPluginMatchesIOStereo)
{
    CyderAudioProcessor cyderProcessor;
    
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
        EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::successfullyLoadedPlugin);
    }
    
    // Expect wrapped processor to be mono
    auto wrappedProcessor = cyderProcessor.getWrappedPluginProcessor();
    EXPECT_EQ(2, wrappedProcessor->getTotalNumInputChannels());
    EXPECT_EQ(2, wrappedProcessor->getTotalNumOutputChannels());
}

#if JUCE_MAC // TODO: figure out a way to make cleanup work on PC
TEST(CyderAudioProcessorUnloadPlugin, DeleteCopiedPluginWhenNoLongerNeeded)
{
    CyderAudioProcessor cyderProcessor;
    
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
        EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::successfullyLoadedPlugin);
    }
    
    // Ensure our copied plugin path exists
    auto copiedPath = cyderProcessor.getCurrentWrappedPluginPathCopy();
    ASSERT_TRUE(copiedPath.exists());
    
    // Unload Plugin
    cyderProcessor.unloadPlugin();
    EXPECT_TRUE(cyderProcessor.getWrappedPluginProcessor() == nullptr);
    EXPECT_TRUE(cyderProcessor.getWrappedPluginEditor() == nullptr);
    EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::idle);
    
    // Ensure our copied plugin path was deleted
    ASSERT_FALSE(copiedPath.exists());
}

TEST(CyderAudioProcessorDestructor, DeleteCopiedPluginWhenCyderIsDestroyed)
{
    juce::File copiedPath;
    
    {
        CyderAudioProcessor cyderProcessor;
        
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
        }
        
        // Ensure our copied plugin path exists
        copiedPath = cyderProcessor.getCurrentWrappedPluginPathCopy();
        ASSERT_TRUE(copiedPath.exists());
    } // Processor deleted
    
    // Ensure our copied plugin path was deleted
    ASSERT_FALSE(copiedPath.exists());
}
#endif // JUCE_MAC

TEST(CyderAudioProcessorAudioProcessorChanged, LatencySyncedWithWrappedPlugin)
{
    CyderAudioProcessor cyderProcessor;
    
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
    
    // Latency should be 0 before wrapping plugin
    ASSERT_EQ(0, cyderProcessor.getLatencySamples());
    
    // Load example plugin
    {
        auto result = cyderProcessor.loadPlugin(pluginFile.getFullPathName());
        ASSERT_TRUE(result);
        EXPECT_TRUE(cyderProcessor.getCurrentStatus() == CyderStatus::successfullyLoadedPlugin);
    }
    
    // Latency should still be 0
    ASSERT_EQ(0, cyderProcessor.getLatencySamples());
    
    // Set latency in wrapped plugin
    auto* wrappedPlugin = cyderProcessor.getWrappedPluginProcessor();
    constexpr auto testLatencyAmount = 256;
    wrappedPlugin->setLatencySamples(testLatencyAmount);
    EXPECT_EQ(testLatencyAmount, cyderProcessor.getLatencySamples());
}
