#pragma once
// Ableton 12 rejects any VST3 plugin whose sub-category contains "Fx" but has
// no audio input bus, including the standard "Fx|MIDI" MIDI-effect category.
// JUCE forces "Fx" into every non-instrument category, so we strip it here at
// the preprocessor level, leaving just "MIDI".
#ifdef JucePlugin_Vst3Category
  #undef JucePlugin_Vst3Category
#endif
#define JucePlugin_Vst3Category "MIDI"
