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
        /*
        juce::String workingDir, fileDir;
        workingDir = juce::File::getCurrentWorkingDirectory().getFullPathName();
        
        fileDir = workingDir + "/Samples/rimshot.wav"; 
        juce::File clickFile(fileDir);
        */

        //TODO don't hardcode this filepath
        //TODO write file loader
        juce::File clickFile("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot_low.wav");
        if (!clickFile.exists())
        {
            jassert("file not found");
        }
        auto formatReader = formatManager.createReaderFor(clickFile);
        rimShotLow.reset(new juce::AudioFormatReaderSource(formatReader, true));

        /*
        juce::File clickFile2("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples/rimshot_high.wav");
        if (!clickFile2.exists())
        {
            jassert("file not found");
        }
        auto formatReader2 = formatManager.createReaderFor(clickFile2);
        rimShotHigh.reset(new juce::AudioFormatReaderSource(formatReader2, true));
        */
    //end load up click file
}


void Metronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
//preparetoplay should call every time we stop
{
    sampleRate = _sampleRate;
    beatInterval = (60.0 / bpm) * sampleRate;
    subInterval = (60 / bpm) * beatInterval;

    if (rimShotLow != nullptr)
    {
        rimShotLow->prepareToPlay(samplesPerBlock, sampleRate);
       // DBG("file loaded");
    }
    if (rimShotHigh != nullptr)
    {
        rimShotHigh->prepareToPlay(samplesPerBlock, sampleRate);
    }


}

void Metronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
{
    //TODO fix this entire function, seems hacky 
    
    //temp because <juce::AudioFormatReaderSource>->getNextAudioBlock expects an AudioSourceChannelInfoObject
    auto temp = juce::AudioSourceChannelInfo(buffer);

    auto bufferSize = buffer.getNumSamples();



    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;

    if (samplesProcessed + bufferSize >= beatInterval)

    {
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        rimShotLow->setNextReadPosition(0); //reset sample to beginning

        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        {
            if (samplenum == timeToStartPlaying)
            {
                rimShotLow->getNextAudioBlock(temp);
            }
        }
    }
    else if (samplesProcessed + bufferSize >= subInterval)
    {
        // make this if statement first?
        // make this a function?
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        rimShotHigh->setNextReadPosition(0); //reset sample to beginning

        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        {
            if (samplenum == timeToStartPlaying)
            {
                rimShotHigh->getNextAudioBlock(temp);
            }
        }

    }

 //if we already started playing and got interupted
    if (rimShotLow->getNextReadPosition() != 0)          
    {
        rimShotLow->getNextAudioBlock(temp);
    }
    else if (rimShotHigh->getNextReadPosition() != 0)
    {
        rimShotHigh->getNextAudioBlock(temp);
    }
}



void Metronome::reset() 
{

    totalSamples = 0;
}



