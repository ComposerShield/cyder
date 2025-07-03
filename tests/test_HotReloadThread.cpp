
#include <gtest/gtest.h>

#include "../source/HotReloadThread.hpp"

//==============================================================================

TEST(HotReloadThreadRun, DetectsChangedBinary)
{
    juce::File currentFile(__FILE__);
    // Cyder.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getSiblingFile("../Cyder.vst3");
    
    bool hotReloadThreadDetectedChange = false;
    
    // Create thread (automatically starts running)
    HotReloadThread thread(pluginFile);
    thread.onPluginChangeDetected = [&hotReloadThreadDetectedChange] { hotReloadThreadDetectedChange = true; };
    
    // Give thread a moment to start churning
    juce::Thread::sleep(10);
    
    // Pretend to change binary
    juce::File binaryFile = pluginFile.getChildFile("Contents")
                                      .getChildFile("MacOS")
                                      .getChildFile("Cyder");
    binaryFile.setLastModificationTime(juce::Time::getCurrentTime());
    
    // Give HotReloadThread time to detect change
    juce::Thread::sleep(1000);
    
    // Change was detected
    ASSERT_TRUE(hotReloadThreadDetectedChange);
    
    // Stop thread
    thread.stopThread(1000);
}

TEST(HotReloadThreadRun, DetectsAddedFile)
{
    juce::File currentFile(__FILE__);
    // Cyder.vst3 must have already been built and copied into root directory
    // so we can use it as a testable VST3
    juce::File pluginFile = currentFile.getSiblingFile("../Cyder.vst3");
    
    bool hotReloadThreadDetectedChange = false;
    
    // Create thread (automatically starts running)
    HotReloadThread thread(pluginFile);
    thread.onPluginChangeDetected = [&hotReloadThreadDetectedChange] { hotReloadThreadDetectedChange = true; };
    
    // Give thread a moment to start churning
    juce::Thread::sleep(10);
    
    // Add a new file in Resources that was not present before
    juce::File resourceFile = pluginFile.getChildFile("Contents")
                                        .getChildFile("Resources")
                                        .getChildFile("data.txt");
    
    auto result = resourceFile.create();
    ASSERT_TRUE(result.ok());
    resourceFile.setLastModificationTime(juce::Time::getCurrentTime());
    
    // Give HotReloadThread time to detect change (should still be
    juce::Thread::sleep(1000);
    
    // Change was detected
    ASSERT_TRUE(hotReloadThreadDetectedChange);
    
    // Stop thread
    thread.stopThread(1000);
    
    // Cleanup
    resourceFile.deleteFile();
}
