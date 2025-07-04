
#include "ExamplePluginProcessor.h"

ExamplePluginAudioProcessor::ExamplePluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
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
    
}

void ExamplePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExamplePluginAudioProcessor();
}
