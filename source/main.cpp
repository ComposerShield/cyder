#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

/**
 * @brief   Entry point of the program.
 * @param   argc  Number of command‐line arguments.
 * @param   argv  Array of C‐strings holding the arguments.
 * @return  Exit status (0 on success, non-zero on error).
 */
int main(int argc, char* argv[])
{
    // Initialize JUCE GUI
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;

    // Parse the command-line argument for plugin path
    juce::StringArray args;
    for (int i = 0; i < argc; ++i)
        args.add (argv[i]);
    if (args.size() < 2)
    {
        juce::Logger::writeToLog ("Usage: " + args[0] + " <path-to-plugin>");
        return 1;
    }
    const juce::File pluginFile (args[1]);
    const auto filePathString = pluginFile.getFullPathName();

    // Set up plugin formats
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();
    juce::KnownPluginList pluginList;

    // Scan the specified file for a plugin
    juce::OwnedArray<juce::PluginDescription> descriptions;
    juce::StringArray files;
    files.add (filePathString);
    pluginList.scanAndAddDragAndDroppedFiles (formatManager, files, descriptions);
    if (descriptions.isEmpty())
    {
        juce::Logger::writeToLog ("Plugin not found: " + filePathString);
        return 1;
    }
    auto description = *descriptions[0];

    // Create the plugin instance
    double sampleRate = 44100.0;
    int blockSize = 512;
    juce::String errorMessage;
    std::unique_ptr<juce::AudioPluginInstance> instance (
        formatManager.createPluginInstance (description, sampleRate, blockSize, errorMessage));
    if (! instance)
    {
        juce::Logger::writeToLog ("Error creating plugin instance: " + errorMessage);
        return 1;
    }

    // Display the plugin's editor in a window
    if (auto* editor = instance->createEditor())
    {
        int w = editor->getWidth();
        int h = editor->getHeight();
        juce::Logger::writeToLog ("Editor initial size: " + juce::String (w) + "x" + juce::String (h));
        if (w <= 0 || h <= 0)
        {
            w = 400; h = 300;
            editor->setSize (w, h);
        }
        auto* window = new juce::DocumentWindow (description.name,
                                                 juce::Colours::lightgrey,
                                                 juce::DocumentWindow::allButtons,
                                                 true);
        window->setUsingNativeTitleBar (true);
        window->setContentOwned (editor, true);
        window->centreWithSize (w, h);
        window->setVisible (true);
        window->setTopLeftPosition(0, 0);
        window->toFront(true);

        // Enter the GUI event loop until the window is closed
        juce::MessageManager::getInstance()->runDispatchLoop();
    }
    else
    {
        juce::Logger::writeToLog ("Plugin has no editor");
    }

    return 0;
}
