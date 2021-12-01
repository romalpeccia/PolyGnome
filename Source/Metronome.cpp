/*
  ==============================================================================

    Metronome.cpp
    Created: 30 Nov 2021 12:18:05pm
    Author:  Romal
    d
  ==============================================================================
*/

#include "Metronome.h"
#include <JuceHeader.h>

Metronome::Metronome()
{


    //load up click file
    //TODO multiple files
        formatManager.registerBasicFormats();
        juce::String dir;

        //TODO don't hardcode this filepath
        juce::File clickFile("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot.wav");
        if (!clickFile.exists())
        {
            jassert("file not found");
        }
        auto formatReader = formatManager.createReaderFor(clickFile);
        pMetronomeSample.reset(new juce::AudioFormatReaderSource(formatReader, true));
    //end load up click file
}


void Metronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
//preparetoplay should call every time we stop
{
    sampleRate = _sampleRate;
    beatInterval = (60.0 / bpm) * sampleRate;
    

    if (pMetronomeSample != nullptr)
    {
        pMetronomeSample->prepareToPlay(samplesPerBlock, sampleRate);
       // DBG("file loaded");
    }


}

void Metronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
{
    //TODO fix this entire function, seems hacky 
    
    auto temp = juce::AudioSourceChannelInfo(buffer);
    //temp.buffer->makeCopyOf(buffer);


    auto bufferSize = buffer.getNumSamples();

    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;


    if (samplesProcessed + bufferSize >= beatInterval)

    {
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        pMetronomeSample->setNextReadPosition(0); //reset sample to beginning

        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        {
            if (samplenum == timeToStartPlaying)
            {
                pMetronomeSample->getNextAudioBlock(temp);
            }
        }
    }
    if (pMetronomeSample->getNextReadPosition() != 0)           //if we already started playing and got interupted
    {
        pMetronomeSample->getNextAudioBlock(temp);
    }
}

void Metronome::reset() 
{

    totalSamples = 0;
}



