/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderHeaderBar.hpp
 *
 * @author   Adam Shield
 * @date     2025-06-30
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

#include <memory>

//==============================================================================

class CyderAudioProcessor;

//==============================================================================

/** */
class CyderHeaderBar final
: public juce::Component
, private juce::Button::Listener
{
public:
    CyderHeaderBar(CyderAudioProcessor& processor);
    ~CyderHeaderBar();
    
private:
    CyderAudioProcessor& processor;
    
    juce::TextButton unloadPluginButton;
    std::unique_ptr<juce::LookAndFeel> lookAndFeel;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void buttonClicked(juce::Button*) override;
    
    CyderHeaderBar(const CyderHeaderBar&) = delete;
    CyderHeaderBar& operator=(const CyderHeaderBar&) = delete;
};
