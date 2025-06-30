/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     HotReloadThread.hpp
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

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

#include <functional>

//==============================================================================

/** */
class HotReloadThread final : public juce::Thread
{
public:
    HotReloadThread(const juce::File& pluginToReload);
    ~HotReloadThread();
    
    std::function<void()> onPluginChangeDetected = nullptr;
    
    juce::String getFullPluginPath() const noexcept;
    
private:
    const juce::File pluginToReload;
    juce::Time lastTimePluginWasModified;
    
    void run() override;
    
    HotReloadThread(const HotReloadThread&) = delete;
    HotReloadThread& operator=(const HotReloadThread&) = delete;
};
