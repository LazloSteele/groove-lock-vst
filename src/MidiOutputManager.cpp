#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager() { reset(); }

void MidiOutputManager::reset()
{
    writePos = 0;
    count    = 0;
}

void MidiOutputManager::scheduleNote(int64 noteOnSample, int64 noteOffSample,
                                      int channel, int noteNumber, int velocity,
                                      int bendValue, int bendOffsetSamples)
{
    auto addEvent = [&](ScheduledEvent e) {
        events[writePos % kMaxEvents] = e;
        ++writePos;
        if (count < kMaxEvents) ++count;
    };

    if (bendValue != 8192)
    {
        ScheduledEvent bend;
        bend.samplePosition = noteOnSample;
        bend.msg = juce::MidiMessage::pitchWheel(channel, bendValue);
        addEvent(bend);

        ScheduledEvent bendOff;
        bendOff.samplePosition = noteOnSample + bendOffsetSamples;
        bendOff.msg = juce::MidiMessage::pitchWheel(channel, 8192);
        addEvent(bendOff);
    }

    ScheduledEvent on;
    on.samplePosition = noteOnSample;
    on.msg = juce::MidiMessage::noteOn(channel, noteNumber, (uint8)velocity);
    addEvent(on);

    ScheduledEvent off;
    off.samplePosition = noteOffSample;
    off.msg = juce::MidiMessage::noteOff(channel, noteNumber);
    addEvent(off);
}

void MidiOutputManager::flush(juce::MidiBuffer& out, int64 blockStart, int numSamples)
{
    int64 blockEnd = blockStart + numSamples;
    int readHead   = (writePos - count + kMaxEvents) % kMaxEvents;

    for (int i = 0; i < count; )
    {
        int idx = (readHead + i) % kMaxEvents;
        auto& e = events[idx];

        if (e.samplePosition >= blockStart && e.samplePosition < blockEnd)
        {
            int relPos = (int)(e.samplePosition - blockStart);
            out.addEvent(e.msg, relPos);
            // Remove by swapping with the last element
            int lastIdx = (readHead + count - 1) % kMaxEvents;
            if (idx != lastIdx) events[idx] = events[lastIdx];
            --count;
            // Don't advance i — recheck this slot
        }
        else
        {
            ++i;
        }
    }
}

void MidiOutputManager::panic(juce::MidiBuffer& out, int channel)
{
    for (int note = 0; note < 128; ++note)
        out.addEvent(juce::MidiMessage::noteOff(channel, note), 0);
    out.addEvent(juce::MidiMessage::allNotesOff(channel), 0);
    out.addEvent(juce::MidiMessage::pitchWheel(channel, 8192), 0);
    reset();
}
