
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
