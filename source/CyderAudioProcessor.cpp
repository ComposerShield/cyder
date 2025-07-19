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

#include <atomic>
#include <limits>
#include <memory>
#include <utility>

//==============================================================================

//#if JUCE_WINDOWS
static std::atomic<int> numInstances = 0;

/** */
void deleteCyderPluginsTempDirectoryAfterShutdown() noexcept
{
    // Cleanup entire fold
    auto cyderTempParentFolder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("CyderPlugins");
    if (! cyderTempParentFolder.exists())
        return;

    auto folderToDeleteAsString = cyderTempParentFolder.getFullPathName();

    // Build a cmd line that:
    //   1) Waits a couple of seconds (so the parent process has *really* exited).
    //   2) Then deletes the directory tree.
    // The Windows “timeout” command will sleep without needing extra tools:
    auto cmd = juce::String("cmd /C timeout /T 2 /NOBREAK >nul && ")
        + "rmdir /S /Q \"" + folderToDeleteAsString + "\"";

    juce::ChildProcess deleter;
    // By default JUCE’s ChildProcess uses CreateProcess with bInheritHandles = FALSE,
    // so the child won’t inherit *any* of your open handles.
    deleter.start(cmd);
}
//#endif  JUCE_WINDOWS

/**
  FYI - Must pass by value instead of const ref to ensure thread does not have dangling ref...
*/
void deleteStalePlugin(juce::File pluginToDelete) noexcept
{
    // Cleanup: Delete copied plugin
    if (pluginToDelete.exists())
    {
        #if JUCE_WINDOWS
        // Windows requires a pause before deleting the copied plugin so we 
        // can do that in another thread to not hold up main thread
        std::thread cleanupWorkerThread([=]
        {
            juce::Thread::sleep(1000);
        #endif

            bool didCleanUp = pluginToDelete.deleteRecursively();
            if (didCleanUp)
                DBG("Successfully deleted unloaded plugin copy");
            else
            {
                DBG("Failed to delete unloaded plugin copy");
                #if JUCE_MAC 
                // we cannot guarantee immediate cleanup on PC, so we'll try again later on shutdown
                CYDER_ASSERT_FALSE;
                #endif
            }

        #if JUCE_WINDOWS
        });
        cleanupWorkerThread.detach(); // Don't wait for it to finish
        #endif
    }
}

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
//#if JUCE_WINDOWS
    ++numInstances;
    jassert(numInstances==1);
//#endif
}

CyderAudioProcessor::~CyderAudioProcessor()
{
    if (hotReloadThread != nullptr)
        hotReloadThread->stopThread(1500);
    unloadPlugin();

    --numInstances;
#if JUCE_WINDOWS
    if (--numInstances <= 0)
        deleteCyderPluginsTempDirectoryAfterShutdown();
#endif
}

void CyderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (wrappedPlugin == nullptr)
        return;
    
    // Wrapped plugin can only be mono or stereo
    // If Cyder is mono-to-stereo, then wrapped plugin must be stereo
    bool isStereo = getTotalNumOutputChannels();
    bool wrappedPluginIsStereo = wrappedPlugin->getTotalNumOutputChannels();
    auto numChannels = isStereo ? 2 : 1;
    
    if (isStereo != wrappedPluginIsStereo)
    {
        wrappedPlugin->setPlayConfigDetails(numChannels, // input
                                            numChannels, // output
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

bool CyderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Get the main output bus layout
    auto mainOut = layouts.getMainOutputChannelSet();

    // Only allow mono or stereo
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

    // Require input layout to match output
    auto mainIn = layouts.getMainInputChannelSet();
    if (mainIn != mainOut)
        return false;

    return true;
}

void CyderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (wrappedPlugin == nullptr)
        return;
    
    auto playhead = getPlayHead();
    wrappedPlugin->setPlayHead(playhead);
    
    const bool wrappedPluginIsStereo = (2 == wrappedPlugin->getTotalNumOutputChannels());
    const auto numChannels = buffer.getNumChannels();
    const bool monoBufferGivenToStereoPlugin = (wrappedPluginIsStereo && numChannels==1);
    
    // Special case where stereo plugin is given mono buffer (e.g. Studio One)
    if (monoBufferGivenToStereoPlugin)
        processUsingMonoToStereoBuffer(buffer, midiMessages);
    else
        wrappedPlugin->processBlock(buffer, midiMessages);
}

void CyderAudioProcessor::processUsingMonoToStereoBuffer(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    monoToStereoBuffer.setSize(/*numChannels*/2,
                               /*numSamples*/buffer.getNumSamples(),
                               /*keepExistingContent*/false,
                               /*clearExtraSpace*/false,
                               /*avoidReallocating*/true);
    monoToStereoBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    monoToStereoBuffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    
    wrappedPlugin->processBlock(monoToStereoBuffer, midiMessages);
    
    buffer.copyFrom(0, 0, monoToStereoBuffer, 0, 0, buffer.getNumSamples());
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
    xml->setAttribute("pluginFilePath", currentPluginFileOriginal.getFullPathName());

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
    
    if (wrappedPlugin == nullptr) // something went wrong when loading wrapped plugin from saved state
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
    
    std::unique_ptr<juce::AudioPluginInstance> newInstance;
    juce::File incomingCopiedPlugin;
    
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();
    
    juce::File pluginFile(pluginPath);
    const bool reloadingSamePlugin = pluginFile == currentPluginFileOriginal;
    
    {
        [[maybe_unused]] const auto inputChannels = getTotalNumInputChannels();
        [[maybe_unused]] const auto outputChannels = getTotalNumOutputChannels();
        jassert(inputChannels == outputChannels); // only support basic mono or stereo I/O
    }
    
    // Wrapped plugin can only be mono or stereo
    // If Cyder is mono-to-stereo, then wrapped plugin must be stereo
    const bool isStereo    = getTotalNumOutputChannels() == 2;
    const auto numChannels = isStereo ? 2 : 1;
    const auto sampleRate  = getSampleRate();
    const auto blockSize   = getBlockSize();
    
    if (hotReloadThread != nullptr)
        hotReloadThread->stopThread(1500); // don't hot reload while we're loading
    
    // Load incoming plugin (without yet removing ours)
    try
    {
        // Copy plugin to temp with a random hash appended
        incomingCopiedPlugin = Utilities::copyPluginToTemp(pluginFile);

        // Only supporting VST3
        juce::AudioProcessor::setTypeOfNextNewPlugin(wrapperType_VST3);

        auto description = Utilities::findPluginDescription(incomingCopiedPlugin, formatManager);
        description.numInputChannels  = numChannels;
        description.numOutputChannels = numChannels;
        
        newInstance = Utilities::createInstance(description, formatManager, sampleRate, blockSize);
    }
    catch(const std::exception& e) // failed to load plugin
    {
        juce::Logger::writeToLog(e.what());
        currentStatus = reloadingSamePlugin ? CyderStatus::failedToReloadPlugin
                                            : CyderStatus::failedToLoadPlugin;
        CYDER_ASSERT_FALSE;
        
        if (hotReloadThread != nullptr)
            hotReloadThread->startThread(); // restart HotReloadThread
        
        return false;
    }
    
    // Make sure nothing above threw exception before swapping out current plugin with new plugin
    
    // Configure incoming plugin
    newInstance->setPlayConfigDetails(numChannels,
                                      numChannels,
                                      sampleRate,
                                      blockSize);
    newInstance->prepareToPlay(sampleRate, blockSize);
    if (reloadingSamePlugin)
        transferPluginState(*newInstance);
    
    // Unload editor
    {
        auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor());
        if (cyderEditor != nullptr)
            cyderEditor->unloadWrappedEditor(getWrappedPluginEditor(),
                                             /*shouldCacheSize*/ reloadingSamePlugin);
        wrappedPluginEditor.reset();
    }
    
    // Remove processor listener
    if (wrappedPlugin != nullptr)
        wrappedPlugin->removeListener(this);
    
    // Swap out processor
    {
        juce::ScopedLock lock(getCallbackLock()); // lock audio thread
        wrappedPlugin.reset(newInstance.release());
        setLatencySamples(wrappedPlugin->getLatencySamples());
    }
    
    // Add processor listener
    wrappedPlugin->addListener(this);
    
    // Cleanup: Delete copied plugin
    deleteStalePlugin(currentPluginFileCopy);
    currentPluginFileCopy = juce::File(); // reset

    // Update refs
    currentPluginFileOriginal = pluginFile;
    currentPluginFileCopy = incomingCopiedPlugin;
    
    // Create new editor
    {
        auto editor = wrappedPlugin->createEditor();
        CYDER_ASSERT(editor != nullptr);
        wrappedPluginEditor.reset(editor);
        
        auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor());
        if (cyderEditor != nullptr)
            cyderEditor->loadWrappedEditor(getWrappedPluginEditor());
    }
    
    // Restart HotReloadThread
    hotReloadThread = std::make_unique<HotReloadThread>(pluginFile); // auto starts thread
    hotReloadThread->onPluginChangeDetected = [&]
    {
        juce::MessageManager::callAsync([safeThis = juce::WeakReference<CyderAudioProcessor>(this)]
        {
            if (safeThis.wasObjectDeleted())
                return;
            
            safeThis->loadPlugin(safeThis->hotReloadThread->getFullPluginPath());
        });
    };
    
    // Update Status
    currentStatus = reloadingSamePlugin ? CyderStatus::successfullyReloadedPlugin
                                        : CyderStatus::successfullyLoadedPlugin;
    return true;
}

void CyderAudioProcessor::unloadPlugin()
{
    if (wrappedPlugin == nullptr)
        return;
    
    // Stop and reset hot reload thread first so we don't reload after unloading
    if (hotReloadThread != nullptr)
    {
        hotReloadThread->stopThread(1500);
        hotReloadThread.reset();
    }
    
    // Remove processor listener
    wrappedPlugin->removeListener(this);
    
    // Unload wrapped editor
    if (auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor()))
        cyderEditor->unloadWrappedEditor(getWrappedPluginEditor(),
                                         /*shouldCacheSize*/ false);
    wrappedPluginEditor.reset();
    
    // Unload wrapped processor
    {
        juce::ScopedLock lock(getCallbackLock()); // lock audio thread
        wrappedPlugin.reset();
    }
    
    // Cleanup: Delete copied plugin
    currentPluginFileOriginal = juce::File(); // reset
    deleteStalePlugin(currentPluginFileCopy);
    
    // Update latency
    setLatencySamples(0);
    
    // Update status
    currentStatus = CyderStatus::idle;
}

CyderStatus CyderAudioProcessor::getCurrentStatus() const noexcept
{
    return currentStatus;
}

CyderStatus CyderAudioProcessor::getCurrentStatusAndClear() noexcept
{
    return std::exchange(currentStatus, CyderStatus::idle);
}

juce::AudioProcessor* CyderAudioProcessor::getWrappedPluginProcessor() const noexcept
{
    return dynamic_cast<juce::AudioProcessor*>(wrappedPlugin.get());
}

juce::AudioProcessorEditor* CyderAudioProcessor::getWrappedPluginEditor() const noexcept
{
    return wrappedPluginEditor.get();
}

juce::File CyderAudioProcessor::getCurrentWrappedPluginPathCopy() const noexcept
{
    return currentPluginFileCopy;
}

juce::File CyderAudioProcessor::getCurrentWrappedPluginPathOriginal() const noexcept
{
    return currentPluginFileOriginal;
}

juce::Thread* CyderAudioProcessor::getHotReloadThread() const noexcept
{
    return hotReloadThread.get();
}

void CyderAudioProcessor::transferPluginState(juce::AudioProcessor& destinationProcessor) noexcept
{
    if (wrappedPlugin == nullptr)
        return;
    
    juce::MemoryBlock memoryBlock;
    wrappedPlugin->getStateInformation(memoryBlock);
    destinationProcessor.setStateInformation(memoryBlock.getData(),
                                             static_cast<int>(memoryBlock.getSize()));
}

void CyderAudioProcessor::audioProcessorChanged (juce::AudioProcessor* processor,
                                                 const AudioProcessorListener::ChangeDetails& details)
{
    if (processor != wrappedPlugin.get())
        return;
    
    if (details.latencyChanged)
        setLatencySamples(wrappedPlugin->getLatencySamples());
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyderAudioProcessor();
}
