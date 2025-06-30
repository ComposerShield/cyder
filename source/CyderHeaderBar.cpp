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

//==============================================================================

CyderHeaderBar::CyderHeaderBar(CyderAudioProcessor& _processor)
: processor(_processor)
{
    lookAndFeel = std::make_unique<CyderHeaderBarLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    unloadPluginButton.setButtonText("Unload Plugin");
    addAndMakeVisible(unloadPluginButton);
    unloadPluginButton.addListener(this);
}

CyderHeaderBar::~CyderHeaderBar()
{
    setLookAndFeel(nullptr);
}

void CyderHeaderBar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
}

void CyderHeaderBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(5);
    unloadPluginButton.setBounds(bounds.removeFromLeft(100));
}

void CyderHeaderBar::buttonClicked(juce::Button* button)
{
    if (button == &unloadPluginButton)
    {
        processor.unloadPlugin();
    }
}
