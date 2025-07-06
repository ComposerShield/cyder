
#include <gtest/gtest.h>

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../source/CyderAudioProcessor.hpp"
#include "../source/CyderAudioProcessorEditor.hpp"

#include <gtest/gtest.h>

//==============================================================================

namespace
{
class MockProcessor : public juce::AudioProcessor
{
public:
    // Pure virtual functions we must override
    const juce::String getName() const override { return {}; }
    void prepareToPlay (double sampleRate,
                        int maximumExpectedSamplesPerBlock) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>& buffer,
                       juce::MidiBuffer& midiMessages) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}
};

class MockEditor : public juce::AudioProcessorEditor
{
public:
    MockEditor(juce::AudioProcessor& processor, int width, int height) 
    : AudioProcessorEditor(processor)
    {
        setSize(width, height);
    }
};
} // namespace

//==============================================================================

TEST(CyderAudioProcessorEditorIsInterestedInFileDrag, AcceptsVST3)
{
    CyderAudioProcessor cyderProcessor;
    std::unique_ptr<CyderAudioProcessorEditor> editor;
    editor.reset(dynamic_cast<CyderAudioProcessorEditor*>(cyderProcessor.createEditor()));
    ASSERT_TRUE(editor != nullptr);
    
    auto* dragAndDropTarget = dynamic_cast<juce::FileDragAndDropTarget*>(editor.get());
    
    juce::File currentFile(__FILE__);
    // ExamplePlugin.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getSiblingFile("../ExamplePlugin.vst3");
    
    juce::StringArray files;
    files.add(pluginFile.getFullPathName());
    
    // Is interested in a VST3
    ASSERT_TRUE(dragAndDropTarget->isInterestedInFileDrag(files));
}

TEST(CyderAudioProcessorEditorIsInterestedInFileDrag, RejectsNonVST3)
{
    CyderAudioProcessor cyderProcessor;
    std::unique_ptr<CyderAudioProcessorEditor> editor;
    editor.reset(dynamic_cast<CyderAudioProcessorEditor*>(cyderProcessor.createEditor()));
    ASSERT_TRUE(editor != nullptr);
    
    auto* dragAndDropTarget = dynamic_cast<juce::FileDragAndDropTarget*>(editor.get());
    
    juce::File currentFile(__FILE__);
    // ExamplePlugin.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getSiblingFile("../mockFile.txt");
    
    juce::StringArray files;
    files.add(pluginFile.getFullPathName());
    
    // Is interested in a VST3
    ASSERT_FALSE(dragAndDropTarget->isInterestedInFileDrag(files));
}

TEST(CyderAudioProcessorEditorLoadWrappedEditor, CacheEditorSizeWhenReloadingPlugin)
{
    CyderAudioProcessor cyderProcessor;
    std::unique_ptr<CyderAudioProcessorEditor> editor;
    editor.reset(dynamic_cast<CyderAudioProcessorEditor*>(cyderProcessor.createEditor()));
    ASSERT_TRUE(editor != nullptr);
    
    // Original editor for plugin
    MockProcessor mockProcessor0;
    auto wrappedEditor0 = std::make_unique<MockEditor>(mockProcessor0,
                                                       /*width*/1080,
                                                       /*height*/720);
    
    MockProcessor mockProcessor1;
    // Mocked new editor for plugin after changes were made to the binary
    auto wrappedEditor1 = std::make_unique<MockEditor>(mockProcessor1,
                                                       /*width*/1111,
                                                       /*height*/222);
    
    // Load first editor
    editor->loadWrappedEditor(wrappedEditor0.get());
    
    // Unload first editor but cache its size
    editor->unloadWrappedEditor(wrappedEditor0.get(), /*shouldCacheSize*/true);
    
    // Load second editor which has a different size by default (but should succumb to our size)
    editor->loadWrappedEditor(wrappedEditor1.get());
    
    // Second editor should have been resized to the cached size of the first editor
    ASSERT_EQ(wrappedEditor1->getWidth(), wrappedEditor0->getWidth());
    ASSERT_EQ(wrappedEditor1->getHeight(), wrappedEditor0->getHeight());
    
    // Cleanup, creates issues on PC if none performed in the correct order
    editor->unloadWrappedEditor(wrappedEditor1.get());
    mockProcessor1.editorBeingDeleted(wrappedEditor1.get());
    wrappedEditor1.reset();
    mockProcessor0.editorBeingDeleted(wrappedEditor0.get());
    wrappedEditor0.reset();
    cyderProcessor.editorBeingDeleted(editor.get());
    editor.reset();
}
