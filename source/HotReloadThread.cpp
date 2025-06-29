
#include "HotReloadThread.hpp"

/** Helper to get the latest modification time of a file or any of its children. */
[[nodiscard]] static juce::Time getLatestModificationTime(const juce::File& file) noexcept
{
    juce::Time latest = file.getLastModificationTime();
    juce::Array<juce::File> children;
    file.findChildFiles (children, juce::File::findFiles, true);
    for (auto& f : children)
        if (f.getLastModificationTime() > latest)
            latest = f.getLastModificationTime();
    return latest;
}

HotReloadThread::HotReloadThread(const juce::File& _pluginToReload)
: juce::Thread("Hot Reload Thread")
, pluginToReload(_pluginToReload)
, lastTimePluginWasModified(getLatestModificationTime(pluginToReload))
{
    startThread();
}

HotReloadThread::~HotReloadThread() = default;

void HotReloadThread::run()
{
    while (true)
    {
        // Check the deepest child files for changes
        juce::Time current = getLatestModificationTime (pluginToReload);
        if (current > lastTimePluginWasModified)
        {
            lastTimePluginWasModified = current;
            DBG ("Plugin Modified!");
        }
        
        juce::Thread::wait(500); // ms
    }
}
