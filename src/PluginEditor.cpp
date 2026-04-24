#include "PluginEditor.h"
#include "GenreProfile.h"

// ─── ListBox model ────────────────────────────────────────────────────────────

class GrooveLockEditor::TemplateListModel : public juce::ListBoxModel
{
public:
    GrooveLockEditor& editor;
    juce::Array<int>  indices;

    explicit TemplateListModel(GrooveLockEditor& e) : editor(e) {}

    int getNumRows() override { return indices.size(); }

    void paintListBoxItem(int row, juce::Graphics& g,
                          int w, int h, bool selected) override
    {
        if (row < 0 || row >= indices.size()) return;
        auto* t = editor.proc.getTemplateBrowser().getTemplate(indices[row]);
        if (!t) return;

        juce::Colour genreCol = genreColor(t->genre);
        if (selected) g.fillAll(genreCol.withAlpha(0.15f));
        else          g.fillAll(juce::Colour(0xff181818));

        g.setFont(juce::Font(14.f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText(t->name, 8, 2, w - 16, 18, juce::Justification::left);

        g.setFont(10.f);
        g.setColour(juce::Colour(0xff888888));
        juce::String sub = t->region + " — " + juce::String((int)t->tempoMin)
                         + "–" + juce::String((int)t->tempoMax) + " BPM  " + t->mood;
        g.drawText(sub, 8, 20, w - 16, 14, juce::Justification::left);

        g.setColour(genreCol.withAlpha(0.7f));
        g.fillRoundedRectangle((float)(w - 60), 6.f, 54.f, 14.f, 4.f);
        g.setFont(9.f);
        g.setColour(juce::Colours::black);
        g.drawText(t->genre, w - 60, 6, 54, 14, juce::Justification::centred);

        if (selected)
        {
            g.setColour(genreCol);
            g.drawRect(0, 0, w, h, 1);
        }
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (row < 0 || row >= indices.size()) return;
        editor.proc.loadTemplate(indices[row]);
        editor.refreshFromTemplate();
    }

private:
    static juce::Colour genreColor(const juce::String& genre)
    {
        if (genre == "G-Funk")             return juce::Colour(0xff4a9eff);
        if (genre == "Mobb")               return juce::Colour(0xffff5555);
        if (genre == "Hyphy")              return juce::Colour(0xffffaa22);
        if (genre == "Wonky")              return juce::Colour(0xffaa77ff);
        if (genre == "Modern West Coast")  return juce::Colour(0xff44cc88);
        return juce::Colours::grey;
    }
};

// ─── Editor ───────────────────────────────────────────────────────────────────

GrooveLockEditor::GrooveLockEditor(GrooveLockProcessor& p)
    : AudioProcessorEditor(p), proc(p)
{
    setSize(780, 560);
    setResizable(true, false);
    setResizeLimits(640, 480, 1600, 1000);

    // Header
    addAndMakeVisible(titleLabel);
    titleLabel.setText("GROOVE LOCK", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6622));

    addAndMakeVisible(presetNameLabel);
    presetNameLabel.setFont(juce::Font(13.f));
    presetNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(prevButton);
    addAndMakeVisible(nextButton);

    prevButton.onClick = [this] {
        int idx = juce::jmax(0, proc.templateIndex.get() - 1);
        proc.loadTemplate(idx);
        refreshFromTemplate();
    };
    nextButton.onClick = [this] {
        int idx = juce::jmin(proc.getTemplateBrowser().getNumTemplates() - 1,
                             proc.templateIndex.get() + 1);
        proc.loadTemplate(idx);
        refreshFromTemplate();
    };

    // Grids
    addAndMakeVisible(drumView);
    addAndMakeVisible(lockView);
    addAndMakeVisible(bassView);

    lockView.onLockClicked = [this](int step) {
        auto* t = proc.getCurrentTemplate();
        if (!t) return;
        auto lp = t->lockAt(step);
        if (lp) infoBar.setText("[Step " + juce::String(step + 1) + "] " + lp->description,
                                juce::dontSendNotification);
    };

    addAndMakeVisible(infoBar);
    infoBar.setFont(juce::Font(11.f));
    infoBar.setColour(juce::Label::textColourId, juce::Colour(0xff888888));

    // Template browser
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colour(0xff555555));
    searchBox.onTextChange = [this] { rebuildTemplateList(); };

    addAndMakeVisible(genreFilter);
    genreFilter.addItem("All Genres",       1);
    genreFilter.addItem("G-Funk",           2);
    genreFilter.addItem("Mobb",             3);
    genreFilter.addItem("Hyphy",            4);
    genreFilter.addItem("Wonky",            5);
    genreFilter.addItem("Modern West Coast",6);
    genreFilter.setSelectedId(1, juce::dontSendNotification);
    genreFilter.onChange = [this] { rebuildTemplateList(); };

    listModel = std::make_unique<TemplateListModel>(*this);
    addAndMakeVisible(templateList);
    templateList.setModel(listModel.get());
    templateList.setRowHeight(40);
    templateList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff141414));

    // Knobs
    setupKnob(swingKnob,      swingLabel,      "Swing",    0,   100, 55,  "%");
    setupKnob(humanizeKnob,   humanizeLabel,   "Humanize", 0,   100, 20,  "%");
    setupKnob(velOffKnob,     velOffLabel,     "Vel Off",  -64, 64,  0,   "");
    setupKnob(timingOffKnob,  timingOffLabel,  "Timing",   -20, 20,  0,   "ms");
    setupKnob(gateScaleKnob,  gateScaleLabel,  "Gate",     50,  150, 100, "%");
    setupKnob(glideKnob,      glideLabel,      "Glide",    10,  300, 100, "ms");

    swingKnob.onValueChange     = [this] { proc.swingPercent.set((float)swingKnob.getValue()); };
    humanizeKnob.onValueChange  = [this] { proc.humanizePercent.set((float)humanizeKnob.getValue()); };
    velOffKnob.onValueChange    = [this] { proc.velOffset.set((float)velOffKnob.getValue()); };
    timingOffKnob.onValueChange = [this] { proc.timingOffsetMs.set((float)timingOffKnob.getValue()); };
    gateScaleKnob.onValueChange = [this] { proc.gateLengthScale.set((float)gateScaleKnob.getValue() / 100.f); };
    glideKnob.onValueChange     = [this] { proc.glideTimeMs.set((float)glideKnob.getValue()); };

    // I/O config
    addAndMakeVisible(inputModeToggle);
    inputModeToggle.setToggleState(proc.inputMode.get() == 0, juce::dontSendNotification);
    inputModeToggle.onStateChange = [this] {
        proc.inputMode.set(inputModeToggle.getToggleState() ? 0 : 1);
    };

    addAndMakeVisible(outputChannelBox);
    for (int i = 1; i <= 16; ++i)
        outputChannelBox.addItem("Ch " + juce::String(i), i);
    outputChannelBox.setSelectedId(proc.outputChannel.get(), juce::dontSendNotification);
    outputChannelBox.onChange = [this] {
        proc.outputChannel.set(outputChannelBox.getSelectedId());
    };

    addAndMakeVisible(rootNoteBox);
    static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    for (int i = 0; i < 128; ++i)
        rootNoteBox.addItem(juce::String(noteNames[i % 12]) + juce::String(i / 12 - 1), i + 1);
    rootNoteBox.setSelectedId(proc.outputRootNote.get() + 1, juce::dontSendNotification);
    rootNoteBox.onChange = [this] {
        proc.outputRootNote.set(rootNoteBox.getSelectedId() - 1);
    };

    addAndMakeVisible(panicButton);
    panicButton.onClick = [this] { proc.sendPanic(); };
    panicButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffaa2200));

    // Pitch panel
    addAndMakeVisible(pitchEnabledToggle);
    pitchEnabledToggle.setToggleState(false, juce::dontSendNotification);
    pitchEnabledToggle.onStateChange = [this] {
        proc.pitchEnabled.set(pitchEnabledToggle.getToggleState() ? 1 : 0);
    };

    addAndMakeVisible(pitchRootBox);
    for (int i = 0; i < 12; ++i)
        pitchRootBox.addItem(noteNames[i], i + 1);
    pitchRootBox.setSelectedId(1, juce::dontSendNotification); // C
    pitchRootBox.onChange = [this] {
        int noteClass = pitchRootBox.getSelectedId() - 1;
        proc.outputRootNote.set(noteClass + (currentOctave + 1) * 12);
        octaveDisplayLabel.setText(juce::String(noteNames[noteClass]) +
                                   juce::String(currentOctave), juce::dontSendNotification);
    };

    addAndMakeVisible(pitchScaleBox);
    pitchScaleBox.addItem("Minor Pentatonic", 1);
    pitchScaleBox.addItem("Natural Minor",    2);
    pitchScaleBox.addItem("Dorian",           3);
    pitchScaleBox.addItem("Blues",            4);
    pitchScaleBox.addItem("Phrygian",         5);
    pitchScaleBox.addItem("Chromatic",        6);
    pitchScaleBox.setSelectedId(1, juce::dontSendNotification);
    pitchScaleBox.onChange = [this] {
        proc.pitchScale.set(pitchScaleBox.getSelectedId() - 1);
    };

    addAndMakeVisible(pitchDensitySlider);
    addAndMakeVisible(pitchDensityLabel);
    pitchDensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchDensitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 24, 18);
    pitchDensitySlider.setRange(0, 5, 1);
    pitchDensitySlider.setValue(0, juce::dontSendNotification);
    pitchDensitySlider.setTextValueSuffix("");
    pitchDensitySlider.onValueChange = [this] {
        proc.pitchDensity.set((int)pitchDensitySlider.getValue());
    };
    pitchDensityLabel.setText("Density", juce::dontSendNotification);
    pitchDensityLabel.setFont(juce::Font(9.f));
    pitchDensityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff666666));

    addAndMakeVisible(pitchChromaticToggle);
    pitchChromaticToggle.setToggleState(true, juce::dontSendNotification);
    pitchChromaticToggle.onStateChange = [this] {
        proc.pitchChromatic.set(pitchChromaticToggle.getToggleState() ? 1 : 0);
    };

    addAndMakeVisible(octaveDownButton);
    addAndMakeVisible(octaveUpButton);
    addAndMakeVisible(octaveDisplayLabel);
    octaveDisplayLabel.setText("C2", juce::dontSendNotification);
    octaveDisplayLabel.setJustificationType(juce::Justification::centred);
    octaveDisplayLabel.setFont(juce::Font(12.f, juce::Font::bold));
    octaveDisplayLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    octaveDownButton.onClick = [this] {
        currentOctave = juce::jmax(0, currentOctave - 1);
        int noteClass = pitchRootBox.getSelectedId() - 1;
        proc.outputRootNote.set(noteClass + (currentOctave + 1) * 12);
        static const char* nn[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        octaveDisplayLabel.setText(juce::String(nn[noteClass]) +
                                   juce::String(currentOctave), juce::dontSendNotification);
    };
    octaveUpButton.onClick = [this] {
        currentOctave = juce::jmin(7, currentOctave + 1);
        int noteClass = pitchRootBox.getSelectedId() - 1;
        proc.outputRootNote.set(noteClass + (currentOctave + 1) * 12);
        static const char* nn[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        octaveDisplayLabel.setText(juce::String(nn[noteClass]) +
                                   juce::String(currentOctave), juce::dontSendNotification);
    };

    // Transport
    addAndMakeVisible(tempoLabel);
    tempoLabel.setFont(juce::Font(11.f));
    tempoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));

    rebuildTemplateList();
    refreshFromTemplate();

    startTimerHz(15);
}

GrooveLockEditor::~GrooveLockEditor()
{
    stopTimer();
    templateList.setModel(nullptr);
}

void GrooveLockEditor::setupKnob(juce::Slider& k, juce::Label& l,
                                  const juce::String& name,
                                  double lo, double hi, double def,
                                  const juce::String& suffix)
{
    addAndMakeVisible(k);
    addAndMakeVisible(l);
    k.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    k.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    k.setRange(lo, hi);
    k.setValue(def, juce::dontSendNotification);
    if (suffix.isNotEmpty()) k.setTextValueSuffix(suffix);
    k.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff6622));
    k.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(9.f));
    l.setColour(juce::Label::textColourId, juce::Colour(0xff666666));
    l.setJustificationType(juce::Justification::centred);
}

void GrooveLockEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0a0a0a));

    // Divider lines
    int sidebarX = (int)(getWidth() * 0.70f);
    g.setColour(juce::Colour(0xff222222));
    g.drawVerticalLine(sidebarX, 30.0f, (float)(getHeight() - 28));

    g.setColour(juce::Colour(0xff1a1a1a));
    g.drawHorizontalLine(30, 0, (float)getWidth());
    g.drawHorizontalLine(getHeight() - 28, 0, (float)getWidth());
}

void GrooveLockEditor::resized()
{
    auto area  = getLocalBounds();
    int  W     = area.getWidth();
    int  H     = area.getHeight();
    int  sidebar = (int)(W * 0.30f);
    int  mainW   = W - sidebar;

    // Header (30px)
    auto header = area.removeFromTop(30);
    titleLabel.setBounds(header.removeFromLeft(140));
    nextButton.setBounds(header.removeFromRight(30));
    prevButton.setBounds(header.removeFromRight(30));
    presetNameLabel.setBounds(header);

    // Transport bar (28px bottom)
    auto transport = area.removeFromBottom(28);
    tempoLabel.setBounds(transport.removeFromRight(160));

    // Main + sidebar
    auto sidebar_area = area.removeFromRight(sidebar).reduced(4);
    auto mainArea     = area.reduced(4, 2);

    // Sidebar layout
    {
        auto s = sidebar_area;
        // Browser header
        searchBox.setBounds(s.removeFromTop(24));
        genreFilter.setBounds(s.removeFromTop(24));
        int listH = (int)(s.getHeight() * 0.45f);
        templateList.setBounds(s.removeFromTop(listH));

        s.removeFromTop(6);

        // Global controls (2×3 grid of knobs)
        int knobW = s.getWidth() / 3;
        int knobH = 60;
        auto knobRow1 = s.removeFromTop(knobH);
        auto knobRow2 = s.removeFromTop(knobH);

        auto place = [&](juce::Slider& k, juce::Label& l, juce::Rectangle<int> cell) {
            l.setBounds(cell.removeFromBottom(14));
            k.setBounds(cell);
        };
        place(swingKnob,     swingLabel,     knobRow1.removeFromLeft(knobW));
        place(humanizeKnob,  humanizeLabel,  knobRow1.removeFromLeft(knobW));
        place(velOffKnob,    velOffLabel,    knobRow1.removeFromLeft(knobW));
        place(timingOffKnob, timingOffLabel, knobRow2.removeFromLeft(knobW));
        place(gateScaleKnob, gateScaleLabel, knobRow2.removeFromLeft(knobW));
        place(glideKnob,     glideLabel,     knobRow2.removeFromLeft(knobW));

        s.removeFromTop(6);
        inputModeToggle.setBounds(s.removeFromTop(22));
        outputChannelBox.setBounds(s.removeFromTop(22));
        rootNoteBox.setBounds(s.removeFromTop(22));
        s.removeFromTop(4);
        panicButton.setBounds(s.removeFromTop(26));

        s.removeFromTop(6);

        // Pitch panel
        pitchEnabledToggle.setBounds(s.removeFromTop(22));
        pitchRootBox.setBounds(s.removeFromTop(22));
        pitchScaleBox.setBounds(s.removeFromTop(22));
        {
            auto densRow = s.removeFromTop(22);
            pitchDensityLabel.setBounds(densRow.removeFromLeft(50));
            pitchDensitySlider.setBounds(densRow);
        }
        pitchChromaticToggle.setBounds(s.removeFromTop(22));
        {
            auto octRow = s.removeFromTop(26);
            octaveDownButton.setBounds(octRow.removeFromLeft(40));
            octaveUpButton.setBounds(octRow.removeFromRight(40));
            octaveDisplayLabel.setBounds(octRow);
        }
    }

    // Main area: drum / lock / bass / info
    {
        auto m = mainArea;
        int drumH    = 22 * 3 + 14; // ~3 rows + step numbers
        int lockH    = 20;
        int infoH    = 36;
        int bassH    = m.getHeight() - drumH - lockH - infoH - 8;

        drumView.setBounds(m.removeFromTop(drumH));
        m.removeFromTop(2);
        lockView.setBounds(m.removeFromTop(lockH));
        m.removeFromTop(2);
        bassView.setBounds(m.removeFromTop(bassH));
        m.removeFromTop(2);
        infoBar.setBounds(m.removeFromTop(infoH));
    }
}

void GrooveLockEditor::timerCallback()
{
    int step = proc.currentStep.get();
    drumView.setCurrentStep(step);
    bassView.setCurrentStep(step);
    lockView.setCurrentStep(step);
}

void GrooveLockEditor::refreshFromTemplate()
{
    auto* t = proc.getCurrentTemplate();
    if (!t) return;

    presetNameLabel.setText(t->name + "  |  " + t->genre + "  |  " + t->mood,
                            juce::dontSendNotification);
    infoBar.setText(t->description, juce::dontSendNotification);

    drumView.setDrumRows(&t->drums);
    drumView.setBassRows(nullptr);
    bassView.setBassRows(&t->bass);
    bassView.setDrumRows(nullptr);
    lockView.setLocks(&t->locks);

    // Auto-update scale to genre default (user can override after)
    auto profile = GenreProfile::forGenre(t->genre);
    int scaleId = (int)profile.defaultScale + 1;
    pitchScaleBox.setSelectedId(scaleId, juce::sendNotification);

    // Sync density hint from template if present
    if (t->pitch.hasPitchData)
        pitchDensitySlider.setValue(t->pitch.densityHint, juce::sendNotification);
}

void GrooveLockEditor::rebuildTemplateList()
{
    juce::String genre = genreFilter.getSelectedId() == 1 ? "" :
                         genreFilter.getText();
    auto indices = proc.getTemplateBrowser().filter(searchBox.getText(), genre, {});
    listModel->indices = indices;
    templateList.updateContent();
    templateList.repaint();
}
