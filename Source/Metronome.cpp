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
        
    //end load up click file
}


void Metronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
//preparetoplay should call every time we start (right before)
{

    sampleRate = _sampleRate;
    resetparams();

    if (rimShotLow != nullptr)
    {
        rimShotLow->prepareToPlay(samplesPerBlock, sampleRate);
       // DBG("file loaded");
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

void Metronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
{
    //TODO fix this entire function, seems hacky 
    //TODO cache calculations for less processing?
    //temp because <juce::AudioFormatReaderSource>->getNextAudioBlock expects an AudioSourceChannelInfoObject
    resetparams();
    auto temp = juce::AudioSourceChannelInfo(buffer);

    auto bufferSize = buffer.getNumSamples();

    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;
    auto subSamplesProcessed = totalSamples % subInterval;

    
     if (subdivisions > 1 && subSamplesProcessed + bufferSize >= subInterval && beatflag != subdivisions)
     {
         const auto timeToStartPlaying = subInterval - subSamplesProcessed;
         DBG("SUB" << beatflag);
             rimShotSub->setNextReadPosition(0); //reset sample to beginning
             for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
             {
                 if (samplenum == timeToStartPlaying)
                 {
                     rimShotSub->getNextAudioBlock(temp);
                 }
             }
             beatflag += 1;
     }


     else if (samplesProcessed + bufferSize >= beatInterval)
     {
         const auto timeToStartPlaying = beatInterval - samplesProcessed;
         if (oneflag >= numerator) //check if its the first beat of the bar
         {
             DBG("HIGH");
             rimShotHigh->setNextReadPosition(0); //reset sample to beginning
             for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
             {
                 if (samplenum == timeToStartPlaying)
                 {
                     rimShotHigh->getNextAudioBlock(temp);
                 }
             }
             oneflag = 1;
         }
         else 
         {
             //non-one main beat
             DBG("LOW");
             rimShotLow->setNextReadPosition(0); //reset sample to beginning
             for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
             {
                 if (samplenum == timeToStartPlaying)
                 {
                     rimShotLow->getNextAudioBlock(temp);
                 }
             }
             oneflag += 1;
             //non-one main beat
         }
         beatflag = 1;
     }


 /*
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
{
    numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    bpm = apvts->getRawParameterValue("BPM")->load();
    totalSamples = 0;
    beatInterval = (60.0 / bpm) * sampleRate;
    subInterval = beatInterval / subdivisions;

    oneflag = numerator;
    beatflag = subdivisions;
}




void Metronome::resetparams()
{
    numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = (60.0 / bpm) * sampleRate;
    subInterval = beatInterval / subdivisions;

    //oneflag = numerator;
    //beatflag = subdivisions;


}

