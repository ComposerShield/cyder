
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
}

void ExamplePluginAudioProcessorEditor::resized()
{

}
