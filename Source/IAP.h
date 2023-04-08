//
//  IAP.h
//  IAPProjectDevelopmentTest1
//
//  Created by Samuel Hunt on 16/07/2018.
//  Copyright (c) 2018 SJHDevelopment. All rights reserved.
//

#ifndef __IAPProjectDevelopmentTest1__IAP__
#define __IAPProjectDevelopmentTest1__IAP__

#include "AserveComs.h"

//---------------------------------------------------------------------------------
// USER CREATED CLASS


class IAP : public AserveComs {
public:

    //---------------------------------------------------------------------------------
    // SHARED VARIABLES (IAP member variables)
    std::array<int, 24> channelList;
    std::array<int, 24> oscReset;
    int freeChannelIndex = -1;
    int busyChannelIndex = -1;

    //misc
    int pbValue = 0; // pitch bend value
    int programExit = 1; //aserve program exit control variable
    float MasterVolume = 0.0;

    //login
    //file
    bool password = false;
    std::string filePassword = "PASSWORD";
    std::string loginPassword = "placehold";

    //arpegiator
    std::vector<int> arpNotes;
    int arpOrder;

    //random note generator
    std::vector<int> notes;
    std::array<int, 6> rngRythm{ 125, 250, 375, 500, 750, 1000 };
    bool rngRandomise = false;
    int rngPitchShift = 0;
    int rngRythmShift = 0;
    int rngWave = 0;
    float rngVolume = 0.0;
    int rngPrevNote;

    //sequencer variables
    std::vector <int> seqChannel;
    std::vector <int> seqNote;
    std::vector <float> seqAmp;
    std::vector <int> seqWave;
    std::vector <int> seqTime;
    std::vector <int> seqDuration;
    bool spolyPlay = false;
    int seqTimeIndex = 0;

    bool polySequencerOn = false;

    //drum sequencer variables
    std::array <int, 4> sDrumAmp;     //records sample amplitude
    std::array <int, 4> sDrumChannel; //records sample channel
    bool drumSequence = false;
    bool drumSequenceOne = false;
    bool drumSequenceTwo = false;
    bool drumSequenceThree = false;
    int drumTempo = 0;
    float drumAmp = 0.0;

    //Wavetype additive synth variables
    const int SINE = 0;   //wave types
    const int SQUARE = 1;
    const int SAW = 3;
    const int TRI = 4;
    const int NOISE = 5;

    float sineAmp = 0; //slider amplitude values
    float squareAmp = 0;
    float sawAmp = 0;
    float triAmp = 0;
    float noiseAmp = 0;

    //constants
    const int oscOFF = -1; // oscillator is off

    //callbackCCvaluechanged(int cc, int value) constants
    //int value constants representing ON/OFF status of the Buttons
    const int ON = 127;
    const int OFF = 0;

    //int cc constants
    //rewind, play, etc, buttons ordered from left to right in descending order.
    const int REWIND_BTN = 112;
    const int FORWARD_BTN = 113;
    const int STOP_BTN = 114;
    const int PLAY_BTN = 115;
    const int REPLAY_BTN = 116;
    const int RECORD_BTN = 117;

    // square buttons numbered in order of left to right
    const int SQUARE_ONE = 51;
    const int SQUARE_TWO = 52;
    const int SQUARE_THREE = 53;
    const int SQUARE_FOUR = 54;
    const int SQUARE_FIVE = 55;
    const int SQUARE_SIX_SDRUM = 56;
    const int SQUARE_SEVEN_SDRUM = 57;
    const int SQUARE_EIGHT_SDRUM = 58;
    const int SQUARE_NINE_RNG = 59;

    //sliders numbered in order of left to right
    const int SLIDER_ONE_SINE_AMP = 41;
    const int SLIDER_TWO_SQUARE_AMP = 42;
    const int SLIDER_THREE_SAW_AMP = 43;
    const int SLIDER_FOUR_TRI_AMP = 44;
    const int SLIDER_FIVE_NOISE_AMP = 45;
    const int SLIDER_SIX_DRUM_AMP = 46;
    const int SLIDER_SEVEN = 47;
    const int SLIDER_EIGHT_DRUM_AMP = 48;
    const int SLIDER_NINE_RNG_AMP = 49;

    //top rotary knobs numbered in order of left to right
    const int KNOB_TOP_ONE = 21;
    const int KNOB_TOP_TWO = 22;
    const int KNOB_TOP_THREE = 23;
    const int KNOB_TOP_FOUR_D_TEMPO = 24; // drum tempo control

    //bottom rotary knobs in order of left to right
    const int KNOB_BOT_ONE_RNG_WAVE = 25;
    const int KNOB_BOT_TWO_RNG_PITCH = 26; //random note generator pitch control
    const int KNOB_BOT_THREE_RNG_RYTHM = 27; // random note generator ryhtm control
    const int KNOB_BOT_FOUR = 28;



    //---------------------------------------------------------------------------------
    // FUNCTIONS (IAP class methods)
    void run();

    //login functions
    void loginFileRead();
    void loginFileWrite();
    void login();
    void instructions();


    //drums
    void drumPadOn(int note, int velocity, int channel);
    void drumControls(int cc, int value);
    void drumSequencer();
    //sequencer and additive synth functions
    void polySynthControls(int cc, int value);
    void polSequencerControls(int cc, int value);
    void polySequencerPlay();
    void polyphonicSequencer(int note, int velocity, int channel);
    void SequencerOscGroupOnOff(int channel, int note, int noteVelocity, float sliderAmplitude, int waveType, int channelOnOff);
    void arrayPrep();
    void clearSequencer();
    void resetOscillators();

    //random note generator functions
    void randomNoteGenerator();
    void rngControls(int cc, int value);
    int majorScale(int note);
    int GetRandomArrayIndex(std::array<int, 6> rythm);
    int GenerateNote();


    //---------------------------------------------------------------------------------
    // CALLBACK FUNCTIONS

    float mtof(int note);
    float amplitude(int velocity);
    //void playNote(int note) { aserveOscillator(0, mtof(note), 0.5, 0); };
    //void stopNote(int note) { aserveOscillator(0, 0.0, 0.0, 0); };
    void callbackNoteReceived(int note, int velocity, int channel);
    void callbackModWheelMoved(int value);
    void callbackPitchbendWheelMoved(int value);
    void callbackCCValueChanged(int cc, int value);

    //void callbackMIDIRecived (MIDI message);
    //void callbackPixelGrid (int x, int y);
#ifdef JUCE_WINDOWS
    void aserveLoadDefaultSounds()
    {
        //make sure you set these to be your own file paths
        aserveLoadSample(0, "C:\\Users\\Kyle Rupik\\Downloads\\IAP-2021-windows\\IAP-2021-windows\\sounds\\bd.wav");
        aserveLoadSample(1, "C:\\Users\\Kyle Rupik\\Downloads\\IAP-2021-windows\\IAP-2021-windows\\sounds\\sd.WAV");
        aserveLoadSample(2, "C:\\Users\\Kyle Rupik\\Downloads\\IAP-2021-windows\\IAP-2021-windows\\sounds\\chh.WAV");
        aserveLoadSample(3, "C:\\Users\\Kyle Rupik\\Downloads\\IAP-2021-windows\\IAP-2021-windows\\sounds\\ohh.WAV");
        aserveLoadPitchedSample(0, "C:\\Users\\Kyle Rupik\\Downloads\\IAP-2021-windows\\IAP-2021-windows\\sounds\\pianoSample.WAV", 60, 0.01, 0.3);

    }
#endif // JUCE_WINDOWS

private:

};

#endif /* defined(__IAPProjectDevelopmentTest1__IAP__) */