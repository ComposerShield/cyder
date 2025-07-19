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
enum class CyderStatus;

//==============================================================================

/** */
class CyderHeaderBar final
: public  juce::Component
, private juce::Button::Listener
, private juce::Timer
{
public:
    CyderHeaderBar(CyderAudioProcessor& processor);
    ~CyderHeaderBar();
    
    /* @returns our user-facing string messages for a given status. */
    const char* getStatusAsString(CyderStatus status) const noexcept;
    
    /** @returns current status that has been detected by the CyderHeaderBar and interpreted as a String */
    juce::String getCurrentStatusString() const noexcept;
    
    /**
     Begin timer callbacks to query current status and clear it.
     Automatically called upon construction.
     
     @see stopReportingStatus()
     */
    void startReportingStatus() noexcept;
    
    /**
     Stop timer callbacks querying and clearing status.
     
     @see startReportingStatus()
     */
    void stopReportingStatus() noexcept;
    
private:
    CyderAudioProcessor& processor;
    CyderStatus currentStatus;
    juce::String currentStatusString;
    
    static constexpr int lengthOfTimeToDisplayStatusMs = 2000;
    int timeSinceStatusReportedMs = 0;
    
    juce::TextButton unloadPluginButton;
    std::unique_ptr<juce::LookAndFeel> lookAndFeel;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void buttonClicked(juce::Button*) override;
    
    void timerCallback() override;
    
    CyderHeaderBar(const CyderHeaderBar&) = delete;
    CyderHeaderBar& operator=(const CyderHeaderBar&) = delete;
};
