
#include "ExamplePluginEditor.h"

ExamplePluginAudioProcessorEditor::ExamplePluginAudioProcessorEditor (ExamplePluginAudioProcessor& p)
: AudioProcessorEditor (&p)
, processorRef (p)
{
    setSize (400, 300);
}

ExamplePluginAudioProcessorEditor::~ExamplePluginAudioProcessorEditor() = default;

void ExamplePluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawFittedText ("Example Plugin", getLocalBounds(), juce::Justification::centredTop, 1);
}

void ExamplePluginAudioProcessorEditor::resized()
{

}
