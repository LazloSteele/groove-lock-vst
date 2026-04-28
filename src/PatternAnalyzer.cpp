#include "PatternAnalyzer.h"

// ─── Preset table ─────────────────────────────────────────────────────────────
// Notes marked * should be verified against hardware/firmware MIDI specs
// before a public release.  The note numbers listed here are the most commonly
// observed defaults in the wild and are correct for the vast majority of users.

const std::vector<DrumMappingPreset>& DrumMappingPresets::getAll()
{
    using A = juce::Array<int>;
    static const std::vector<DrumMappingPreset> presets = [] {
        std::vector<DrumMappingPreset> p;

        auto add = [&](const char* name, A kicks, A snares, A hats) {
            DrumMappingPreset preset;
            preset.name              = name;
            preset.mapping.kickNotes  = std::move(kicks);
            preset.mapping.snareNotes = std::move(snares);
            preset.mapping.hatNotes   = std::move(hats);
            p.push_back(std::move(preset));
        };

        //                            name                      kick         snare + clap      hat (closed/pedal/open)
        add("GM / General MIDI",               {35, 36},    {38, 39, 40}, {42, 44, 46});
        add("Ableton Drum Rack",               {36},        {38, 39},     {42, 46});
        add("Roland TR-808",                   {36},        {38, 39, 40}, {42, 46});
        add("Roland TR-909",                   {36},        {38, 39, 40}, {42, 44, 46});
        add("Roland TR-8S",                    {36},        {38, 39},     {42, 46});
        add("Akai MPC (Classic)",              {35},        {38, 39},     {42, 46}); // *MPC60/3000 kick on B0
        add("Akai MPC (Modern)",               {36},        {38, 39},     {42, 46});
        add("NI Maschine",                     {36},        {38, 39},     {42, 46});
        add("NI Battery 4",                    {36},        {38, 39},     {42, 46});
        add("XO by XLN Audio",                 {36},        {38, 39},     {42, 46}); // *verify
        add("Roland SP-404 / SP-404SX",        {36},        {38, 39},     {42, 46}); // *verify factory pad layout
        add("Elektron Digitakt",               {36},        {38, 39},     {42, 46}); // *user-assignable; reflects common template
        add("Reason Redrum",                   {36},        {38, 39},     {42, 46});
        add("Superior Drummer 3",              {36},        {38, 39, 40}, {42, 46}); // *core notes only
        add("EZdrummer 3",                     {36},        {38, 39},     {42, 46});
        add("Slate Drums SSD5",                {36},        {38, 39},     {42, 46});

        return p;
    }();
    return presets;
}

// ─── PatternAnalyzer ──────────────────────────────────────────────────────────

void PatternAnalyzer::buildNoteMap(std::array<int8_t, 128>& map, const DrumMapping& m)
{
    map.fill(3); // default: perc (catch-all)
    // Write in reverse priority so kick always beats snare beats hat on collision
    for (int n : m.hatNotes)   if (n >= 0 && n < 128) map[(size_t)n] = 2;
    for (int n : m.snareNotes) if (n >= 0 && n < 128) map[(size_t)n] = 1;
    for (int n : m.kickNotes)  if (n >= 0 && n < 128) map[(size_t)n] = 0;
}

PatternAnalyzer::PatternAnalyzer()
{
    noteMaps[0].fill(3);
    noteMaps[1].fill(3);
    reset();
    setMapping(DrumMapping{}); // build initial noteMap from GM defaults
}

void PatternAnalyzer::reset()
{
    std::memset(&state, 0, sizeof(state));
}

void PatternAnalyzer::setMapping(const DrumMapping& m)
{
    int inactive = 1 - activeMap.load(std::memory_order_relaxed);
    buildNoteMap(noteMaps[inactive], m);
    activeMap.store(inactive, std::memory_order_release);
}

int PatternAnalyzer::noteToCategory(int note) const
{
    if (note < 0 || note >= 128) return 3;
    return noteMaps[activeMap.load(std::memory_order_acquire)][(size_t)note];
}

void PatternAnalyzer::process(const juce::MidiBuffer& midi,
                               const juce::AudioPlayHead::CurrentPositionInfo& pos,
                               double sampleRate, int numSamples)
{
    if (!pos.isPlaying) return;

    const double beatsPerBar  = 4.0;
    const double stepsPerBeat = 4.0;
    const double bpm          = pos.bpm > 0.0 ? pos.bpm : 120.0;

    // Clear stale hits whenever a bar boundary falls within this block
    double blockEndPPQ = pos.ppqPosition + (double)numSamples / sampleRate * bpm / 60.0;
    if ((int)(pos.ppqPosition / beatsPerBar) < (int)(blockEndPPQ / beatsPerBar))
        reset();

    for (const auto meta : midi)
    {
        auto msg = meta.getMessage();
        if (!msg.isNoteOn()) continue;

        double sampleOffset = meta.samplePosition;
        double ppqAtEvent   = pos.ppqPosition + (sampleOffset / sampleRate) * (bpm / 60.0);
        double barPos       = std::fmod(ppqAtEvent, beatsPerBar);
        int    step         = (int)std::floor(barPos * stepsPerBeat) & 15;
        int    vel          = msg.getVelocity();

        switch (noteToCategory(msg.getNoteNumber()))
        {
            case 0: state.kickHits[step]  = vel; break;
            case 1: state.snareHits[step] = vel; break;
            case 2: state.hatHits[step]   = vel; break;
            case 3: state.percHits[step]  = vel; break;
        }
    }
}
