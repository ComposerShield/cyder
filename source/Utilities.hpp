
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>


class Utilities final
{
public:
    /**
     * @brief Copies the plugin file to a temporary directory, appending a random UUID to its name.
     * @param originalFile The original plugin File.
     * @return juce::File pointing to the newly copied plugin.
     * @throws std::runtime_error if the copy operation fails.
     */
    [[nodiscard]] static juce::File copyPluginToTempWithHash(const juce::File& originalFile) noexcept(false);
    
private:
    Utilities() = delete;
};
