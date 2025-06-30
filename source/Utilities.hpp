/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     Utilites.hpp
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

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

#include <memory>

//==============================================================================

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
    
    /**
     * @brief Parses command-line arguments and returns the plugin file path.
     * @param argc Number of command-line arguments.
     * @param argv Array of C-strings holding the arguments.
     * @return juce::File representing the plugin file.
     * @throws std::runtime_error if usage is incorrect or the plugin path is missing.
     */
    [[nodiscard]] static juce::File parsePluginFilePath(int argc, char* argv[]) noexcept(false);

    /**
     * @brief Scans the specified file for a plugin description.
     * @param pluginFile The JUCE File pointing to the plugin binary.
     * @param formatManager The AudioPluginFormatManager used to discover plugin formats.
     * @return juce::PluginDescription for the first plugin found.
     * @throws std::runtime_error if no plugin description is found.
     */
    [[nodiscard]] static juce::PluginDescription findPluginDescription(const juce::File& pluginFile,
                                                                       juce::AudioPluginFormatManager& formatManager) noexcept(false);

    /**
     * @brief Creates a plugin instance from its description.
     * @param desc The PluginDescription of the plugin to instantiate.
     * @param formatManager The format manager used to create the plugin instance.
     * @param sampleRate The sample rate for initializing the plugin.
     * @param blockSize The audio block size for the plugin.
     * @return std::unique_ptr<juce::AudioPluginInstance> owning the new instance.
     * @throws std::runtime_error if the plugin instance could not be created.
     */
    [[nodiscard]] static std::unique_ptr<juce::AudioPluginInstance> createInstance(const juce::PluginDescription& description,
                                                                                   juce::AudioPluginFormatManager& formatManager,
                                                                                   double sampleRate,
                                                                                   int blockSize) noexcept(false);

    /**
     * @brief Displays the plugin's editor in a DocumentWindow and starts the GUI event loop.
     * @param instance Pointer to the AudioPluginInstance whose editor to show.
     * @param windowTitle The title to use for the window.
     * @throws std::runtime_error if the plugin has no editor.
     */
    static std::unique_ptr<juce::DocumentWindow> createAndShowEditorWindow(juce::AudioPluginInstance* instance,
                                                                           const juce::String& windowTitle) noexcept(false);
    
private:
    Utilities() = delete;
};
