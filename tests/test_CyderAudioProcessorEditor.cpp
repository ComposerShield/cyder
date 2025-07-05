
#include <gtest/gtest.h>

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../source/CyderAudioProcessor.hpp"
#include "../source/CyderAudioProcessorEditor.hpp"

#include <gtest/gtest.h>

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
