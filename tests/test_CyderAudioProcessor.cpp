
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
    juce::File pluginFile = currentFile.getSiblingFile("../ExamplePlugin.vst3");

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
