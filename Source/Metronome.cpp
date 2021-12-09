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
}

Metronome::Metronome(juce::AudioProcessorValueTreeState* _apvts)
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


void Metronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
{
//preparetoplay should call every time we start (right before)

    resetparams();

    if (sampleRate != _sampleRate)
    {
        //if the audioprocessors samplerate hasn't changed, nothing else needs to be done
        sampleRate = _sampleRate;
        if (rimShotLow != nullptr)
        {
            rimShotLow->prepareToPlay(samplesPerBlock, sampleRate);
        }
        if (rimShotHigh != nullptr)
        {
            rimShotHigh->prepareToPlay(samplesPerBlock, sampleRate);
        }
        if (rimShotSub != nullptr)
        {
            rimShotSub->prepareToPlay(samplesPerBlock, sampleRate);
        }
    }

}

void Metronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
{
 //TODO cache calculations for less processing
   
    resetparams();

    //TODO fix bug instead of this bandaid for sync issues
    if (beatflag > subdivisions)
        beatflag = subdivisions;

    //temp wrapper because <juce::AudioFormatReaderSource>->getNextAudioBlock expects an AudioSourceChannelInfoObject
    auto audiosourcechannelinfo = juce::AudioSourceChannelInfo(buffer);

    auto bufferSize = buffer.getNumSamples();

    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;
    auto subSamplesProcessed = totalSamples % subInterval;

    
     if (subdivisions > 1 && subSamplesProcessed + bufferSize >= subInterval && beatflag != subdivisions)
     {// subdivision logic
        const auto timeToStartPlaying = subInterval - subSamplesProcessed;
        DBG("SUB" << beatflag);
        rimShotSub->setNextReadPosition(0); //reset sample to beginning
        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        { //TODO this loop seems weird, why is it a loop? double check tutorial
            if (samplenum == timeToStartPlaying)
            {
                rimShotSub->getNextAudioBlock(audiosourcechannelinfo);
            }
        }
        beatflag += 1;
     }
     else if (samplesProcessed + bufferSize >= beatInterval)
     { 
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        if (oneflag >= numerator) //check if its the first beat of the bar
        {
            //first beat logic
            DBG("HIGH");
            rimShotHigh->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotHigh->getNextAudioBlock(audiosourcechannelinfo);
                }
            }
            oneflag = 1; 
        }
        else 
        {
            //regular beat logic
            DBG("LOW");
            rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotLow->getNextAudioBlock(audiosourcechannelinfo);
                }
            }
            oneflag += 1;
            //non-one main beat
        }
        beatflag = 1;
    }


 /* if the read pointer is not 0, reading was interupted for some reason
 
 if (rimShotHigh->getNextReadPosition() != 0)
 {
     rimShotHigh->getNextAudioBlock(temp);

 }
 else if (rimShotLow->getNextReadPosition() != 0)          
    {
        rimShotLow->getNextAudioBlock(temp);
    }
    */
 
}



void Metronome::resetall() 
{   //this should be called whenever the metronome is stopped
    resetparams();
    totalSamples = 0;
    oneflag = numerator;
    beatflag = subdivisions;
}




void Metronome::resetparams()
{  //this should be called whenever the processor changes a parameter (which should only happen when the user interacts with the GUI)
    numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = (60.0 / bpm) * sampleRate;
    subInterval = beatInterval / subdivisions;

}

