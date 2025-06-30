
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================

class CyderAudioProcessor;
class CyderHeaderBar;

//==============================================================================

class CyderAudioProcessorEditor
: public juce::AudioProcessorEditor
, public juce::FileDragAndDropTarget
, private juce::ComponentListener
{
public:
    CyderAudioProcessorEditor (CyderAudioProcessor&);
    ~CyderAudioProcessorEditor() override;

    void resized() override;
    
    void unloadWrappedEditor(bool shouldCacheSize = false);
    void loadWrappedEditorFromProcessor();

private:
    CyderAudioProcessor& processor;
    
    std::unique_ptr<CyderHeaderBar> headerBar;
    
    std::optional<int> cachedWidth;
    std::optional<int> cachedHeight;
    
    bool fileDraggingOverEditor = false;
    
    void paint (juce::Graphics& g) override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;
    
    void componentMovedOrResized(juce::Component& component,
                                 bool wasMoved,
                                 bool wasResized) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyderAudioProcessorEditor)
};
