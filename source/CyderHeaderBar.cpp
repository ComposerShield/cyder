/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderHeaderBar.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-30
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "CyderHeaderBar.hpp"

#include "CyderAudioProcessor.hpp"

#include <memory>

//==============================================================================

static constexpr int margin = 5;

class CyderHeaderBarLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CyderHeaderBarLookAndFeel()
    {
        setColour(juce::TextButton::ColourIds::buttonColourId,   juce::Colours::black.withAlpha(0.5f));
        setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::pink);
        setColour(juce::TextButton::ColourIds::textColourOffId,  juce::Colours::white);
        setColour(juce::TextButton::ColourIds::textColourOnId,   juce::Colours::white);
    }
};

const char* getStatusAsString(CyderStatus status) noexcept
{
    switch(status)
    {
        case CyderStatus::idle                       : return "";
        case CyderStatus::loading                    : return "";
        case CyderStatus::reloading                  : return "";
        case CyderStatus::successfullyLoadedPlugin   : return "Successfully loaded plugin";
        case CyderStatus::successfullyReloadedPlugin : return "Successfully reloaded plugin";
        case CyderStatus::failedToLoadPlugin         : return "Failed to load plugin...";
        case CyderStatus::failedToReloadPlugin       : return "Failed to reload plugin...";
    };
}

//==============================================================================

CyderHeaderBar::CyderHeaderBar(CyderAudioProcessor& _processor)
: processor(_processor)
{
    lookAndFeel = std::make_unique<CyderHeaderBarLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    unloadPluginButton.setButtonText("Unload Plugin");
    addAndMakeVisible(unloadPluginButton);
    unloadPluginButton.addListener(this);
    
    startTimer(/*ms*/500);
}

CyderHeaderBar::~CyderHeaderBar()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void CyderHeaderBar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
    
    g.setColour(juce::Colours::white);
    auto statusString = juce::String(getStatusAsString(currentStatus));
    g.drawText(statusString,
               getLocalBounds().withTrimmedRight(margin),
               juce::Justification(juce::Justification::centredRight));
}

void CyderHeaderBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(margin);
    unloadPluginButton.setBounds(bounds.removeFromLeft(100));
}

void CyderHeaderBar::buttonClicked(juce::Button* button)
{
    if (button == &unloadPluginButton)
    {
        processor.unloadPlugin();
    }
}

void CyderHeaderBar::timerCallback()
{
    auto status = processor.getCurrentStatus();
    if (currentStatus != status)
    {
        currentStatus = status;
        repaint();
    }
}
