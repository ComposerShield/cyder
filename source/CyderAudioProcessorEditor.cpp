
#include "CyderAudioProcessorEditor.hpp"

#include "CyderAudioProcessor.hpp"

//==============================================================================

CyderAudioProcessorEditor::CyderAudioProcessorEditor(CyderAudioProcessor& p)
: juce::AudioProcessorEditor(p)
, processor (p)
{
    setSize (400, 300);
}

CyderAudioProcessorEditor::~CyderAudioProcessorEditor()
{
}

void CyderAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
    g.setColour(juce::Colours::white);
    g.drawText("Drag VST3 plugin here.", getLocalBounds(), juce::Justification::centred);
}

void CyderAudioProcessorEditor::resized()
{

}

bool CyderAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    return (files.size() == 1 && files[0].endsWith("vst3")); // only vst3 supported for now
}

void CyderAudioProcessorEditor::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    const auto& pluginString = files[0];
    auto result = processor.loadPlugin(pluginString);
    jassert(result);
}

