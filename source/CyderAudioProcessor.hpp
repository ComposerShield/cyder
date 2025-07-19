/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderAudioProcessor.hpp
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

#include <memory>

//==============================================================================

enum class CyderStatus
{
    idle,
    loading,
    reloading,
    successfullyLoadedPlugin,
    successfullyReloadedPlugin,
    failedToLoadPlugin,
    failedToReloadPlugin,
};

class HotReloadThread;

//==============================================================================

class CyderAudioProcessor
: public juce::AudioProcessor
, public juce::AudioProcessorListener
{
public:
    CyderAudioProcessor();
    ~CyderAudioProcessor() override;
    
    //==============================================================================
    
    /** */
    bool loadPlugin(const juce::String& pluginPath);
    /** */
    void unloadPlugin();
    
    /** */
    CyderStatus getCurrentStatus() const noexcept;
    
    /** */
    juce::AudioProcessor* getWrappedPluginProcessor() const noexcept;
    /** */
    juce::AudioProcessorEditor* getWrappedPluginEditor() const noexcept;
    
    /** */
    juce::Thread* getHotReloadThread() const noexcept;
    
    /** */
    juce::File getCurrentWrappedPluginPathCopy() const noexcept;
    /** */
    juce::File getCurrentWrappedPluginPathOriginal() const noexcept;
    
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
    
    //==============================================================================
    
    void audioProcessorParameterChanged (juce::AudioProcessor* /*processor*/,
                                         int /*parameterIndex*/,
                                         float /*newValue*/) override {}
    void audioProcessorChanged (juce::AudioProcessor* processor,
                                const AudioProcessorListener::ChangeDetails& details) override;
    
    //==============================================================================
    
private:
    CyderStatus currentStatus = CyderStatus::idle;
    
    juce::File currentPluginFileOriginal;
    juce::File currentPluginFileCopy;
    
    std::unique_ptr<juce::AudioPluginInstance>  wrappedPlugin;
    std::unique_ptr<juce::AudioProcessorEditor> wrappedPluginEditor;
    
    std::unique_ptr<HotReloadThread> hotReloadThread;
    
    juce::AudioBuffer<float> monoToStereoBuffer;
    
    void transferPluginState(juce::AudioProcessor& destinationProcessor) noexcept;
    
    void processUsingMonoToStereoBuffer(juce::AudioBuffer<float>&, juce::MidiBuffer&);
    
    JUCE_DECLARE_WEAK_REFERENCEABLE (CyderAudioProcessor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyderAudioProcessor)
};
