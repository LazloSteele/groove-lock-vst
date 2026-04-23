#pragma once
#include <JuceHeader.h>

struct ScheduledEvent
{
    int64  samplePosition = 0;
    juce::MidiMessage msg;
    bool   operator<(const ScheduledEvent& o) const { return samplePosition < o.samplePosition; }
};

class MidiOutputManager
{
public:
    MidiOutputManager();

    // Queue a note with note-off. All sample positions are absolute (from start of current processBlock).
    void scheduleNote(int64 noteOnSample, int64 noteOffSample,
                      int channel, int noteNumber, int velocity,
                      int bendValue = 8192, int bendOffsetSamples = 0);

    // Flush events that fall within [blockStart, blockStart+numSamples)
    // into the output buffer, using relative sample positions.
    void flush(juce::MidiBuffer& out, int64 blockStart, int numSamples);

    void panic(juce::MidiBuffer& out, int channel);

    void reset();

private:
    // Fixed-size ring buffer for scheduled events
    static constexpr int kMaxEvents = 512;
    ScheduledEvent events[kMaxEvents];
    int            writePos = 0;
    int            count    = 0;

    juce::CriticalSection lock; // only used for note tracking (message thread panic)
};
