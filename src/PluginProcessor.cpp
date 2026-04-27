#include "PluginProcessor.h"
#include "PluginEditor.h"

GrooveLockProcessor::GrooveLockProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Locate presets. For VST3 the DLL lives at bundle.vst3/Contents/x86_64-win/
    // so walk three parent dirs up to reach bundle.vst3/, then look for presets/.
    juce::File dllFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    juce::File presetsDir = dllFile.getParentDirectory()   // x86_64-win/
                                   .getParentDirectory()   // Contents/
                                   .getParentDirectory()   // bundle.vst3/
                                   .getChildFile("presets");
    if (!presetsDir.isDirectory())
        presetsDir = dllFile.getParentDirectory().getChildFile("presets");
    if (!presetsDir.isDirectory())
        presetsDir = juce::File::getCurrentWorkingDirectory().getChildFile("presets");

    browser.loadFromDirectory(presetsDir);
    loadTemplate(0);
}

GrooveLockProcessor::~GrooveLockProcessor() {}

void GrooveLockProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate  = sampleRate;
    totalSamplesPlayed = 0;
    midiOut.reset();
    lockEngine.reset();
}

void GrooveLockProcessor::releaseResources() {}

void GrooveLockProcessor::loadTemplate(int index)
{
    auto* t = browser.getTemplate(index);
    if (!t) return;
    templateIndex.set(index);
    currentTmpl.store(t, std::memory_order_release);
    phraseParamsDirty.set(1); // new template → regenerate phrase
}

void GrooveLockProcessor::regeneratePhrase()
{
    auto* t     = currentTmpl.load(std::memory_order_relaxed);
    auto  genre = t ? t->genre : juce::String("Modern West Coast");
    auto  prof  = GenreProfile::forGenre(genre);

    float d  = density.get();
    float tn = tension.get();

    // Ignore genre clamps when override is active
    if (clampOverride.get())
    {
        prof.densityClampMin = 0.f; prof.densityClampMax = 1.f;
        prof.tensionClampMin = 0.f; prof.tensionClampMax = 1.f;
    }

    phraseExpander.compute(t, prof, d, tn);

    // Write to inactive buffer, then atomically make it active
    phraseBuffers[inactiveBuffer] = phraseExpander.getPhrase();
    activePhraseIdx.store(inactiveBuffer, std::memory_order_release);
    inactiveBuffer = 1 - inactiveBuffer;
}

void GrooveLockProcessor::sendPanic() { panicFlag.set(1); }

void GrooveLockProcessor::syncParams()
{
    LockEngineParams p;
    p.swingPercent    = swingPercent.get();
    p.humanizePercent = humanizePercent.get();
    p.velOffset       = velOffset.get();
    p.timingOffsetMs  = timingOffsetMs.get();
    p.gateLengthScale = gateLengthScale.get();
    p.glideTimeMs     = glideTimeMs.get();
    p.outputChannel   = outputChannel.get();
    p.outputRootNote  = outputRootNote.get();
    p.pitchBendRange  = pitchBendRange.get();

    p.pitch.rootMidiNote      = outputRootNote.get();
    p.pitch.scaleType         = static_cast<ScaleType>(pitchScale.get());
    p.pitch.densityOverride   = pitchDensity.get();
    p.pitch.chromaticApproach = pitchChromatic.get() != 0;
    p.pitch.pitchEnabled      = pitchEnabled.get() != 0;

    // Pass active phrase buffer pointer — acquire ordering sees completed phrase write
    const ExpandedPhrase* activePhrase =
        &phraseBuffers[activePhraseIdx.load(std::memory_order_acquire)];
    lockEngine.setExpandedPhrase(activePhrase->isValid ? activePhrase : nullptr);

    // Live drum gating: pass analyzer state when in live input mode
    p.liveDrums = (inputMode.get() == 0) ? &analyzer.getState() : nullptr;

    lockEngine.setParams(p);
}

void GrooveLockProcessor::processBlock(juce::AudioBuffer<float>& audio,
                                        juce::MidiBuffer& midi)
{
    audio.clear();
    juce::ScopedNoDenormals noDenormals;

    // Panic
    if (panicFlag.compareAndSetBool(0, 1))
    {
        midiOut.panic(midi, outputChannel.get());
        return;
    }

    // Pick up template changes written by the message thread
    auto* t = currentTmpl.load(std::memory_order_acquire);
    if (t != lastAppliedTmpl)
    {
        lastAppliedTmpl = t;
        lockEngine.setTemplate(t);
    }

    syncParams();

    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (auto* ph = getPlayHead())
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)
        ph->getCurrentPosition(pos);
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    if (!pos.isPlaying || !patternActive.get()) return;

    // Feed drum MIDI to analyzer (live mode)
    if (inputMode.get() == 0)
        analyzer.process(midi, pos, currentSampleRate, audio.getNumSamples());

    // Update step + phrase-bar position for UI
    if (pos.isPlaying)
    {
        double barPos = std::fmod(pos.ppqPosition, 4.0);
        int step = (int)std::floor(barPos * 4.0) & 15;
        currentStep.set(step);
    }

    juce::MidiBuffer outBuf;
    lockEngine.process(midiOut, pos, currentSampleRate, totalSamplesPlayed, audio.getNumSamples());
    midiOut.flush(outBuf, totalSamplesPlayed, audio.getNumSamples());

    // Expose phrase-bar for UI and detect per-loop boundary
    int pBar = lockEngine.getCurrentPhraseBar();
    currentPhraseBar.set(pBar);
    if (regenMode.get() == 1 && pBar == 0 && lastPhraseBar == 7)
        needsRegen.set(1);
    lastPhraseBar = pBar;
    totalSamplesPlayed += audio.getNumSamples();

    midi.clear();
    midi.addEvents(outBuf, 0, audio.getNumSamples(), 0);
}

void GrooveLockProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    juce::ValueTree state("GrooveLock");
    state.setProperty("templateIndex",   templateIndex.get(),   nullptr);
    state.setProperty("swingPercent",    swingPercent.get(),    nullptr);
    state.setProperty("humanizePercent", humanizePercent.get(), nullptr);
    state.setProperty("velOffset",       velOffset.get(),       nullptr);
    state.setProperty("timingOffsetMs",  timingOffsetMs.get(),  nullptr);
    state.setProperty("gateLengthScale", gateLengthScale.get(), nullptr);
    state.setProperty("glideTimeMs",     glideTimeMs.get(),     nullptr);
    state.setProperty("outputChannel",   outputChannel.get(),   nullptr);
    state.setProperty("outputRootNote",  outputRootNote.get(),  nullptr);
    state.setProperty("pitchBendRange",  pitchBendRange.get(),  nullptr);
    state.setProperty("inputMode",       inputMode.get(),       nullptr);
    state.setProperty("pitchEnabled",    pitchEnabled.get(),    nullptr);
    state.setProperty("pitchScale",      pitchScale.get(),      nullptr);
    state.setProperty("pitchDensity",    pitchDensity.get(),    nullptr);
    state.setProperty("pitchChromatic",  pitchChromatic.get(),  nullptr);
    state.setProperty("density",         density.get(),         nullptr);
    state.setProperty("tension",         tension.get(),         nullptr);
    state.setProperty("regenMode",       regenMode.get(),       nullptr);

    juce::MemoryOutputStream mos(dest, true);
    state.writeToStream(mos);
}

void GrooveLockProcessor::setStateInformation(const void* data, int size)
{
    juce::ValueTree state = juce::ValueTree::readFromData(data, (size_t)size);
    if (!state.isValid()) return;

    auto getF = [&](const char* id, float def) -> float {
        return state.hasProperty(id) ? (float)state[id] : def;
    };
    auto getI = [&](const char* id, int def) -> int {
        return state.hasProperty(id) ? (int)state[id] : def;
    };

    swingPercent.set    (getF("swingPercent",    55.f));
    humanizePercent.set (getF("humanizePercent", 20.f));
    velOffset.set       (getF("velOffset",        0.f));
    timingOffsetMs.set  (getF("timingOffsetMs",   0.f));
    gateLengthScale.set (getF("gateLengthScale", 1.0f));
    glideTimeMs.set     (getF("glideTimeMs",    100.f));
    outputChannel.set   (getI("outputChannel",     2));
    outputRootNote.set  (getI("outputRootNote",   36));
    pitchBendRange.set  (getI("pitchBendRange",    2));
    inputMode.set       (getI("inputMode",         1));
    pitchEnabled.set    (getI("pitchEnabled",      0));
    pitchScale.set      (getI("pitchScale",        0));
    pitchDensity.set    (getI("pitchDensity",      0));
    pitchChromatic.set  (getI("pitchChromatic",    1));
    density.set         (getF("density",         0.5f));
    tension.set         (getF("tension",         0.5f));
    regenMode.set       (getI("regenMode",          1));
    loadTemplate        (getI("templateIndex",     0));
}

juce::AudioProcessorEditor* GrooveLockProcessor::createEditor()
{
    return new GrooveLockEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveLockProcessor();
}
