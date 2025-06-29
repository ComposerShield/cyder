#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <stdexcept>

/**
 * @brief Parses command-line arguments and returns the plugin file path.
 * @param argc Number of command-line arguments.
 * @param argv Array of C-strings holding the arguments.
 * @return juce::File representing the plugin file.
 * @throws std::runtime_error if usage is incorrect or the plugin path is missing.
 */
[[nodiscard]] static juce::File parsePluginFilePath(int argc, char* argv[]) noexcept(false)
{
    juce::StringArray args;
    for (int i = 0; i < argc; ++i)
        args.add (argv[i]);
    if (args.size() < 2)
        throw std::runtime_error(("Usage: " + args[0] + " <path-to-plugin>").toStdString());
    return juce::File (args[1]);
}

/**
 * @brief Scans the specified file for a plugin description.
 * @param pluginFile The JUCE File pointing to the plugin binary.
 * @param formatManager The AudioPluginFormatManager used to discover plugin formats.
 * @return juce::PluginDescription for the first plugin found.
 * @throws std::runtime_error if no plugin description is found.
 */
[[nodiscard]] static juce::PluginDescription findPluginDescription(const juce::File& pluginFile,
                                                                   juce::AudioPluginFormatManager& formatManager) noexcept(false)
{
    juce::KnownPluginList pluginList;
    juce::OwnedArray<juce::PluginDescription> descriptions;
    juce::StringArray files;
    files.add (pluginFile.getFullPathName());
    pluginList.scanAndAddDragAndDroppedFiles(formatManager, files, descriptions);
    if (descriptions.isEmpty())
        throw std::runtime_error(("Plugin not found: " + pluginFile.getFullPathName()).toStdString());
    return *descriptions[0];
}

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
                                                                               int blockSize) noexcept(false)
{
    juce::String errorMessage;
    auto instance = formatManager.createPluginInstance(description, sampleRate, blockSize, errorMessage);
    if (! instance)
        throw std::runtime_error(("Error creating plugin instance: " + errorMessage).toStdString());
    return instance;
}

/**
 * @brief Displays the plugin's editor in a DocumentWindow and starts the GUI event loop.
 * @param instance Pointer to the AudioPluginInstance whose editor to show.
 * @param windowTitle The title to use for the window.
 * @throws std::runtime_error if the plugin has no editor.
 */
static void showEditorWindow(juce::AudioPluginInstance* instance,
                             const juce::String& windowTitle) noexcept(false)
{
    auto* editor = instance->createEditor();
    if (editor == nullptr)
        throw std::runtime_error("Plugin has no editor");
    int w = editor->getWidth();
    int h = editor->getHeight();
    if (w <= 0 || h <= 0)
        editor->setSize(w = 400, h = 300);
    auto* window = new juce::DocumentWindow(windowTitle,
                                            juce::Colours::lightgrey,
                                            juce::DocumentWindow::allButtons,
                                            true);
    window->setUsingNativeTitleBar(true);
    window->setContentOwned(editor, true);
    window->centreWithSize(w, h);
    window->setVisible(true);
    window->setTopLeftPosition(0, 0);
    juce::MessageManager::getInstance()->runDispatchLoop();
}

/**
 * @brief   Entry point of the program.
 * @param   argc  Number of command‐line arguments.
 * @param   argv  Array of C‐strings holding the arguments.
 * @return  Exit status (0 on success, non-zero on error).
 */
int main(int argc, char* argv[])
{
    try
    {
        juce::ScopedJuceInitialiser_GUI libraryInitialiser;
        auto pluginFile = parsePluginFilePath(argc, argv);

        juce::AudioPluginFormatManager formatManager;
        formatManager.addDefaultFormats();

        auto description = findPluginDescription(pluginFile, formatManager);
        auto instance    = createInstance(description, formatManager, 44100.0, 512);

        showEditorWindow(instance.get(), description.name);
    }
    catch(const std::exception& e)
    {
        juce::Logger::writeToLog(e.what());
        return 1;
    }
    return 0;
}
