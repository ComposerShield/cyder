#include "CyderAudioProcessor.hpp"

#include "CyderAudioProcessorEditor.hpp"
#include "HotReloadThread.hpp"
#include "Utilities.hpp"

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
}

void CyderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void CyderAudioProcessor::releaseResources()
{
}

bool CyderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void CyderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    std::scoped_lock<std::mutex> lock(wrappedPluginMutex);
//    if (wrappedPluginMutex != nullptr)
//        wrappedPluginMutex->processBlock(buffer);
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
    return false;
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
    return 0.0;
}

int CyderAudioProcessor::getNumPrograms()
{
    return 1;
}

int CyderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CyderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CyderAudioProcessor::getProgramName (int index)
{
    return {};
}

void CyderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void CyderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void CyderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

bool CyderAudioProcessor::loadPlugin(const juce::String& pluginPath)
{
    try
    {
        juce::ScopedJuceInitialiser_GUI libraryInitialiser;
        juce::File pluginFile(pluginPath);
        const bool reloadingSamePlugin = pluginFile == currentPluginFile;
        
        // Copy plugin to temp with a random hash appended
        auto tempPluginFile = Utilities::copyPluginToTempWithHash(pluginFile);
        
        const auto sampleRate = getSampleRate();
        const auto blockSize  = getBlockSize();

        // Only supporting VST3
        juce::AudioProcessor::setTypeOfNextNewPlugin(wrapperType_VST3);
        
        auto description  = Utilities::findPluginDescription(tempPluginFile, formatManager);
        auto instance     = Utilities::createInstance(description, formatManager, sampleRate, blockSize);
        auto editor       = instance->createEditor();
        auto* cyderEditor = dynamic_cast<CyderAudioProcessorEditor*>(getActiveEditor());

        // Make sure nothing above threw exception before swapping out current members
        
        currentPluginFile = pluginFile;
        if (reloadingSamePlugin)
            transferPluginState(*instance);
        
        if (hotReloadThread != nullptr)
            hotReloadThread->stopThread(1000); // don't hot reload while we're loading
        
        cyderEditor->unloadWrappedEditor(/*shouldCacheSize*/ reloadingSamePlugin);
        wrappedPluginEditor.reset();
        
        {
            std::scoped_lock<std::mutex> lock(wrappedPluginMutex);
            wrappedPlugin.reset(instance.release());
        }
        wrappedPluginEditor.reset(std::move(editor));
        
        cyderEditor->loadWrappedEditorFromProcessor();
        
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
    
    return true;
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
