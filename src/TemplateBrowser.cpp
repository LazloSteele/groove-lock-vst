#include "TemplateBrowser.h"

TemplateBrowser::TemplateBrowser() {}

void TemplateBrowser::loadFromDirectory(const juce::File& presetsDir)
{
    templates.clear();
    juce::Array<juce::File> jsonFiles;
    presetsDir.findChildFiles(jsonFiles, juce::File::findFiles, true, "*.json");
    jsonFiles.sort();

    for (auto& f : jsonFiles)
    {
        auto* t = templates.add(new GrooveTemplate());
        if (!t->loadFromJSON(f))
            templates.removeLast();
    }
}

const GrooveTemplate* TemplateBrowser::getTemplate(int index) const
{
    if (index < 0 || index >= templates.size()) return nullptr;
    return templates[index];
}

juce::Array<int> TemplateBrowser::filter(const juce::String& searchText,
                                          const juce::String& genre,
                                          const juce::String& region) const
{
    juce::Array<int> result;
    for (int i = 0; i < templates.size(); ++i)
    {
        auto* t = templates[i];
        if (!genre.isEmpty() && !t->genre.equalsIgnoreCase(genre)) continue;
        if (!region.isEmpty() && !t->region.equalsIgnoreCase(region)) continue;
        if (!searchText.isEmpty())
        {
            auto lower = searchText.toLowerCase();
            if (!t->name.toLowerCase().contains(lower) &&
                !t->mood.toLowerCase().contains(lower) &&
                !t->description.toLowerCase().contains(lower))
                continue;
        }
        result.add(i);
    }
    return result;
}
