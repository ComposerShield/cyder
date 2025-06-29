
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
    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void CyderAudioProcessorEditor::resized()
{

}


