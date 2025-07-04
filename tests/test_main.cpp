
#include <gtest/gtest.h>

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================

int main(int argc, char** argv)
{
    juce::MessageManager::getInstance()->setCurrentThreadAsMessageThread();
    
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    
    juce::DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance(); // must be last
    
    return result;
}
