/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderAudioProcessorEditor.hpp
 *
 * @author   Adam Shield
 * @date     2025-06-29
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

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
    explicit CyderAudioProcessorEditor(CyderAudioProcessor&);
    ~CyderAudioProcessorEditor() override;

    void resized() override;
    
    /** */
    void unloadWrappedEditor(juce::AudioProcessorEditor* editor, bool shouldCacheSize = false);
    /** */
    void loadWrappedEditor(juce::AudioProcessorEditor* editor);
    
    /** */
    [[nodiscard]] bool isFileDraggingOverEditor() const noexcept;
    
    /** */
    CyderHeaderBar& getHeaderBar() noexcept;
    /** */
    const CyderHeaderBar& getHeaderBar() const noexcept;

private:
    CyderAudioProcessor& processor;
    
    std::unique_ptr<CyderHeaderBar> headerBar;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    std::optional<int> cachedWidth;
    std::optional<int> cachedHeight;
    
    bool fileDraggingOverEditor = false;
    
    void paint(juce::Graphics& g) override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    
    void componentMovedOrResized(juce::Component& component,
                                 bool wasMoved,
                                 bool wasResized) override;
    
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    JUCE_DECLARE_WEAK_REFERENCEABLE (CyderAudioProcessorEditor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyderAudioProcessorEditor)
};
