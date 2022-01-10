/*
  ==============================================================================

    PolyRhythmMetronome.cpp
    Created: 6 Jan 2022 3:59:04pm
    Author:  Romal

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PolyRhythmMetronome.h"

//==============================================================================
PolyRhythmMetronome::PolyRhythmMetronome()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

PolyRhythmMetronome::PolyRhythmMetronome(juce::AudioProcessorValueTreeState* _apvts)
{
    apvts = _apvts;
    resetall();
    formatManager.registerBasicFormats();


    //TODO figure out how to use relative paths in JUCE
    juce::File clickFile("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot_low.wav");
    jassert(clickFile.exists());
    auto formatReader = formatManager.createReaderFor(clickFile);
    rimShotLow.reset(new juce::AudioFormatReaderSource(formatReader, true));


    juce::File clickFile2("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot_high.wav");
    jassert(clickFile2.exists());
    auto formatReader2 = formatManager.createReaderFor(clickFile2);
    rimShotHigh.reset(new juce::AudioFormatReaderSource(formatReader2, true));

    juce::File clickFile3("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot_sub.wav");
    jassert(clickFile3.exists());
    auto formatReader3 = formatManager.createReaderFor(clickFile3);
    rimShotSub.reset(new juce::AudioFormatReaderSource(formatReader3, true));

}

PolyRhythmMetronome::~PolyRhythmMetronome()
{
}



void PolyRhythmMetronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
{
    resetparams();
    auto audiosourcechannelinfo = juce::AudioSourceChannelInfo(buffer);
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;
    subSamplesProcessed = totalSamples % subInterval;


    bool beatflag = (samplesProcessed + bufferSize >= beatInterval);
    bool subflag = (subSamplesProcessed + bufferSize >= subInterval);
   


    if (rhythm1Counter >= numerator)
    {
        rhythm1Counter = 0;
        totalSamples = 0;
    }
    if (rhythm2Counter >= subdivisions)
    {
        rhythm2Counter = 0;
        totalSamples = 0;
    }

    //TODO make this if statement less ugly
    if ( ((beatflag && subflag )) && numerator > 1 && subdivisions > 1)
    { //edge case, both beats hit at the same time , play a unique tick for that 
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        DBG("HIGH");
        rimShotHigh->setNextReadPosition(0); //reset sample to beginning
        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        {
            if (samplenum == timeToStartPlaying)
            {
                rimShotHigh->getNextAudioBlock(audiosourcechannelinfo);
            }
        }
        rhythm1Counter += 1;
        rhythm2Counter += 1;
    }
    else if (beatflag && (numerator > 1))
    {
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        DBG("LOW");
        rimShotLow->setNextReadPosition(0); //reset sample to beginning
        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        {
            if (samplenum == timeToStartPlaying)
            {
                rimShotLow->getNextAudioBlock(audiosourcechannelinfo);
            }
        }
        rhythm1Counter += 1;
    }
    else if (subflag && (subdivisions > 1))
    {
        const auto timeToStartPlaying = subInterval - subSamplesProcessed;
        DBG("SUB" );
        rimShotSub->setNextReadPosition(0); //reset sample to beginning
        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        { //TODO this loop seems weird, why is it a loop? double check tutorial
            if (samplenum == timeToStartPlaying)
            {
                rimShotSub->getNextAudioBlock(audiosourcechannelinfo);
            }
        }
        rhythm2Counter += 1;

    }

}
/*
void PolyRhythmMetronome::resetall()
{   //this should be called whenever the metronome is stopped
    resetparams();
    totalSamples = 0;
    rhythm1Counter = 0;
    rhythm2Counter = 0;
    samplesProcessed = 0;
    subSamplesProcessed = 0;
}


void PolyRhythmMetronome::resetparams()
{  //this should be called whenever the processor changes a parameter (which should only happen when the user interacts with the GUI)
    numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = 4* ((60.0 / bpm) * sampleRate)/numerator;
    subInterval = 4* ((60.0 / bpm) * sampleRate)/subdivisions;
    ///TODO (maybe) assumes 4/4 time, maybe make time sig a parameter to advance this

}
*/


void PolyRhythmMetronome::resetall()
{   //this should be called whenever the metronome is stopped
    resetparams();
    totalSamples = 0;
    rhythm1Counter = 0;
    rhythm2Counter = 0;
    samplesProcessed = 0;
    subSamplesProcessed = 0;
}


void PolyRhythmMetronome::resetparams()
{  //this should being called constantly, should do something when params change in UI to reflect changes in logic

    int temp_numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    if (numerator != temp_numerator)
    {
        numerator = temp_numerator;
        resetall();
    }
    int temp_subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    if (subdivisions != temp_subdivisions)
    {
        subdivisions = temp_subdivisions;
        resetall();
    }
    float temp_bpm = apvts->getRawParameterValue("BPM")->load();
   /*
    if (1)//(bpm != temp_bpm)
    {
        bpm = temp_bpm;
        beatInterval = 4.f * ((60.f / bpm) * sampleRate) / float(numerator);
        subInterval = 4.f * ((60.f / bpm) * sampleRate) / float(subdivisions);
        resetall();
    }*/

    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = 4 * ((60.0 / bpm) * sampleRate) / numerator;
    subInterval = 4 * ((60.0 / bpm) * sampleRate) / subdivisions;
    ///TODO (maybe) assumes 4/4 time, maybe make time sig a parameter to advance this

}
