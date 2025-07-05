/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderAudioProcessor.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-29
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "CyderAudioProcessor.hpp"

#include "CyderAudioProcessorEditor.hpp"
#include "CyderAssert.hpp"
#include "HotReloadThread.hpp"
#include "Utilities.hpp"

#include <limits>
#include <memory>
#include <utility>

//==============================================================================
CyderAudioProcessor::CyderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : juce::AudioProcessor (juce::AudioProcessor::BusesProperties()
 #if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
 #endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
 #endif
     )
#endif
{
    formatManager.addDefaultFormats();
}

CyderAudioProcessor::~CyderAudioProcessor()
{
    if (hotReloadThread != nullptr)
        hotReloadThread->stopThread(1000);
    unloadPlugin();
}

void CyderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (wrappedPlugin == nullptr)
        return;
    
    if (getTotalNumInputChannels() != wrappedPlugin->getTotalNumInputChannels()
        || getTotalNumOutputChannels() != wrappedPlugin->getTotalNumOutputChannels())
    {
        wrappedPlugin->setPlayConfigDetails(getTotalNumInputChannels(),
                                            getTotalNumOutputChannels(),
                                            sampleRate,
                                            samplesPerBlock);
    }
    
    wrappedPlugin->prepareToPlay(sampleRate, samplesPerBlock);
}

void CyderAudioProcessor::releaseResources()
{
    if (wrappedPlugin == nullptr)
        return;
    
    wrappedPlugin->releaseResources();
}

bool CyderAudioProcessor::isBusesLayoutSupported (const BusesLayout& /*layouts*/) const
{
    return true;
}

void CyderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    std::scoped_lock<std::mutex> lock(wrappedPluginMutex);
    if (wrappedPlugin == nullptr)
        return;
    
    wrappedPlugin->processBlock(buffer, midiMessages);
}

juce::AudioProcessorEditor* CyderAudioProcessor::createEditor()
{
    return new CyderAudioProcessorEditor (*this);
}

bool CyderAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String CyderAudioProcessor::getName() const
{
    return "Cyder";
}

bool CyderAudioProcessor::acceptsMidi() const
{
    return true;
}

bool CyderAudioProcessor::producesMidi() const
{
    return false;
}

bool CyderAudioProcessor::isMidiEffect() const
{
    return false;
}

double CyderAudioProcessor::getTailLengthSeconds() const
{
    return std::numeric_limits<double>::infinity();
}

int CyderAudioProcessor::getNumPrograms()
{
    return 1;
}

int CyderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CyderAudioProcessor::setCurrentProgram(int /*index*/) // NOLINT (false positive)
{
}

const juce::String CyderAudioProcessor::getProgramName(int /*index*/) // NOLINT (false positive)
{
    return {};
}

void CyderAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

void CyderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (wrappedPlugin == nullptr)
        return;

    // Build XML root element
    auto xml = std::make_unique<juce::XmlElement>(JucePlugin_Name);
    xml->setAttribute("version", JucePlugin_VersionString);
    xml->setAttribute("pluginFilePath", currentPluginFile.getFullPathName());

    // Serialize wrapped plugin state
    juce::MemoryBlock pluginData;
    wrappedPlugin->getStateInformation(pluginData);
    auto base64Data = juce::Base64::toBase64(pluginData.getData(), pluginData.getSize());

    // Embed the base64-encoded state
    auto* stateElem = xml->createNewChildElement("WrappedPluginState");
    stateElem->addTextElement(base64Data);

    // Convert XML to UTF-8 and write into destData
    auto xmlString = xml->toString();
    destData.reset();
    destData.append(xmlString.toRawUTF8(), xmlString.getNumBytesAsUTF8());
}

void CyderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Parse incoming XML
    auto xmlString = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
    std::unique_ptr<juce::XmlElement> xml { juce::XmlDocument::parse(xmlString) };
    
    if (xml == nullptr || ! xml->hasTagName("Cyder"))
        return;
    
    // Restore plugin file path
    {
        auto savedPathString = xml->getStringAttribute("pluginFilePath");
        unloadPlugin();
        loadPlugin(savedPathString);
    }
    
    if (wrappedPlugin == nullptr) // something went wrong
        return;

    // Decode and restore wrapped plugin state
    if (auto* stateElem = xml->getChildByName("WrappedPluginState"))
    {
        auto base64Data = stateElem->getAllSubText();
        juce::MemoryBlock pluginData;
        {
            juce::MemoryOutputStream stream(pluginData, false);
            juce::Base64::convertFromBase64(stream, base64Data);
        }
        if (wrappedPlugin != nullptr)
            wrappedPlugin->setStateInformation(pluginData.getData(),
                                               static_cast<int>(pluginData.getSize()));
    }
}

bool CyderAudioProcessor::loadPlugin(const juce::String& pluginPath)
{
    // CFBundle does not like it if we attempt to load a dll outside the message thread
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    
    DBG("pluginPath: " << pluginPath);
    
    try
    {
        juce::ScopedJuceInitialiser_GUI libraryInitialiser;
        juce::File pluginFile(pluginPath);
        const bool reloadingSamePlugin = pluginFile == currentPluginFile;
        
        // Copy plugin to temp with a random hash appended
        auto tempPluginFile = Utilities::copyPluginToTemp(pluginFile);
        
        const auto sampleRate = getSampleRate();
        const auto blockSize  = getBlockSize();

        // Only supporting VST3
        juce::AudioProcessor::setTypeOfNextNewPlugin(wrapperType_VST3);

        auto description  = Utilities::findPluginDescription(tempPluginFile, formatManager);
        
        description.numInputChannels  = getTotalNumInputChannels();
        description.numOutputChannels = getTotalNumOutputChannels();
        
        auto instance     = Utilities::createInstance(description, formatManager, sampleRate, blockSize);
        auto editor       = instance->createEditor();
        auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor());
        
        instance->setPlayConfigDetails(getTotalNumInputChannels(),
                                       getTotalNumOutputChannels(),
                                       sampleRate,
                                       blockSize);
        
        instance->prepareToPlay(sampleRate, blockSize);

        // Make sure nothing above threw exception before swapping out current members
        
        currentPluginFile = pluginFile;
        if (reloadingSamePlugin)
            transferPluginState(*instance);
        
        if (hotReloadThread != nullptr)
            hotReloadThread->stopThread(1000); // don't hot reload while we're loading
        
        if (cyderEditor != nullptr)
            cyderEditor->unloadWrappedEditor(getWrappedPluginEditor(),
                                             /*shouldCacheSize*/ reloadingSamePlugin);
        
        wrappedPluginEditor.reset();
        
        {
            std::scoped_lock<std::mutex> lock(wrappedPluginMutex);
            wrappedPlugin.reset(instance.release());
            setLatencySamples(wrappedPlugin->getLatencySamples());
        }
        wrappedPluginEditor.reset(std::move(editor));
        
        if (cyderEditor != nullptr)
            cyderEditor->loadWrappedEditor(getWrappedPluginEditor());
        
        hotReloadThread = std::make_unique<HotReloadThread>(pluginFile); // auto starts thread
        
        hotReloadThread->onPluginChangeDetected = [&]
        {
            juce::MessageManager::callAsync([&]
            {
                try
                {
                    loadPlugin(hotReloadThread->getFullPluginPath());
                }
                catch(const std::exception& e)
                {
                    juce::Logger::writeToLog(e.what());
                    CYDER_ASSERT_FALSE;
                    // Failed to reload plugin...
                }
            });
        };
    }
    catch(const std::exception& e)
    {
        juce::Logger::writeToLog(e.what());
        return false;
    }
    
    juce::MessageManager::getInstance()->setCurrentThreadAsMessageThread();
    JUCE_ASSERT_MESSAGE_THREAD
    
    return true;
}

void CyderAudioProcessor::unloadPlugin()
{
    if (auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor()))
        cyderEditor->unloadWrappedEditor(getWrappedPluginEditor(),
                                         /*shouldCacheSize*/ false);
    
    wrappedPluginEditor.reset();
    
    {
        std::scoped_lock<std::mutex> lock(wrappedPluginMutex);
        wrappedPlugin.reset();
    }
}

juce::AudioProcessor* CyderAudioProcessor::getWrappedPluginProcessor() const noexcept
{
    return dynamic_cast<juce::AudioProcessor*>(wrappedPlugin.get());
}

juce::AudioProcessorEditor* CyderAudioProcessor::getWrappedPluginEditor() const noexcept
{
    return wrappedPluginEditor.get();
}

void CyderAudioProcessor::transferPluginState(juce::AudioProcessor& destinationProcessor) noexcept
{
    juce::MemoryBlock memoryBlock;
    wrappedPlugin->getStateInformation(memoryBlock);
    destinationProcessor.setStateInformation(memoryBlock.getData(),
                                             static_cast<int>(memoryBlock.getSize()));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyderAudioProcessor();
}
