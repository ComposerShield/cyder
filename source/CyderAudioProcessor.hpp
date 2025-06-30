#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================

class HotReloadThread;

//==============================================================================

class CyderAudioProcessor : public juce::AudioProcessor
{
public:
    CyderAudioProcessor();
    ~CyderAudioProcessor() override;
    
    //==============================================================================
    
    bool loadPlugin(const juce::String& pluginPath);
    void unloadPlugin();
    
    juce::AudioProcessorEditor* getWrappedPluginEditor() const noexcept;
    
    //==============================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
private:
    juce::File currentPluginFile;
    juce::AudioPluginFormatManager formatManager;
    
    std::mutex wrappedPluginMutex;
    
    std::unique_ptr<juce::AudioPluginInstance>  wrappedPlugin;
    std::unique_ptr<juce::AudioProcessorEditor> wrappedPluginEditor;
    
    std::unique_ptr<HotReloadThread> hotReloadThread;
    
    void transferPluginState(juce::AudioProcessor& destinationProcessor) noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyderAudioProcessor)
};
