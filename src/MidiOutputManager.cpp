#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager() { reset(); }

void MidiOutputManager::reset()
{
    readPos = 0;
    count   = 0;
}

void MidiOutputManager::scheduleNote(int64 noteOnSample, int64 noteOffSample,
                                      int channel, int noteNumber, int velocity,
                                      int bendValue, int bendOffsetSamples)
{
    auto addEvent = [&](ScheduledEvent e) {
        if (count < kMaxEvents)
        {
            events[(readPos + count) % kMaxEvents] = e;
            ++count;
        }
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
    int   kept     = 0;

    for (int i = 0; i < count; ++i)
    {
        ScheduledEvent& e = events[(readPos + i) % kMaxEvents];

        if (e.samplePosition >= blockStart && e.samplePosition < blockEnd)
        {
            out.addEvent(e.msg, (int)(e.samplePosition - blockStart));
            // consumed — not copied to the kept region
        }
        else
        {
            // Compact to the front of the active region.
            // When kept == i the assignment is a harmless self-copy.
            events[(readPos + kept) % kMaxEvents] = e;
            ++kept;
        }
    }

    count = kept;
    // readPos is unchanged — kept events start from the same physical slot.
    // writePos (logical) is now readPos + count, maintained implicitly.
}

void MidiOutputManager::panic(juce::MidiBuffer& out, int channel)
{
    for (int note = 0; note < 128; ++note)
        out.addEvent(juce::MidiMessage::noteOff(channel, note), 0);
    out.addEvent(juce::MidiMessage::allNotesOff(channel), 0);
    out.addEvent(juce::MidiMessage::pitchWheel(channel, 8192), 0);
    reset();
}
