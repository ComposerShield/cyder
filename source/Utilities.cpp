/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     Utilites.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-29
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "Utilities.hpp"

#include "CyderAssert.hpp"

#include <memory>

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN // speed up compilation, prevent namespace pollution
#include <Windows.h> // for GetLastError()
#endif

//==============================================================================

juce::File Utilities::copyPluginToTemp(const juce::File& originalFile) noexcept(false)
{
    // Create or reuse a temp subdirectory for copied plugins
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("CyderPlugins");
    {
        auto result = tempDir.createDirectory();
        CYDER_ASSERT(result);
    }

    // Generate a random UUID and build new filename
    auto uuid      = juce::Uuid().toString();
    auto baseName  = originalFile.getFileNameWithoutExtension();
    auto ext       = originalFile.getFileExtension(); // includes leading dot
    auto destFile  = tempDir.getChildFile (baseName + "_" + uuid + ext);

    {
        auto result = destFile.createDirectory();
        jassert(result);
    }

    CYDER_ASSERT(originalFile.exists());
    CYDER_ASSERT(originalFile.hasReadAccess());
    CYDER_ASSERT(destFile.hasWriteAccess());

    // Copy and verify
    if (! originalFile.copyDirectoryTo(destFile))
    {
        #if JUCE_WINDOWS
        auto err = GetLastError();
        DBG("Copy failed, error " << err);
        #endif

        CYDER_ASSERT_FALSE;

        throw std::runtime_error(juce::String("Failed to copy plugin to: "
            + destFile.getFullPathName()
            #if JUCE_WINDOWS
            + " (Win32 error " + juce::String(err) + ")")
            #else
            )
            #endif
            .toStdString());
    }

    #if JUCE_WINDOWS
    // Copy the matching PDF file (if it exists) into the same folder as the copied dll ---
    // else the original will be used (since it is hard coded in the binary) --- 
    auto originalPdb = originalFile.withFileExtension(".pdb");
    if (originalPdb.existsAsFile())
    {
        juce::File destDir = destFile
                .getChildFile("Contents")
                .getChildFile("x86_64-win");

        if (destDir.exists())
        {
            // place the PDB next to the DLL
            auto destPdb = destDir.getChildFile(originalPdb.getFileNameWithoutExtension())
                                  .withFileExtension(".pdb");
            if (! originalPdb.copyFileTo(destPdb))
            {
                DBG("Failed to copy PDB to temp: " << originalPdb.getFullPathName());
                CYDER_ASSERT_FALSE;
            }
        }
        else
        {
            DBG("Destination directory not found, skipping PDB copy");
            CYDER_ASSERT_FALSE;
        }
    }
    #endif

    return destFile;
}

juce::PluginDescription Utilities::findPluginDescription(const juce::File& pluginFile,
                                                         juce::AudioPluginFormatManager& formatManager) noexcept(false)
{
    juce::KnownPluginList pluginList;
    juce::OwnedArray<juce::PluginDescription> descriptions;
    juce::StringArray files;
    files.add (pluginFile.getFullPathName());
    pluginList.scanAndAddDragAndDroppedFiles(formatManager, files, descriptions);
    if (descriptions.isEmpty())
    {
        CYDER_ASSERT_FALSE;
        throw std::runtime_error(("Plugin not found: " + pluginFile.getFullPathName()).toStdString());
    }
    return *descriptions[0];
}

std::unique_ptr<juce::AudioPluginInstance> Utilities::createInstance(const juce::PluginDescription& description,
                                                                     juce::AudioPluginFormatManager& formatManager,
                                                                     double sampleRate,
                                                                     int blockSize) noexcept(false)
{    
    juce::String errorMessage;
    auto instance = formatManager.createPluginInstance(description, sampleRate, blockSize, errorMessage);
    if (instance == nullptr)
    {
        CYDER_ASSERT_FALSE;
        throw std::runtime_error(("Error creating plugin instance: " + errorMessage).toStdString());
    }
    return instance;
}

std::unique_ptr<juce::DocumentWindow> Utilities::createAndShowEditorWindow(juce::AudioPluginInstance* instance,
                                                                           const juce::String& windowTitle) noexcept(false)
{
    auto* editor = instance->createEditor();
    if (editor == nullptr)
    {
        CYDER_ASSERT_FALSE;
        throw std::runtime_error("Plugin has no editor");
    }
    int w = editor->getWidth();
    int h = editor->getHeight();
    if (w <= 0 || h <= 0)
        editor->setSize(w = 400, h = 300);
    auto window = std::make_unique<juce::DocumentWindow>(windowTitle,
                                                         juce::Colours::lightgrey,
                                                         juce::DocumentWindow::allButtons,
                                                         true);
    window->setUsingNativeTitleBar(true);
    window->setContentOwned(editor, true);
    window->centreWithSize(w, h);
    window->setVisible(true);
    window->setTopLeftPosition(0, 0);
    
    return window;
}
