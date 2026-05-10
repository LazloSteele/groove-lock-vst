#pragma once
#include <JuceHeader.h>

struct ScheduledEvent
{
    int64  samplePosition = 0;
    juce::MidiMessage msg;
};

class MidiOutputManager
{
public:
    MidiOutputManager();

    // Queue a note with note-off. All sample positions are absolute.
    void scheduleNote(int64 noteOnSample, int64 noteOffSample,
                      int channel, int noteNumber, int velocity,
                      int bendValue = 8192, int bendOffsetSamples = 0);

    // Emit all events in [blockStart, blockStart+numSamples) into out.
    // Remaining events are compacted to the front of the internal buffer —
    // no dead copies are left behind.
    void flush(juce::MidiBuffer& out, int64 blockStart, int numSamples);

    void panic(juce::MidiBuffer& out, int channel);
    void reset();

private:
    static constexpr int kMaxEvents = 512;
    ScheduledEvent events[kMaxEvents];
    int readPos = 0; // logical start of active region (circular)
    int count   = 0; // number of active events
};
