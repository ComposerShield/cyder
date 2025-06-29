
#include "Utilities.hpp"

juce::File Utilities::copyPluginToTempWithHash(const juce::File& originalFile) noexcept(false)
{
    // Create or reuse a temp subdirectory for copied plugins
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("CyderPlugins");
    tempDir.createDirectory();

    // Generate a random UUID and build new filename
    auto uuid      = juce::Uuid().toString();
    auto baseName  = originalFile.getFileNameWithoutExtension();
    auto ext       = originalFile.getFileExtension(); // includes leading dot
    auto destFile  = tempDir.getChildFile (baseName + "_" + uuid + ext);

    // Copy and verify
    if (! originalFile.copyFileTo(destFile))
        throw std::runtime_error(("Failed to copy plugin to: " + destFile.getFullPathName()).toStdString());

    return destFile;
}
