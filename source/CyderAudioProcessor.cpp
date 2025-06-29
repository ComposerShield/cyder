#include "CyderAudioProcessor.hpp"

#include "CyderAudioProcessorEditor.hpp"
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
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
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
    
    return true;
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyderAudioProcessor();
}
