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
    auto sampleDir = "C:/JUCE_PROJECTS/MetroGnome/Samples/";

    //TODO figure out how to use relative paths in JUCE
    juce::File clickFile("C:/JUCE_PROJECTS/MetroGnome/Samples/rimshot_low.wav");
    jassert(clickFile.exists());
    auto formatReader = formatManager.createReaderFor(clickFile);
    rimShotLow.reset(new juce::AudioFormatReaderSource(formatReader, true));


    juce::File clickFile2("C:/JUCE_PROJECTS/MetroGnome/Samples/rimshot_high.wav");
    jassert(clickFile2.exists());
    auto formatReader2 = formatManager.createReaderFor(clickFile2);
    rimShotHigh.reset(new juce::AudioFormatReaderSource(formatReader2, true));

    juce::File clickFile3("C:/JUCE_PROJECTS/MetroGnome/Samples/rimshot_sub.wav");
    jassert(clickFile3.exists());
    auto formatReader3 = formatManager.createReaderFor(clickFile3);
    rimShotSub.reset(new juce::AudioFormatReaderSource(formatReader3, true));

}

PolyRhythmMetronome::~PolyRhythmMetronome()
{
}



void PolyRhythmMetronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{   


    resetparams();
    auto audioSourceChannelInfo = juce::AudioSourceChannelInfo(buffer);
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;
    samplesProcessed = totalSamples % beatInterval;
    subSamplesProcessed = totalSamples % subInterval;


    bool beatflag = (samplesProcessed + bufferSize >= beatInterval && numerator > 1);
    bool subflag = (subSamplesProcessed + bufferSize >= subInterval && subdivisions > 1);
   



    if (rhythm1Counter >= numerator )
    {
        rhythm1Counter = 0;
        totalSamples = 0;
    }
    if (rhythm2Counter >= subdivisions )
    {
        rhythm2Counter = 0;
        totalSamples = 0;
    }



    //TODO make this block cleaner
    if ( ((beatflag && subflag )) )
    { // both beats hit at the same time , play a unique tick for that 

        const auto timeToStartPlaying = beatInterval - samplesProcessed;
        rhythm1Counter += 1;
        rhythm2Counter += 1;
        int ID1 = rhythm1Counter;
        int ID2 = rhythm2Counter;
        if (rhythm1Counter == numerator){
            ID1 = 0;
        }
        if (rhythm2Counter == subdivisions) {
            ID2 = 0;
        }


        if (apvts->getRawParameterValue("RHYTHM1." + to_string(ID1) + "TOGGLE")->load() == true && apvts->getRawParameterValue("RHYTHM2." + to_string(ID2) + "TOGGLE")->load() == true) {

            //rimShotHigh->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    //rimShotHigh->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, 38);
                }
            }
        }
        else if (apvts->getRawParameterValue("RHYTHM1." + to_string(ID1) + "TOGGLE")->load() == true) {

            //rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    //rimShotLow->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, 36);
                }
            }
        }
        else if (apvts->getRawParameterValue("RHYTHM2." + to_string(ID2) + "TOGGLE")->load() == true) {


           // rimShotSub->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            { 
                if (samplenum == timeToStartPlaying)
                {
                    //rimShotSub->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, 37);
                }
            }
        }
       // DBG(to_string(rhythm1Counter) + ";" + to_string(rhythm2Counter));




    }
    else if (beatflag)
    {
        rhythm1Counter += 1;
        //DBG(to_string(rhythm1Counter) + ";" + to_string(rhythm2Counter));
        if (apvts->getRawParameterValue("RHYTHM1." + to_string(rhythm1Counter) + "TOGGLE")->load() == true) {

            const auto timeToStartPlaying = beatInterval - samplesProcessed;
            rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    //rimShotLow->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, 36);
                }
            }
        }

       


    }
    else if (subflag )
    {
        rhythm2Counter += 1;
        //DBG(to_string(rhythm1Counter) + ";" + to_string(rhythm2Counter));
        if (apvts->getRawParameterValue("RHYTHM2." + to_string(rhythm2Counter) + "TOGGLE")->load() == true) {

            const auto timeToStartPlaying = subInterval - subSamplesProcessed;
            //rimShotSub->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            { 
                if (samplenum == timeToStartPlaying)
                {
                    //rimShotSub->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, 37);
                }
            }
        }
    }




}

void PolyRhythmMetronome::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber)
{
    auto noteDuration = sampleRate;
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    //message.setTimeStamp(noteDuration);

    auto messageOff = juce::MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());
    //messageOff.setTimeStamp((noteDuration));

    if (! midiBuffer.addEvent(message, 0)  || ! midiBuffer.addEvent(messageOff, 100) )
    {
        DBG("error adding messages to midiBuffer");
    }

}
void PolyRhythmMetronome::resetall()
{   //this should be called whenever the metronome is stopped
   // resetparams();
    totalSamples = 0;
    rhythm1Counter = 0;
    rhythm2Counter = 0;
    samplesProcessed = 0;
    subSamplesProcessed = 0;
}


void PolyRhythmMetronome::resetparams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm

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


    bpm = apvts->getRawParameterValue("BPM")->load();
    beatInterval = 4 * ((60.0 / bpm) * sampleRate) / numerator;
    subInterval = 4 * ((60.0 / bpm) * sampleRate) / subdivisions;
    ///TODO (maybe) assumes 4/4 time, maybe make time sig a parameter to advance this

}
