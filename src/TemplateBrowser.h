#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"

class TemplateBrowser
{
public:
    TemplateBrowser();

    void loadFromDirectory(const juce::File& presetsDir);

    int                  getNumTemplates() const { return templates.size(); }
    const GrooveTemplate* getTemplate(int index) const;

    // Returns indices matching the filter
    juce::Array<int> filter(const juce::String& searchText,
                            const juce::String& genre,
                            const juce::String& region) const;

private:
    juce::OwnedArray<GrooveTemplate> templates;
};
