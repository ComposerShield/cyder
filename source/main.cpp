
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include "HotReloadThread.hpp"
#include "Utilities.hpp"

#include <stdexcept>

//==============================================================================

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
        auto pluginFile = Utilities::parsePluginFilePath(argc, argv);
        // Copy plugin to temp with a random hash appended
        auto tempPluginFile = Utilities::copyPluginToTempWithHash(pluginFile);

        juce::AudioPluginFormatManager formatManager;
        formatManager.addDefaultFormats();

        auto description = Utilities::findPluginDescription(tempPluginFile, formatManager);
        auto instance    = Utilities::createInstance(description, formatManager, 44100.0, 512);

        auto window = Utilities::createAndShowEditorWindow(instance.get(), description.name);
        
        HotReloadThread hotReloadThread(pluginFile);
        hotReloadThread.onPluginChangeDetected = [&]
        {
            juce::MessageManager::callSync([&]
            {
                try
                {
                    // Copy plugin to temp with a random hash appended
                    auto tempPluginFile = Utilities::copyPluginToTempWithHash(pluginFile);
                    auto description    = Utilities::findPluginDescription(tempPluginFile, formatManager);
                    auto reloadInstance = Utilities::createInstance(description, formatManager, 44100.0, 512);
                    auto* editor        = reloadInstance->createEditor();
                    
                    window->setContentOwned(editor, true);
                    
                    instance.reset();                     // delete original instance
                    instance = std::move(reloadInstance); // replace with new instance
                }
                catch(const std::exception& e)
                {
                    juce::Logger::writeToLog(e.what());
                    // Failed to reload plugin...
                }
            });
        };
        
        // Run indefinitely
        juce::MessageManager::getInstance()->runDispatchLoop();
    }
    catch(const std::exception& e)
    {
        juce::Logger::writeToLog(e.what());
        return 1;
    }
    return 0;
}
