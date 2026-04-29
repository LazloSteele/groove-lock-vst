cmake_minimum_required(VERSION 3.22)
project(GrooveLock VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GrooveLock
    COMPANY_NAME            "GrooveLock"
    PLUGIN_MANUFACTURER_CODE Grvl
    PLUGIN_CODE             Glck
    FORMATS                 VST3 AU Standalone
    PRODUCT_NAME            "Groove Lock"
    IS_MIDI_EFFECT          FALSE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       TRUE
    IS_SYNTH                TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES         Instrument MIDI
)

juce_generate_juce_header(GrooveLock)

target_sources(GrooveLock PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/GrooveTemplate.cpp
    src/PatternAnalyzer.cpp
    src/LockEngine.cpp
    src/PitchEngine.cpp
    src/PhraseExpander.cpp
    src/MidiOutputManager.cpp
    src/TemplateBrowser.cpp
    src/StepSequencerView.cpp
    src/LockPointView.cpp
)

target_compile_definitions(GrooveLock PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GrooveLock PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# juce_recommended_warning_flags enables /W4 on MSVC, which triggers C4244
# (implicit narrowing: double→int, int64→int) and C4267 (size_t→int) across
# JUCE's own headers. Suppress them without dropping the rest of /W4.
if(MSVC)
    target_compile_options(GrooveLock PRIVATE /wd4244 /wd4267)
endif()


# Copy presets into the VST3 bundle (bundle.vst3/presets/).
# The DLL sits two dirs deep inside the bundle: bundle.vst3/Contents/x86_64-win/
# so ../../ from TARGET_FILE_DIR reaches the bundle root.
add_custom_command(TARGET GrooveLock_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/presets"
        "$<TARGET_FILE_DIR:GrooveLock_VST3>/../../presets"
    COMMENT "Copying presets into VST3 bundle"
    VERBATIM)
