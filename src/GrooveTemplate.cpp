#include "GrooveTemplate.h"

static juce::String drumTimingStr(DrumTiming t)
{
    switch (t) {
        case DrumTiming::PUSH: return "push";
        case DrumTiming::LAY:  return "lay";
        case DrumTiming::FLAM: return "flam";
        case DrumTiming::DRAG: return "drag";
        default:               return "grid";
    }
}

static juce::String bassArtStr(BassArt a)
{
    switch (a) {
        case BassArt::PUSH:     return "push";
        case BassArt::LAY:      return "lay";
        case BassArt::SLIDE:    return "slide";
        case BassArt::BEND:     return "bend";
        case BassArt::STACCATO: return "staccato";
        case BassArt::LEGATO:   return "legato";
        default:                return "grid";
    }
}

static juce::String lockTypeStr(LockType t)
{
    switch (t) {
        case LockType::ALTERNATE:  return "alternate";
        case LockType::ANTICIPATE: return "anticipate";
        case LockType::FILL:       return "fill";
        default:                   return "unison";
    }
}

bool GrooveTemplate::loadFromJSON(const juce::File& file)
{
    return loadFromJSON(file.loadFileAsString());
}

bool GrooveTemplate::loadFromJSON(const juce::String& jsonText)
{
    juce::var root;
    if (juce::JSON::parse(jsonText, root).failed())
        return false;

    auto* meta = root["meta"].getDynamicObject();
    if (!meta) return false;

    name          = meta->getProperty("name").toString();
    genre         = meta->getProperty("genre").toString();
    region        = meta->getProperty("region").toString();
    mood          = meta->getProperty("mood").toString();
    description   = meta->getProperty("description").toString();
    tempoMin      = (float)(double)meta->getProperty("tempoMin");
    tempoMax      = (float)(double)meta->getProperty("tempoMax");
    swingPercent  = (float)(double)meta->getProperty("swingPercent");

    drums.clear();
    auto& drumArr = *root["drums"].getArray();
    for (auto& rowVar : drumArr)
    {
        auto* rowObj = rowVar.getDynamicObject();
        if (!rowObj) continue;
        auto* row = drums.add(new DrumRow());
        row->label = rowObj->getProperty("label").toString();
        auto& steps  = *rowObj->getProperty("steps").getArray();
        auto& timing = *rowObj->getProperty("timing").getArray();
        for (int i = 0; i < 16; ++i)
        {
            row->steps[i]  = i < steps.size()  ? (int)steps[i]            : 0;
            row->timing[i] = i < timing.size() ? parseDrumTiming(timing[i].toString()) : DrumTiming::GRID;
        }
    }

    bass.clear();
    auto& bassArr = *root["bass"].getArray();
    for (auto& rowVar : bassArr)
    {
        auto* rowObj = rowVar.getDynamicObject();
        if (!rowObj) continue;
        auto* row = bass.add(new BassRow());
        row->label = rowObj->getProperty("label").toString();
        auto& steps  = *rowObj->getProperty("steps").getArray();
        auto& timing = *rowObj->getProperty("timing").getArray();
        for (int i = 0; i < 16; ++i)
        {
            row->steps[i]  = i < steps.size()  ? (int)steps[i]           : 0;
            row->timing[i] = i < timing.size() ? parseBassArt(timing[i].toString()) : BassArt::GRID;
        }
    }

    locks.clear();
    if (auto* lockArr = root["locks"].getArray())
    {
        for (auto& lVar : *lockArr)
        {
            auto* lObj = lVar.getDynamicObject();
            if (!lObj) continue;
            LockPoint lp;
            lp.step        = (int)lObj->getProperty("step");
            lp.type        = parseLockType(lObj->getProperty("type").toString());
            lp.description = lObj->getProperty("description").toString();
            locks.add(lp);
        }
    }

    bass2.clear();
    if (auto* bass2ArrPtr = root["bass2"].getArray())
    {
        for (auto& rowVar : *bass2ArrPtr)
        {
            auto* rowObj = rowVar.getDynamicObject();
            if (!rowObj) continue;
            auto* row = bass2.add(new BassRow());
            row->label = rowObj->getProperty("label").toString();
            auto& steps  = *rowObj->getProperty("steps").getArray();
            auto& timing = *rowObj->getProperty("timing").getArray();
            for (int i = 0; i < 16; ++i)
            {
                row->steps[i]  = i < steps.size()  ? (int)steps[i]           : 0;
                row->timing[i] = i < timing.size() ? parseBassArt(timing[i].toString()) : BassArt::GRID;
            }
        }
    }

    locks2.clear();
    if (auto* lock2Arr = root["locks2"].getArray())
    {
        for (auto& lVar : *lock2Arr)
        {
            auto* lObj = lVar.getDynamicObject();
            if (!lObj) continue;
            LockPoint lp;
            lp.step        = (int)lObj->getProperty("step");
            lp.type        = parseLockType(lObj->getProperty("type").toString());
            lp.description = lObj->getProperty("description").toString();
            locks2.add(lp);
        }
    }

    pitch = PitchBlock();
    if (auto* pitchObj = root["pitch"].getDynamicObject())
    {
        pitch.hasPitchData            = true;
        pitch.densityHint             = (int)pitchObj->getProperty("densityHint");
        {
            auto v = pitchObj->getProperty("productionComplexity");
            pitch.productionComplexity = v.isVoid() ? 0.5f : (float)(double)v;
        }
        pitch.allowChromaticApproach  = (bool)pitchObj->getProperty("allowChromaticApproach");

        if (auto* intArr = pitchObj->getProperty("preferredIntervals").getArray())
            for (auto& v : *intArr)
                pitch.preferredIntervals.add(parsePitchRole(v.toString()));

        if (auto* hintArr = pitchObj->getProperty("stepHints").getArray())
        {
            for (auto& hVar : *hintArr)
            {
                auto* hObj = hVar.getDynamicObject();
                if (!hObj) continue;
                PitchStepHint h;
                h.step = (int)hObj->getProperty("step");
                h.role = parsePitchRole(hObj->getProperty("role").toString());
                pitch.stepHints.add(h);
            }
        }

        if (auto* chordArr = pitchObj->getProperty("chordSequence").getArray())
        {
            for (auto& cVar : *chordArr)
            {
                auto* cObj = cVar.getDynamicObject();
                if (!cObj) continue;
                ChordRegion cr;
                cr.barStart  = (int)cObj->getProperty("barStart");
                cr.barEnd    = (int)cObj->getProperty("barEnd");
                cr.semitones = (int)cObj->getProperty("semitones");
                pitch.chordSequence.add(cr);
            }
        }
    }

    return true;
}

juce::String GrooveTemplate::toJSON() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("version", 1);

    juce::DynamicObject::Ptr metaObj = new juce::DynamicObject();
    metaObj->setProperty("name",         name);
    metaObj->setProperty("genre",        genre);
    metaObj->setProperty("region",       region);
    metaObj->setProperty("mood",         mood);
    metaObj->setProperty("description",  description);
    metaObj->setProperty("tempoMin",     tempoMin);
    metaObj->setProperty("tempoMax",     tempoMax);
    metaObj->setProperty("swingPercent", swingPercent);
    root->setProperty("meta", juce::var(metaObj.get()));

    juce::Array<juce::var> drumsArr;
    for (auto* r : drums)
    {
        juce::DynamicObject::Ptr rowObj = new juce::DynamicObject();
        rowObj->setProperty("label", r->label);
        juce::Array<juce::var> sv, tv;
        for (int i = 0; i < 16; ++i) { sv.add(r->steps[i]); tv.add(drumTimingStr(r->timing[i])); }
        rowObj->setProperty("steps",  sv);
        rowObj->setProperty("timing", tv);
        drumsArr.add(juce::var(rowObj.get()));
    }
    root->setProperty("drums", drumsArr);

    juce::Array<juce::var> bassArr;
    for (auto* r : bass)
    {
        juce::DynamicObject::Ptr rowObj = new juce::DynamicObject();
        rowObj->setProperty("label", r->label);
        juce::Array<juce::var> sv, tv;
        for (int i = 0; i < 16; ++i) { sv.add(r->steps[i]); tv.add(bassArtStr(r->timing[i])); }
        rowObj->setProperty("steps",  sv);
        rowObj->setProperty("timing", tv);
        bassArr.add(juce::var(rowObj.get()));
    }
    root->setProperty("bass", bassArr);

    juce::Array<juce::var> locksArr;
    for (auto& lp : locks)
    {
        juce::DynamicObject::Ptr lObj = new juce::DynamicObject();
        lObj->setProperty("step",        lp.step);
        lObj->setProperty("type",        lockTypeStr(lp.type));
        lObj->setProperty("description", lp.description);
        locksArr.add(juce::var(lObj.get()));
    }
    root->setProperty("locks", locksArr);

    if (!bass2.isEmpty())
    {
        juce::Array<juce::var> bass2Arr;
        for (auto* r : bass2)
        {
            juce::DynamicObject::Ptr rowObj = new juce::DynamicObject();
            rowObj->setProperty("label", r->label);
            juce::Array<juce::var> sv, tv;
            for (int i = 0; i < 16; ++i) { sv.add(r->steps[i]); tv.add(bassArtStr(r->timing[i])); }
            rowObj->setProperty("steps",  sv);
            rowObj->setProperty("timing", tv);
            bass2Arr.add(juce::var(rowObj.get()));
        }
        root->setProperty("bass2", bass2Arr);
    }

    if (!locks2.isEmpty())
    {
        juce::Array<juce::var> locks2Arr;
        for (auto& lp : locks2)
        {
            juce::DynamicObject::Ptr lObj = new juce::DynamicObject();
            lObj->setProperty("step",        lp.step);
            lObj->setProperty("type",        lockTypeStr(lp.type));
            lObj->setProperty("description", lp.description);
            locks2Arr.add(juce::var(lObj.get()));
        }
        root->setProperty("locks2", locks2Arr);
    }

    if (pitch.hasPitchData)
    {
        static auto roleStr = [](PitchRole r) -> juce::String {
            switch (r) {
                case PitchRole::ROOT:    return "root";
                case PitchRole::SECOND:  return "2";
                case PitchRole::FLAT3:   return "b3";
                case PitchRole::FOURTH:  return "4";
                case PitchRole::FIFTH:   return "5";
                case PitchRole::FLAT5:   return "b5";
                case PitchRole::SIXTH:   return "6";
                case PitchRole::FLAT7:   return "b7";
                case PitchRole::OCTAVE:  return "octave";
                case PitchRole::APPROACH:return "approach";
                case PitchRole::ANY:     return "any";
                default:                 return "root";
            }
        };

        juce::DynamicObject::Ptr pObj = new juce::DynamicObject();
        pObj->setProperty("densityHint",            pitch.densityHint);
        pObj->setProperty("productionComplexity",   pitch.productionComplexity);
        pObj->setProperty("allowChromaticApproach", pitch.allowChromaticApproach);

        juce::Array<juce::var> prefArr;
        for (auto r : pitch.preferredIntervals) prefArr.add(roleStr(r));
        pObj->setProperty("preferredIntervals", prefArr);

        juce::Array<juce::var> hintArr;
        for (auto& h : pitch.stepHints)
        {
            juce::DynamicObject::Ptr hObj = new juce::DynamicObject();
            hObj->setProperty("step", h.step);
            hObj->setProperty("role", roleStr(h.role));
            hintArr.add(juce::var(hObj.get()));
        }
        pObj->setProperty("stepHints", hintArr);

        if (!pitch.chordSequence.isEmpty())
        {
            juce::Array<juce::var> chordArr;
            for (auto& cr : pitch.chordSequence)
            {
                juce::DynamicObject::Ptr cObj = new juce::DynamicObject();
                cObj->setProperty("barStart",  cr.barStart);
                cObj->setProperty("barEnd",    cr.barEnd);
                cObj->setProperty("semitones", cr.semitones);
                chordArr.add(juce::var(cObj.get()));
            }
            pObj->setProperty("chordSequence", chordArr);
        }

        root->setProperty("pitch", juce::var(pObj.get()));
    }

    return juce::JSON::toString(juce::var(root.get()), true);
}

std::optional<LockPoint> GrooveTemplate::lockAt(int step, int bar) const
{
    const auto& source = (bar == 1 && !locks2.isEmpty()) ? locks2 : locks;
    for (auto& lp : source)
        if (lp.step == step)
            return lp;
    return std::nullopt;
}
