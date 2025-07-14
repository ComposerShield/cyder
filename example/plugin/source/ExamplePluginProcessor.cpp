
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
