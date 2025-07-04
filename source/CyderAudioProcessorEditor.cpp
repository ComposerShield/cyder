/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderAudioProcessorEditor.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-29
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "CyderAudioProcessorEditor.hpp"

#include "CyderAudioProcessor.hpp"
#include "CyderHeaderBar.hpp"

#include <memory>

static constexpr int headerBarHeight = 22;

//==============================================================================

CyderAudioProcessorEditor::CyderAudioProcessorEditor(CyderAudioProcessor& p)
: juce::AudioProcessorEditor(p)
, processor(p)
{
    headerBar = std::make_unique<CyderHeaderBar>(processor);
    addAndMakeVisible(headerBar.get());
    
    setSize(400, 300);
    
    loadWrappedEditor(processor.getWrappedPluginEditor());
}

CyderAudioProcessorEditor::~CyderAudioProcessorEditor()
{
    unloadWrappedEditor(processor.getWrappedPluginEditor());
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
    unloadWrappedEditor(processor.getWrappedPluginEditor());
    
    fileDraggingOverEditor = false;
    
    const auto& pluginString = files[0];
    [[maybe_unused]] auto result = processor.loadPlugin(pluginString);
    jassert(result);
    
    loadWrappedEditor(processor.getWrappedPluginEditor());
}

void CyderAudioProcessorEditor::fileDragEnter(const juce::StringArray& /*files*/, int /*x*/, int /*y*/)
{
    fileDraggingOverEditor = true;
    repaint();
}

void CyderAudioProcessorEditor::fileDragExit(const juce::StringArray& /*files*/)
{
    fileDraggingOverEditor = false;
    repaint();
}

void CyderAudioProcessorEditor::unloadWrappedEditor(juce::AudioProcessorEditor* editor, bool shouldCacheSize)
{
    if (editor != nullptr)
    {
        if (shouldCacheSize)
        {
            cachedWidth = editor->getWidth();
            cachedHeight = editor->getHeight();
        }
        
        editor->removeComponentListener(this);
        removeChildComponent(editor);
        removeChildComponent(headerBar.get());
    }
}

void CyderAudioProcessorEditor::loadWrappedEditor(juce::AudioProcessorEditor* editor)
{
    if (editor != nullptr)
    {
        addAndMakeVisible(editor);
        
        if (cachedWidth.has_value() && cachedHeight.has_value())
        {
            const auto width  = std::exchange(cachedWidth,  std::nullopt).value();
            const auto height = std::exchange(cachedHeight,  std::nullopt).value();
            editor->setSize(width, height);
        }
        else
            setSize(editor->getWidth(),
                    editor->getHeight() + headerBarHeight);
        
        addChildComponent(headerBar.get());
        headerBar->setBounds(0, 0, getWidth(), headerBarHeight);
        
        editor->setTopLeftPosition(0, headerBarHeight);
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
        setSize(editor->getWidth(),
                editor->getHeight() + headerBarHeight);
        headerBar->setBounds(0, 0, getWidth(), headerBarHeight);
    }
}

bool CyderAudioProcessorEditor::isFileDraggingOverEditor() const noexcept
{
    return fileDraggingOverEditor;
}
