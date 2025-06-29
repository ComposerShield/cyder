
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>


class HotReloadThread final : public juce::Thread
{
public:
    HotReloadThread(const juce::File& pluginToReload);
    ~HotReloadThread();
    
private:
    const juce::File& pluginToReload;
    juce::Time lastTimePluginWasModified;
    
    void run() override;
    
    void binaryModified();
    
    HotReloadThread(const HotReloadThread&) = delete;
    HotReloadThread& operator=(const HotReloadThread&) = delete;
};
