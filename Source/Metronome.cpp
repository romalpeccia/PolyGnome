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
    //TODO: probably don't need to copy the apvts, double check here and in other constructors
    apvts = _apvts;  
    resetAll();
    formatManager.registerBasicFormats();
    
    auto buildDir = juce::File(juce::File::getCurrentWorkingDirectory().getFullPathName());
    auto baseDir = buildDir.getParentDirectory().getParentDirectory();
    jassert(baseDir.exists());
    //TODO: make this block a loop
    juce::MemoryInputStream inputStream(BinaryData::rimshot_low_wav, BinaryData::rimshot_low_wavSize, false);
    juce::WavAudioFormat wavFormat;
    juce::AudioFormatReader* formatReader = wavFormat.createReaderFor(&inputStream, false);
    //auto clickFile = juce::File(baseDir.getFullPathName() + "\\Samples\\rimshot_low.wav");
    //jassert(clickFile.exists());
    //auto formatReader = formatManager.createReaderFor(clickFile);
    rimShotLow.reset(new juce::AudioFormatReaderSource(formatReader, true));

    juce::MemoryInputStream inputStream2(BinaryData::rimshot_high_wav, BinaryData::rimshot_high_wavSize, false);
    juce::WavAudioFormat wavFormat2;
    juce::AudioFormatReader* formatReader2 = wavFormat.createReaderFor(&inputStream2, false);
    //juce::File clickFile2 = juce::File(baseDir.getFullPathName() + "\\Samples\\rimshot_high.wav");
    //jassert(clickFile2.exists());
    //auto formatReader2 = formatManager.createReaderFor(clickFile2);
    rimShotHigh.reset(new juce::AudioFormatReaderSource(formatReader2, true));

    juce::MemoryInputStream inputStream3(BinaryData::rimshot_sub_wav, BinaryData::rimshot_sub_wavSize, false);
    juce::WavAudioFormat wavFormat3;
    juce::AudioFormatReader* formatReader3 = wavFormat.createReaderFor(&inputStream3, false);
    //juce::File clickFile3 = juce::File(baseDir.getFullPathName() + "\\Samples\\rimshot_sub.wav");
    //jassert(clickFile3.exists());
    //auto formatReader3 = formatManager.createReaderFor(clickFile3);
    rimShotSub.reset(new juce::AudioFormatReaderSource(formatReader3, true));
    
}


void Metronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
{
//preparetoplay should call every time we start (right before)

    resetParams();

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
 //TODO cache calculations for less processing?
   
    resetParams();

    //TODO fix bug instead of this bandaid for sync issues
    if (subdivisionCounter > subdivisions)
        subdivisionCounter = subdivisions;

    //temp wrapper because <juce::AudioFormatReaderSource>->getNextAudioBlock expects an AudioSourceChannelInfoObject
    auto audiosourcechannelinfo = juce::AudioSourceChannelInfo(buffer);
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;
    subSamplesProcessed = totalSamples % subInterval;

    
     if (subdivisions > 1 && subSamplesProcessed + bufferSize >= subInterval && subdivisionCounter != subdivisions)
     {// subdivision logic
        const auto timeToStartPlaying = subInterval - subSamplesProcessed;
        rimShotSub->setNextReadPosition(0); //reset sample to beginning
        for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
        { 
            if (samplenum == timeToStartPlaying)
            {
                rimShotSub->getNextAudioBlock(audiosourcechannelinfo);
            }
        }
        subdivisionCounter += 1;
     }
     else if (samplesProcessed + bufferSize >= beatInterval)
     { 
        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        if (beatCounter >= numerator) //check if its the first beat of the bar
        {
            rimShotHigh->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotHigh->getNextAudioBlock(audiosourcechannelinfo);
                }
            }
            beatCounter = 1; 
        }
        else 
        {
            //regular beat logic
            rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotLow->getNextAudioBlock(audiosourcechannelinfo);
                }
            }
            beatCounter += 1;
            //non-one main beat
        }
        subdivisionCounter = 1;
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



void Metronome::resetAll() 
{   //this should be called whenever the metronome is stopped
    resetParams();
    totalSamples = 0;
    beatCounter = numerator;
    subdivisionCounter = subdivisions;
    samplesProcessed = 0;
    subSamplesProcessed = 0;
}


void Metronome::resetParams()
{  //this should be called whenever the processor changes a parameter (which should only happen when the user interacts with the GUI)
    //TODO: check if loading all of these is expensive, maybe we can only load the changed param
    numerator = apvts->getRawParameterValue("NUMERATOR")->load();
    subdivisions = apvts->getRawParameterValue("SUBDIVISION")->load();
    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = (60.0 / bpm) * sampleRate;
    subInterval = beatInterval / subdivisions;

}