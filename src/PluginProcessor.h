#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "PatternAnalyzer.h"
#include "LockEngine.h"
#include "AdaptedPattern.h"
#include "MidiOutputManager.h"
#include "TemplateBrowser.h"
#include "PhraseExpander.h"

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
    bool   isMidiEffect() const override { return false; }
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
    juce::Atomic<int>   inputMode       { 1    };
    juce::Atomic<int>   patternActive   { 1    };
    juce::Atomic<int>   templateIndex   { 0    };
    juce::Atomic<int>   panicFlag       { 0    };

    // Pitch system
    juce::Atomic<int>   pitchEnabled    { 0    }; // master toggle
    juce::Atomic<int>   pitchScale      { 0    }; // ScaleType enum
    juce::Atomic<int>   pitchDensity    { 0    }; // 0=auto, 1-5=override
    juce::Atomic<int>   pitchChromatic  { 1    }; // chromatic approach on/off

    // 8-bar phrase expansion controls
    juce::Atomic<float> density         { 0.5f }; // 0=sparse, 1=busy
    juce::Atomic<float> tension         { 0.5f }; // 0=safe, 1=adventurous
    juce::Atomic<int>   regenMode       { 1    }; // 0=fixed, 1=per-loop, 2=manual
    juce::Atomic<int>   clampOverride   { 0    }; // 1=ignore genre clamps

    // Current template for UI reads (written audio-thread-safe via double buffer)
    const GrooveTemplate* getCurrentTemplate() const { return currentTmpl.load(std::memory_order_relaxed); }

    // Step and phrase-bar position for UI (written by audio thread, read by UI)
    juce::Atomic<int> currentStep      { 0 };
    juce::Atomic<int> currentPhraseBar { 0 };

    // Live drum state snapshot for display (written by audio thread, read by UI timer)
    DrumState liveDrumDisplay;

    // Drum mapping — message thread owns currentMapping; audio thread reads via analyzer
    DrumMapping currentMapping;
    void applyMapping(const DrumMapping& m); // message thread only

    // MIDI Learn state (set from message thread, captured on audio thread)
    juce::Atomic<int> learnState          { 0 };  // 0=idle 1=kick 2=snare 3=hat
    juce::Atomic<int> learnCapturedNote     { -1 };
    juce::Atomic<int> learnCapturedCategory { 0  };
    juce::Atomic<int> learnCaptureReady     { 0  };

    // Regenerate the 8-bar phrase. Call from message thread only.
    void regeneratePhrase();

    // Dirty flag: set from editor when density/tension change so timer can regen
    juce::Atomic<int> phraseParamsDirty { 1 }; // start dirty for initial generation
    juce::Atomic<int> needsRegen        { 0 }; // set by audio thread (per-loop)

    // Record Pattern session — message thread writes, audio thread reads
    juce::Atomic<int> recordArmed         { 0 }; // set by button to arm/re-record
    juce::Atomic<int> recordStopRequested { 0 }; // set by button to finalize recording
    juce::Atomic<int> recordStateForUI    { 0 }; // 0=idle, 1=recording, 2=locked (UI read-only)
    juce::Atomic<int> recordedBarCountUI  { 0 }; // bar count for UI display

private:
    TemplateBrowser  browser;
    PatternAnalyzer  analyzer;
    LockEngine       lockEngine;
    MidiOutputManager midiOut;

    // Browser templates are read-only and live for the plugin's lifetime,
    // so both threads can safely read through this atomic pointer.
    std::atomic<const GrooveTemplate*> currentTmpl { nullptr };
    const GrooveTemplate*              lastAppliedTmpl { nullptr }; // audio thread only

    double currentSampleRate  = 44100.0;
    int64  totalSamplesPlayed = 0;

    // Adaptive movement — audio thread only
    DrumState      completedBarState;
    AdaptedPattern adaptedPattern;

    // Record Pattern session — audio thread only
    enum class RecordState { IDLE, RECORDING, LOCKED };
    RecordState   recordState       = RecordState::IDLE;
    DrumState     recordedBars[16];
    int           recordedBarsCount = 0;
    int           lockedBarIdx      = 0;
    int           recordingBarsSeen = 0;
    int           lastAbsoluteBar   = -1;

    // Phrase expansion — double-buffered so message thread can write while audio thread reads
    PhraseExpander          phraseExpander;        // message thread only
    ExpandedPhrase          phraseBuffers[2];
    std::atomic<int>        activePhraseIdx { 0 }; // index of buffer audio thread reads
    int                     inactiveBuffer  { 1 }; // message thread only
    int                     lastPhraseBar   { -1 }; // audio thread only, for per-loop detection

    void syncParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveLockProcessor)
};
