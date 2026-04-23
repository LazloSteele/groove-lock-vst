#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "PatternAnalyzer.h"
#include "LockEngine.h"
#include "MidiOutputManager.h"
#include "TemplateBrowser.h"

class GrooveLockProcessor : public juce::AudioProcessor
{
public:
    GrooveLockProcessor();
    ~GrooveLockProcessor() override;

    //=== AudioProcessor ===
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Groove Lock"; }
    bool   acceptsMidi() const override  { return true; }
    bool   producesMidi() const override { return true; }
    bool   isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override    { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    //=== Public thread-safe API for editor ===
    void loadTemplate(int index);
    void sendPanic();

    TemplateBrowser& getTemplateBrowser() { return browser; }

    // Parameters (accessed from editor; modified on message thread, read on audio thread via atomics)
    juce::Atomic<float> swingPercent    { 55.f };
    juce::Atomic<float> humanizePercent { 20.f };
    juce::Atomic<float> velOffset       { 0.f  };
    juce::Atomic<float> timingOffsetMs  { 0.f  };
    juce::Atomic<float> gateLengthScale { 1.0f };
    juce::Atomic<float> glideTimeMs     { 100.f };
    juce::Atomic<int>   outputChannel   { 2    };
    juce::Atomic<int>   outputRootNote  { 36   };
    juce::Atomic<int>   pitchBendRange  { 2    };
    juce::Atomic<int>   inputMode       { 1    }; // 0=live 1=internal
    juce::Atomic<int>   patternActive   { 1    };
    juce::Atomic<int>   templateIndex   { 0    };
    juce::Atomic<int>   panicFlag       { 0    };

    // Current template for UI reads (written audio-thread-safe via double buffer)
    const GrooveTemplate* getCurrentTemplate() const { return activeTmpl.load(); }

    // Step position indicator for UI (written by audio thread, read by UI)
    juce::Atomic<int> currentStep { 0 };

private:
    TemplateBrowser  browser;
    PatternAnalyzer  analyzer;
    LockEngine       lockEngine;
    MidiOutputManager midiOut;

    // Double-buffer template swap
    std::unique_ptr<GrooveTemplate> stagingTmpl;
    std::atomic<GrooveTemplate*>    activeTmpl  { nullptr };
    juce::Atomic<int>               templateSwapFlag { 0 };

    double currentSampleRate  = 44100.0;
    int64  totalSamplesPlayed = 0;

    void syncParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveLockProcessor)
};
