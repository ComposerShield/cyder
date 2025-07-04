
#include "ExamplePluginProcessor.h"

ExamplePluginAudioProcessor::ExamplePluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Hack to allow plugin to work when loaded into Cyder in unit tests
    juce::MessageManager::getInstance()->setCurrentThreadAsMessageThread();
}

ExamplePluginAudioProcessor::~ExamplePluginAudioProcessor() = default;

void ExamplePluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
}

void ExamplePluginAudioProcessor::releaseResources()
{
    
}

bool ExamplePluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Only allow stereo in → stereo out
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ExamplePluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& /*midiMessages*/)
{
    /** Do nothing */
}

void ExamplePluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    destData.copyFrom(mockState.getData(), 0, static_cast<int>(mockState.getSize()));
    DBG("");
}

void ExamplePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    mockState.reset();
    mockState.setSize(static_cast<size_t>(sizeInBytes));
    mockState.copyFrom(data, 0, sizeInBytes);
    DBG("");
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExamplePluginAudioProcessor();
}
