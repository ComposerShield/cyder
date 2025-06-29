
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
    bool reloadPending = false;
    juce::Time pendingDetectedTime;
    constexpr int debounceMs       = 500; // delay after last change before reloading
    constexpr int pollIntervalMs   = 200; // how often to poll for file changes

    while (true)
    {
        if (threadShouldExit())
            return;

        // Scan for the newest modification time in the bundle
        juce::Time current = getLatestModificationTime(pluginToReload);

        // If we see a new modification, start (or restart) the debounce timer
        if (current > lastTimePluginWasModified)
        {
            lastTimePluginWasModified = current;
            pendingDetectedTime       = juce::Time::getCurrentTime();
            reloadPending             = true;
            DBG("Plugin change detected! Waiting in case other changes are still being made...");
        }

        // If a change was detected and the debounce interval has passed, fire callback once
        if (reloadPending &&
            juce::Time::getCurrentTime() > (pendingDetectedTime + juce::RelativeTime::milliseconds(debounceMs)))
        {
            DBG("Triggering plugin reload.");
            if (auto callback = std::exchange(onPluginChangeDetected, nullptr))
                return callback();
            reloadPending = false;
        }

        // Wait before polling again
        juce::Thread::wait(pollIntervalMs);
    }
}
