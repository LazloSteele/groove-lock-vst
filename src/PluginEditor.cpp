#include "PluginEditor.h"
#include "GenreProfile.h"

// ─── DensityTensionPad ────────────────────────────────────────────────────────

class GrooveLockEditor::DensityTensionPad : public juce::Component
{
public:
    std::function<void(float, float)> onValueChange;

    void setValues(float x, float y, bool notify = false)
    {
        xVal = juce::jlimit(0.f, 1.f, x);
        yVal = juce::jlimit(0.f, 1.f, y);
        repaint();
        if (notify && onValueChange) onValueChange(xVal, yVal);
    }

    void setGenreClamp(float xMin, float xMax, float yMin, float yMax)
    {
        cxMin = xMin; cxMax = xMax; cyMin = yMin; cyMax = yMax;
        repaint();
    }

    float getDensity() const { return xVal; }
    float getTension() const { return yVal; }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.fillAll(juce::Colour(0xff121417));

        // Experimental zone (density > 70% OR tension > 70%) — subtle warm tint
        float expXPx = b.getX() + 0.70f * b.getWidth();
        float expYPx = b.getY() + (1.f - 0.70f) * b.getHeight(); // top 30% = tension exp.
        g.setColour(juce::Colour(0x0cff9f1c));
        g.fillRect(expXPx, b.getY(), b.getRight() - expXPx, b.getHeight()); // density exp.
        g.fillRect(b.getX(), b.getY(), b.getWidth(), expYPx - b.getY());     // tension exp.
        // Dotted boundary lines
        g.setColour(juce::Colour(0x28ff9f1c));
        g.drawVerticalLine  ((int)expXPx, b.getY(),    b.getBottom());
        g.drawHorizontalLine((int)expYPx, b.getX(), b.getRight());
        // Labels
        g.setFont(juce::Font(6.5f));
        g.setColour(juce::Colour(0x50ff9f1c));
        g.drawText("Experimental", (int)expXPx + 2, b.getBottom() - 11,
                   (int)(b.getRight() - expXPx) - 4, 10, juce::Justification::centred);
        g.drawText("Experimental", b.getX() + 2, b.getY() + 2,
                   (int)b.getWidth() - 4, 10, juce::Justification::centred);

        // Genre-accessible region (focus-tinted)
        float cx1 = b.getX() + cxMin * b.getWidth();
        float cy1 = b.getY() + (1.f - cyMax) * b.getHeight();
        float cx2 = b.getX() + cxMax * b.getWidth();
        float cy2 = b.getY() + (1.f - cyMin) * b.getHeight();
        g.setColour(juce::Colour(0xff1a2030));
        g.fillRect(cx1, cy1, cx2 - cx1, cy2 - cy1);

        // Centre grid lines
        g.setColour(juce::Colour(0xff1e2226));
        g.drawLine(b.getCentreX(), b.getY(), b.getCentreX(), b.getBottom(), 0.5f);
        g.drawLine(b.getX(), b.getCentreY(), b.getRight(), b.getCentreY(), 0.5f);

        // Border
        g.setColour(juce::Colour(0xff3a4652));
        g.drawRect(b, 1.f);

        // Crosshair through cursor
        float dotX = b.getX() + xVal * b.getWidth();
        float dotY = b.getY() + (1.f - yVal) * b.getHeight();
        g.setColour(juce::Colour(0xff2a3340));
        g.drawLine(dotX, b.getY(), dotX, b.getBottom(), 0.5f);
        g.drawLine(b.getX(), dotY, b.getRight(), dotY, 0.5f);

        // Cursor dot — brighter when in experimental zone
        bool inExp = (xVal > 0.70f || yVal > 0.70f);
        g.setColour(inExp ? juce::Colour(0xffff9f1c) : juce::Colour(0xffcc7a10));
        g.fillEllipse(dotX - 5.f, dotY - 5.f, 10.f, 10.f);
        g.setColour(juce::Colours::white.withAlpha(inExp ? 0.8f : 0.45f));
        g.drawEllipse(dotX - 5.f, dotY - 5.f, 10.f, 10.f, 1.f);
    }

    void mouseDown    (const juce::MouseEvent& e) override { updateFromMouse(e); }
    void mouseDrag    (const juce::MouseEvent& e) override { updateFromMouse(e); }
    void mouseDoubleClick(const juce::MouseEvent&)  override { setValues(0.5f, 0.5f, true); }

private:
    float xVal = 0.5f, yVal = 0.5f;
    float cxMin = 0.f, cxMax = 1.f, cyMin = 0.f, cyMax = 1.f;

    void updateFromMouse(const juce::MouseEvent& e)
    {
        auto b  = getLocalBounds().toFloat();
        float nx = juce::jlimit(0.f, 1.f, (e.x - b.getX()) / b.getWidth());
        float ny = juce::jlimit(0.f, 1.f, 1.f - (e.y - b.getY()) / b.getHeight());
        setValues(nx, ny, true);
    }
};

// ─── PhraseBarIndicator ───────────────────────────────────────────────────────

class GrooveLockEditor::PhraseBarIndicator : public juce::Component
{
public:
    void setCurrentBar(int bar) { currentBar = bar & 7; repaint(); }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        int w = b.getWidth() / 8;
        for (int i = 0; i < 8; ++i)
        {
            juce::Rectangle<int> cell(b.getX() + i * w, b.getY(), w - 1, b.getHeight());
            bool active = (i == currentBar);
            g.setColour(active ? juce::Colour(0xffff9f1c) : juce::Colour(0xff3a4652));
            g.fillRoundedRectangle(cell.toFloat().reduced(1.f), 2.f);
            g.setFont(juce::Font(8.f));
            g.setColour(active ? juce::Colours::white : juce::Colour(0xff555555));
            g.drawText(juce::String(i + 1), cell, juce::Justification::centred);
        }
    }

private:
    int currentBar = 0;
};

// ─── DrumMapPanel ─────────────────────────────────────────────────────────────

class GrooveLockEditor::DrumMapPanel : public juce::Component
{
public:
    explicit DrumMapPanel(GrooveLockProcessor& p) : proc(p)
    {
        addAndMakeVisible(presetBox);
        presetBox.addItem("Custom", 1);
        int id = 2;
        for (auto& preset : DrumMappingPresets::getAll())
            presetBox.addItem(preset.name, id++);
        presetBox.setSelectedId(1, juce::dontSendNotification);
        presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1a1a));

        presetBox.onChange = [this] {
            int sel = presetBox.getSelectedId();
            if (sel <= 1) return; // Custom selected — no action
            const auto& presets = DrumMappingPresets::getAll();
            size_t idx = (size_t)(sel - 2);
            if (idx < presets.size())
            {
                proc.applyMapping(presets[idx].mapping);
                refreshNoteLabels(proc.currentMapping);
            }
        };

        static const char* kCatNames[3] = { "Kick", "Snare", "Hat" };
        for (int i = 0; i < 3; ++i)
        {
            auto& row = rows[i];
            row.catLabel.setText(kCatNames[i], juce::dontSendNotification);
            row.catLabel.setFont(juce::Font(10.f));
            row.catLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
            addAndMakeVisible(row.catLabel);

            row.notesLabel.setFont(juce::Font(9.f));
            row.notesLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
            addAndMakeVisible(row.notesLabel);

            auto makeBtn = [this](juce::TextButton& b, const juce::String& text) {
                b.setButtonText(text);
                b.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff3a4652));
                b.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffaabbcc));
                addAndMakeVisible(b);
            };
            makeBtn(row.learnBtn, "Learn");
            makeBtn(row.clearBtn, "x");

            int category = i + 1; // 1=kick 2=snare 3=hat
            row.learnBtn.onClick = [this, category] {
                // Toggle: clicking again cancels
                proc.learnState.set(proc.learnState.get() == category ? 0 : category);
            };

            row.clearBtn.onClick = [this, i] {
                auto m = proc.currentMapping;
                if      (i == 0) m.kickNotes.clear();
                else if (i == 1) m.snareNotes.clear();
                else             m.hatNotes.clear();
                proc.applyMapping(m);
                refreshNoteLabels(proc.currentMapping);
                presetBox.setSelectedId(1, juce::dontSendNotification); // Custom
            };
        }

        refreshNoteLabels(proc.currentMapping);
    }

    // Called from timerCallback when audio thread captured a learn note
    void onLearnCapture(int category, int note)
    {
        auto m = proc.currentMapping;
        switch (category)
        {
            case 1: if (!m.kickNotes.contains(note))  m.kickNotes.add(note);  break;
            case 2: if (!m.snareNotes.contains(note)) m.snareNotes.add(note); break;
            case 3: if (!m.hatNotes.contains(note))   m.hatNotes.add(note);   break;
            default: return;
        }
        proc.applyMapping(m);
        refreshNoteLabels(proc.currentMapping);
        presetBox.setSelectedId(1, juce::dontSendNotification); // Custom
    }

    // Called from timerCallback to keep Learn button state in sync
    void updateLearnButtons(int learnState)
    {
        for (int i = 0; i < 3; ++i)
        {
            bool active = (learnState == i + 1);
            rows[i].learnBtn.setColour(juce::TextButton::buttonColourId,
                active ? juce::Colour(0xffff9f1c) : juce::Colour(0xff3a4652));
            rows[i].learnBtn.setButtonText(active ? "..." : "Learn");
        }
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(3, 2);
        presetBox.setBounds(b.removeFromTop(20));
        b.removeFromTop(2);
        for (auto& row : rows)
        {
            auto r = b.removeFromTop(20);
            row.catLabel.setBounds(r.removeFromLeft(36));
            row.clearBtn.setBounds(r.removeFromRight(16));
            r.removeFromRight(2);
            row.learnBtn.setBounds(r.removeFromRight(38));
            r.removeFromRight(2);
            row.notesLabel.setBounds(r);
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xff121417));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 3.f);
        g.setColour(juce::Colour(0xff3a4652));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 3.f, 1.f);
    }

private:
    GrooveLockProcessor& proc;

    juce::ComboBox presetBox;

    struct Row
    {
        juce::Label      catLabel;
        juce::Label      notesLabel;
        juce::TextButton learnBtn;
        juce::TextButton clearBtn;
    };
    Row rows[3];

    void refreshNoteLabels(const DrumMapping& m)
    {
        rows[0].notesLabel.setText(notesToString(m.kickNotes),  juce::dontSendNotification);
        rows[1].notesLabel.setText(notesToString(m.snareNotes), juce::dontSendNotification);
        rows[2].notesLabel.setText(notesToString(m.hatNotes),   juce::dontSendNotification);
    }

    static juce::String notesToString(const juce::Array<int>& notes)
    {
        if (notes.isEmpty()) return "(none)";
        juce::StringArray names;
        for (int n : notes)
            names.add(juce::MidiMessage::getMidiNoteName(n, true, true, 4));
        return names.joinIntoString(", ");
    }
};

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

// ─── HifiKnobLookAndFeel ─────────────────────────────────────────────────────

class GrooveLockEditor::HifiKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override
    {
        const float cx     = x + width  * 0.5f;
        const float cy     = y + height * 0.5f;
        const float outerR = juce::jmin(width, height) * 0.5f - 1.f;
        const float knobR  = outerR * 0.76f;
        const float arcR   = outerR * 0.91f;
        const float arcW   = outerR * 0.095f;
        const float angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Drop shadow
        g.setColour(juce::Colour(0x60000000));
        g.fillEllipse(cx - knobR + 1.5f, cy - knobR + 2.5f, knobR * 2.f, knobR * 2.f);

        // Outer metallic rim
        {
            juce::ColourGradient rim(
                juce::Colour(0xff4e5a66), cx - knobR * 0.4f, cy - knobR * 0.4f,
                juce::Colour(0xff191d22), cx + knobR * 0.6f, cy + knobR * 0.7f,
                true);
            g.setGradientFill(rim);
            g.fillEllipse(cx - knobR, cy - knobR, knobR * 2.f, knobR * 2.f);
        }

        // Knob body — sphere gradient lit from top-left
        const float bR = knobR * 0.86f;
        {
            juce::ColourGradient body(
                juce::Colour(0xff505c68), cx - bR * 0.28f, cy - bR * 0.26f,
                juce::Colour(0xff0c0f13), cx + bR * 0.55f, cy + bR * 0.60f,
                true);
            body.addColour(0.40, juce::Colour(0xff1e252e));
            body.addColour(0.72, juce::Colour(0xff111518));
            g.setGradientFill(body);
            g.fillEllipse(cx - bR, cy - bR, bR * 2.f, bR * 2.f);
        }

        // Rim light: warm orange-tinted glow at bottom edge
        {
            juce::ColourGradient rl(
                juce::Colour(0x28ff9f4c), cx, cy + bR * 0.65f,
                juce::Colour(0x00ff9f4c), cx, cy,
                true);
            g.setGradientFill(rl);
            g.fillEllipse(cx - bR * 0.65f, cy + bR * 0.25f, bR * 1.3f, bR * 0.75f);
        }

        // Track arc (unfilled)
        {
            juce::Path bg;
            bg.addCentredArc(cx, cy, arcR, arcR, 0.f,
                             rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(juce::Colour(0xff1c222c));
            g.strokePath(bg, juce::PathStrokeType(arcW, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        }

        // Value arc (orange)
        if (angle > rotaryStartAngle + 0.01f)
        {
            juce::Path val;
            val.addCentredArc(cx, cy, arcR, arcR, 0.f,
                              rotaryStartAngle, angle, true);
            g.setColour(juce::Colour(0xffff9f1c));
            g.strokePath(val, juce::PathStrokeType(arcW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

            // Arc end cap glow
            const float capX = cx + std::sin(angle) * arcR;
            const float capY = cy - std::cos(angle) * arcR;
            g.setColour(juce::Colour(0x60ff9f1c));
            g.fillEllipse(capX - arcW, capY - arcW, arcW * 2.f, arcW * 2.f);
        }

        // Indicator dot
        const float dotDist = bR * 0.66f;
        const float dotX = cx + std::sin(angle) * dotDist;
        const float dotY = cy - std::cos(angle) * dotDist;
        g.setColour(juce::Colour(0xccffffff));
        g.fillEllipse(dotX - 3.f, dotY - 3.f, 6.f, 6.f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(dotX - 1.5f, dotY - 1.5f, 3.f, 3.f);
    }
};

// ─── Editor ───────────────────────────────────────────────────────────────────

GrooveLockEditor::GrooveLockEditor(GrooveLockProcessor& p)
    : AudioProcessorEditor(p), proc(p), hifiLnF(std::make_unique<HifiKnobLookAndFeel>())
{

    // Header
    addAndMakeVisible(titleLabel);
    titleLabel.setText("GROOVE LOCK", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff9f1c));

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

    // Groove selector dropdown — used in main view instead of full browser
    addAndMakeVisible(grooveDropdown);
    grooveDropdown.setTextWhenNothingSelected("Select a groove...");
    grooveDropdown.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1d21));
    grooveDropdown.setColour(juce::ComboBox::textColourId,       juce::Colours::white);
    grooveDropdown.setColour(juce::ComboBox::outlineColourId,    juce::Colour(0xff3a4652));
    grooveDropdown.setColour(juce::ComboBox::arrowColourId,      juce::Colour(0xffff9f1c));
    {
        auto& tb = proc.getTemplateBrowser();
        for (int i = 0; i < tb.getNumTemplates(); ++i)
        {
            auto* t = tb.getTemplate(i);
            juce::String label = t ? (t->name + "  \xe2\x80\x94  " + t->genre) : juce::String("Template " + juce::String(i));
            grooveDropdown.addItem(label, i + 1);
        }
    }
    grooveDropdown.onChange = [this] {
        int idx = grooveDropdown.getSelectedId() - 1;
        if (idx >= 0)
        {
            proc.loadTemplate(idx);
            refreshFromTemplate();
        }
    };

    // XY pad — Density (X) / Tension (Y)
    xyPad = std::make_unique<DensityTensionPad>();
    addAndMakeVisible(*xyPad);

    xyPad->setValues(proc.density.get(), proc.tension.get());
    xyPad->onValueChange = [this](float d, float t) {
        proc.density.set(d);
        proc.tension.set(t);
        proc.phraseParamsDirty.set(1);
        xyCoordLabel.setText("Dense " + juce::String(d, 2) + "  Tense " + juce::String(t, 2),
                             juce::dontSendNotification);
    };

    addAndMakeVisible(xyCoordLabel);
    xyCoordLabel.setFont(juce::Font(9.f));
    xyCoordLabel.setColour(juce::Label::textColourId, juce::Colour(0xff666666));
    xyCoordLabel.setJustificationType(juce::Justification::centred);
    xyCoordLabel.setText("D 0.50  T 0.50", juce::dontSendNotification);

    addAndMakeVisible(regenModeBox);
    regenModeBox.addItem("Fixed",    1);
    regenModeBox.addItem("Per-Loop", 2);
    regenModeBox.addItem("Manual",   3);
    regenModeBox.setSelectedId(proc.regenMode.get() + 1, juce::dontSendNotification);
    regenModeBox.onChange = [this] {
        proc.regenMode.set(regenModeBox.getSelectedId() - 1);
        regenButton.setVisible(regenModeBox.getSelectedId() == 3);
    };

    addAndMakeVisible(regenButton);
    regenButton.setVisible(proc.regenMode.get() == 2);
    regenButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a4652));
    regenButton.onClick = [this] { proc.regeneratePhrase(); };

    // Phrase bar indicator
    phraseBarIndicator = std::make_unique<PhraseBarIndicator>();
    addAndMakeVisible(*phraseBarIndicator);

    // Knobs
    setupKnob(swingKnob,      swingLabel,      "Swing",      0,   100, 55,  "%");
    setupKnob(humanizeKnob,   humanizeLabel,   "Feel",       0,   100, 20,  "%");
    setupKnob(timingOffKnob,  timingOffLabel,  "Push / Lay", -20, 20,  0,   "ms");
    setupKnob(gateScaleKnob,  gateScaleLabel,  "Length",    50,  150, 100, "%");
    setupKnob(glideKnob,      glideLabel,      "Glide",     10,  300, 100, "ms");

    swingKnob.onValueChange     = [this] { proc.swingPercent.set((float)swingKnob.getValue()); };
    humanizeKnob.onValueChange  = [this] { proc.humanizePercent.set((float)humanizeKnob.getValue()); };
    timingOffKnob.onValueChange = [this] { proc.timingOffsetMs.set((float)timingOffKnob.getValue()); };
    gateScaleKnob.onValueChange = [this] { proc.gateLengthScale.set((float)gateScaleKnob.getValue() / 100.f); };
    glideKnob.onValueChange     = [this] { proc.glideTimeMs.set((float)glideKnob.getValue()); };

    for (auto* k : { &swingKnob, &humanizeKnob, &timingOffKnob, &gateScaleKnob, &glideKnob })
        k->setLookAndFeel(hifiLnF.get());

    // I/O config
    addAndMakeVisible(inputModeToggle);
    inputModeToggle.setToggleState(proc.inputMode.get() == 0, juce::dontSendNotification);
    inputModeToggle.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xffff9f1c));
    inputModeToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff555555));
    inputModeToggle.onStateChange = [this] {
        bool live = inputModeToggle.getToggleState();
        proc.inputMode.set(live ? 0 : 1);
        if (!live) proc.learnState.set(0); // cancel any active learn on mode exit
        drumMapPanel->setVisible(live);
        resized();
    };

    drumMapPanel = std::make_unique<DrumMapPanel>(proc);
    addAndMakeVisible(*drumMapPanel);
    drumMapPanel->setVisible(proc.inputMode.get() == 0);

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
    pitchEnabledToggle.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xffff9f1c));
    pitchEnabledToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff555555));
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
    pitchDensitySlider.setRange(0, 10, 1);
    pitchDensitySlider.setValue(0, juce::dontSendNotification);
    pitchDensitySlider.setTextValueSuffix("");
    pitchDensitySlider.onValueChange = [this] {
        proc.pitchDensity.set((int)pitchDensitySlider.getValue());
    };
    pitchDensityLabel.setText("Note Count", juce::dontSendNotification);
    pitchDensityLabel.setFont(juce::Font(9.f));
    pitchDensityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff666666));

    addAndMakeVisible(pitchChromaticToggle);
    pitchChromaticToggle.setToggleState(true, juce::dontSendNotification);
    pitchChromaticToggle.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xffff9f1c));
    pitchChromaticToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff555555));
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

    // View toggle button
    addAndMakeVisible(patternToggleBtn);
    patternToggleBtn.setColour(juce::TextButton::buttonColourId,   juce::Colour(0xff1e2226));
    patternToggleBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff3a4652));
    patternToggleBtn.onClick = [this] {
        showPatternView = !showPatternView;
        applyViewMode();
    };
    applyViewMode(); // set initial visibility before first resize

    // setSize must come after all child components are constructed so that
    // resized() fires with every component already existing.
    setResizable(true, false);
    setResizeLimits(480, 520, 1600, 1200);
    setSize(560, 580);

    startTimerHz(15);
}

GrooveLockEditor::~GrooveLockEditor()
{
    for (auto* k : { &swingKnob, &humanizeKnob, &timingOffKnob, &gateScaleKnob, &glideKnob })
        k->setLookAndFeel(nullptr);
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
    k.setColour(juce::Slider::rotarySliderFillColourId,    juce::Colour(0xffff9f1c));
    k.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff3a4652));
    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(11.f));
    l.setColour(juce::Label::textColourId, juce::Colour(0xffaabbcc));
    l.setJustificationType(juce::Justification::centred);
}

void GrooveLockEditor::applyViewMode()
{
    const bool pat = showPatternView;
    patternToggleBtn.setButtonText(pat ? "Pattern  \xe2\x8a\xa0" : "Pattern");

    // Groove dropdown — only in main view; full browser used in pattern view
    grooveDropdown.setVisible(!pat);

    // Grids — only visible in pattern view
    drumView.setVisible(pat);
    lockView.setVisible(pat);
    bassView.setVisible(pat);
    infoBar.setVisible(pat);

    // Secondary knobs
    timingOffKnob.setVisible(pat);   timingOffLabel.setVisible(pat);
    gateScaleKnob.setVisible(pat);   gateScaleLabel.setVisible(pat);
    glideKnob.setVisible(pat);       glideLabel.setVisible(pat);

    // I/O section
    inputModeToggle.setVisible(pat);
    if (drumMapPanel) drumMapPanel->setVisible(pat && proc.inputMode.get() == 0);
    outputChannelBox.setVisible(pat);
    panicButton.setVisible(pat);

    // Regen mode (auto-regen still runs; dropdown is a setup control)
    regenModeBox.setVisible(pat);
    regenButton.setVisible(pat && regenModeBox.getSelectedId() == 3);

    // Advanced pitch controls
    pitchDensitySlider.setVisible(pat);
    pitchDensityLabel.setVisible(pat);
    pitchChromaticToggle.setVisible(pat);

    resized();
    repaint();
}

void GrooveLockEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121417));

    if (showPatternView)
    {
        int divX = (int)(getWidth() * 0.70f);
        g.setColour(juce::Colour(0xff1e2226));
        g.drawVerticalLine(divX, 30.0f, (float)(getHeight() - 28));
    }

    g.setColour(juce::Colour(0xff1a1d21));
    g.drawHorizontalLine(30, 0, (float)getWidth());
    g.drawHorizontalLine(getHeight() - 28, 0, (float)getWidth());
}

void GrooveLockEditor::resized()
{
    auto area = getLocalBounds();

    // Header (30px)
    auto header = area.removeFromTop(30);
    titleLabel.setBounds(header.removeFromLeft(140));
    nextButton.setBounds(header.removeFromRight(30));
    prevButton.setBounds(header.removeFromRight(30));
    presetNameLabel.setBounds(header);

    // Transport bar (28px bottom)
    auto transport = area.removeFromBottom(28);
    tempoLabel.setBounds(transport.removeFromRight(160));
    patternToggleBtn.setBounds(transport.removeFromLeft(90).reduced(2, 3));
    if (phraseBarIndicator)
        phraseBarIndicator->setBounds(transport.removeFromLeft(130).reduced(2, 4));

    if (!showPatternView)
    {
        // ── Main view: anchor bottom controls, XY pad fills centre ────────
        auto main = area.reduced(12, 4);

        // Groove selector — top
        grooveDropdown.setBounds(main.removeFromTop(30));
        main.removeFromTop(8);

        // Pitch row — flush against transport (bottom)
        {
            auto pitchRow = main.removeFromBottom(28);
            pitchEnabledToggle.setBounds(pitchRow.removeFromLeft(80));
            pitchRow.removeFromLeft(4);
            pitchRootBox.setBounds(pitchRow.removeFromLeft(68));
            pitchRow.removeFromLeft(4);
            pitchScaleBox.setBounds(pitchRow.removeFromLeft(130));
            pitchRow.removeFromLeft(8);
            octaveDownButton.setBounds(pitchRow.removeFromLeft(44));
            octaveUpButton.setBounds(pitchRow.removeFromRight(44));
            octaveDisplayLabel.setFont(juce::Font(13.f, juce::Font::bold));
            octaveDisplayLabel.setBounds(pitchRow);
        }

        // Swing + Humanize — above pitch row
        main.removeFromBottom(6);
        {
            int knobH   = 76;
            int knobW   = juce::jmin(main.getWidth() / 2, 110);
            auto knobRow = main.removeFromBottom(knobH)
                               .withSizeKeepingCentre(knobW * 2, knobH);
            auto placeKnob = [&](juce::Slider& k, juce::Label& l, juce::Rectangle<int> cell) {
                l.setBounds(cell.removeFromBottom(16));
                k.setBounds(cell);
            };
            placeKnob(swingKnob,    swingLabel,    knobRow.removeFromLeft(knobW));
            placeKnob(humanizeKnob, humanizeLabel, knobRow.removeFromLeft(knobW));
        }

        // Coord readout — above knobs
        main.removeFromBottom(4);
        xyCoordLabel.setFont(juce::Font(11.f));
        xyCoordLabel.setBounds(main.removeFromBottom(16));

        // XY pad — fills remaining centre space
        int xySize = juce::jlimit(120, 260, juce::jmin(main.getWidth(), main.getHeight()));
        int xyX    = main.getX() + (main.getWidth()  - xySize) / 2;
        int xyY    = main.getY() + (main.getHeight() - xySize) / 2;
        xyPad->setBounds(xyX, xyY, xySize, xySize);
    }
    else
    {
        // ── Pattern view: original full layout ────────────────────────────
        int W       = area.getWidth();
        int sidebar = (int)(W * 0.30f);
        auto sidebar_area = area.removeFromRight(sidebar).reduced(4);
        auto mainArea     = area.reduced(4, 2);

        // Sidebar
        {
            auto s = sidebar_area;
            searchBox.setBounds(s.removeFromTop(24));
            genreFilter.setBounds(s.removeFromTop(24));
            int listH = juce::jmax(80, (int)(s.getHeight() * 0.13f));
            templateList.setBounds(s.removeFromTop(listH));

            s.removeFromTop(4);

            int xySize = juce::jmin(s.getWidth(), 120);
            xyPad->setBounds(s.removeFromTop(xySize));
            xyCoordLabel.setBounds(s.removeFromTop(14));
            {
                auto regenRow = s.removeFromTop(22);
                regenModeBox.setBounds(regenButton.isVisible()
                                        ? regenRow.removeFromLeft(regenRow.getWidth() - 54)
                                        : regenRow);
                if (regenButton.isVisible())
                    regenButton.setBounds(regenRow);
            }

            s.removeFromTop(4);

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
            place(timingOffKnob, timingOffLabel, knobRow1.removeFromLeft(knobW));
            place(gateScaleKnob, gateScaleLabel, knobRow2.removeFromLeft(knobW));
            place(glideKnob,     glideLabel,     knobRow2.removeFromLeft(knobW));

            s.removeFromTop(6);
            inputModeToggle.setBounds(s.removeFromTop(22));
            if (drumMapPanel && drumMapPanel->isVisible())
            {
                s.removeFromTop(2);
                drumMapPanel->setBounds(s.removeFromTop(86));
                s.removeFromTop(2);
            }
            outputChannelBox.setBounds(s.removeFromTop(22));
            s.removeFromTop(4);
            panicButton.setBounds(s.removeFromTop(26));

            s.removeFromTop(6);

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
            int drumH = 22 * 3 + 14;
            int lockH = 20;
            int infoH = 36;
            int bassH = m.getHeight() - drumH - lockH - infoH - 8;

            drumView.setBounds(m.removeFromTop(drumH));
            m.removeFromTop(2);
            lockView.setBounds(m.removeFromTop(lockH));
            m.removeFromTop(2);
            bassView.setBounds(m.removeFromTop(bassH));
            m.removeFromTop(2);
            infoBar.setBounds(m.removeFromTop(infoH));
        }
    }
}

void GrooveLockEditor::timerCallback()
{
    int step = proc.currentStep.get();
    drumView.setCurrentStep(step);
    bassView.setCurrentStep(step);
    lockView.setCurrentStep(step);

    // Switch drum grid between live input and template display
    if (proc.inputMode.get() == 0)
        drumView.setLiveDrumState(&proc.liveDrumDisplay);
    else
        drumView.setLiveDrumState(nullptr);

    // Poll for MIDI learn capture and update learn button appearance
    if (drumMapPanel && drumMapPanel->isVisible())
    {
        if (proc.learnCaptureReady.compareAndSetBool(0, 1))
            drumMapPanel->onLearnCapture(proc.learnCapturedCategory.get(),
                                         proc.learnCapturedNote.get());
        drumMapPanel->updateLearnButtons(proc.learnState.get());
    }

    phraseBarIndicator->setCurrentBar(proc.currentPhraseBar.get());

    // Regenerate phrase when parameters changed or per-loop boundary was crossed
    if (proc.phraseParamsDirty.compareAndSetBool(0, 1))
        proc.regeneratePhrase();
    else if (proc.regenMode.get() == 1 && proc.needsRegen.compareAndSetBool(0, 1))
        proc.regeneratePhrase();
}

void GrooveLockEditor::refreshFromTemplate()
{
    auto* t = proc.getCurrentTemplate();
    if (!t) return;

    presetNameLabel.setText(t->name + "  |  " + t->genre + "  |  " + t->mood,
                            juce::dontSendNotification);
    grooveDropdown.setSelectedId(proc.templateIndex.get() + 1, juce::dontSendNotification);
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

    // Update XY pad clamp region to reflect genre's accessible range
    xyPad->setGenreClamp(profile.densityClampMin, profile.densityClampMax,
                         profile.tensionClampMin, profile.tensionClampMax);

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
