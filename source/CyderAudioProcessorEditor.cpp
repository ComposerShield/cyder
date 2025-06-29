
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
    unloadWrappedEditor();
}

void CyderAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
    g.setColour(juce::Colours::white);
    g.drawText("Drag VST3 plugin here.", getLocalBounds(), juce::Justification::centred);
    
    if (fileDraggingOverEditor)
    {
        g.setColour(juce::Colours::pink.withAlpha(0.5f));
        g.fillAll();
    }
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
    unloadWrappedEditor();
    
    fileDraggingOverEditor = false;
    
    const auto& pluginString = files[0];
    auto result = processor.loadPlugin(pluginString);
    jassert(result);
    
    loadWrappedEditorFromProcessor();
}

void CyderAudioProcessorEditor::fileDragEnter(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    fileDraggingOverEditor = true;
    repaint();
}

void CyderAudioProcessorEditor::fileDragExit(const juce::StringArray& files)
{
    fileDraggingOverEditor = false;
    repaint();
}

void CyderAudioProcessorEditor::unloadWrappedEditor(bool shouldCacheSize)
{
    auto* editor = processor.getWrappedPluginEditor();
    if (editor != nullptr)
    {
        if (shouldCacheSize)
        {
            cachedWidth = editor->getWidth();
            cachedHeight = editor->getHeight();
        }
        
        editor->removeComponentListener(this);
        removeChildComponent(editor);
    }
}

void CyderAudioProcessorEditor::loadWrappedEditorFromProcessor()
{
    auto* editor = processor.getWrappedPluginEditor();
    if (editor != nullptr)
    {
        addAndMakeVisible(editor);
        
        if (cachedWidth.has_value() && cachedHeight.has_value())
        {
            editor->setSize(std::exchange(cachedWidth,  std::nullopt).value(),
                            std::exchange(cachedHeight, std::nullopt).value());
        }
        else
            setSize(editor->getWidth(), editor->getHeight());
        
        editor->setTopLeftPosition(0, 0);
        editor->addComponentListener(this);
    }
}

void CyderAudioProcessorEditor::componentMovedOrResized(juce::Component& /*component*/,
                                                        bool /*wasMoved*/,
                                                        bool wasResized)
{
    if (wasResized)
    {
        auto* editor = processor.getWrappedPluginEditor();
        setSize(editor->getWidth(), editor->getHeight());
    }
}
