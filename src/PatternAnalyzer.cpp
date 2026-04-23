#include "PatternAnalyzer.h"

PatternAnalyzer::PatternAnalyzer() { reset(); }

void PatternAnalyzer::reset()
{
    std::memset(&state, 0, sizeof(state));
}

int PatternAnalyzer::noteToCategory(int note) const
{
    if (mapping.kickNotes.contains(note))  return 0;
    if (mapping.snareNotes.contains(note)) return 1;
    if (mapping.hatNotes.contains(note))   return 2;
    return 3; // perc
}

void PatternAnalyzer::process(const juce::MidiBuffer& midi,
                               const juce::AudioPlayHead::CurrentPositionInfo& pos,
                               double sampleRate, int numSamples)
{
    if (!pos.isPlaying) return;

    const double beatsPerBar  = 4.0;
    const double stepsPerBeat = 4.0;

    for (const auto meta : midi)
    {
        auto msg = meta.getMessage();
        if (!msg.isNoteOn()) continue;

        // Calculate beat position of this event
        double sampleOffset   = meta.samplePosition;
        double ppqAtEvent     = pos.ppqPosition + (sampleOffset / sampleRate) * (pos.bpm / 60.0);
        double barPos         = std::fmod(ppqAtEvent, beatsPerBar);
        int    step           = (int)std::floor(barPos * stepsPerBeat) & 15;
        int    vel            = msg.getVelocity();

        switch (noteToCategory(msg.getNoteNumber()))
        {
            case 0: state.kickHits[step]  = vel; break;
            case 1: state.snareHits[step] = vel; break;
            case 2: state.hatHits[step]   = vel; break;
            case 3: state.percHits[step]  = vel; break;
        }
    }
}
