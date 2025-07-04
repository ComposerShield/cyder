
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ExamplePluginProcessor.h"

/**
    A simple custom editor for ExamplePlugin.
*/
class ExamplePluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit ExamplePluginAudioProcessorEditor (ExamplePluginAudioProcessor&);
    ~ExamplePluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ExamplePluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExamplePluginAudioProcessorEditor)
};
