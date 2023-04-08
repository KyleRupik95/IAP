//
//  IAP.cpp
//  IAPProjectDevelopmentTest1
//
//  Created by Samuel Hunt on 16/07/2018.
//  Copyright (c) 2018 SJHDevelopment. All rights reserved.
//

#include "IAP.h"
#include <iostream>
#include <fstream>

void IAP::run()
{
	while (loginPassword != filePassword) {
		loginFileRead();
		login();
	}
	std::cout << "login successful" << std::endl;
	loginFileWrite();
	arrayPrep();
	aserveLoadDefaultSounds();
	instructions();
	while (programExit < 50){ //program exits when a rotary controls midi value goes past 50
		aserveSleep(1);
		drumSequencer();
		randomNoteGenerator();
		polySequencerPlay();
	}
}
//---------------------------------------------------------------------------------------------------------
// callback functions


void IAP::callbackNoteReceived(int note, int velocity, int channel)
{
	if (polySequencerOn == false) drumPadOn(note, velocity, channel);
	if (polySequencerOn == true) polyphonicSequencer(note, velocity, channel);
}
void IAP::callbackCCValueChanged(int cc, int value)
{
	//polysequencer controls
	polSequencerControls(cc, value);
	//wave type poly synthesiser controls
	polySynthControls(cc, value);
	// drumpad and drum sequencer controls
	drumControls(cc, value);
	//Random Note Generator controls
	rngControls(cc, value);
	// PROGRAM EXIT control.
	if (cc == KNOB_BOT_FOUR) programExit = value; // if value goes above 50, program will exit
}
void IAP::callbackModWheelMoved(int value) {
	float cutoff = (pow((value / 127.0), 3.0) * 19800) + 20;
	aserveLPF(cutoff);
}
void IAP::callbackPitchbendWheelMoved(int value) {
	pbValue = (value / 16383.0) * 24 - 12.0;
}

//---------------------------------------------------------------------------------------------------------------------------------
//Additive polyphonic synthesiser and sequencer functions
void IAP::polySynthControls(int cc, int value) {
	//oscillator amplitude slider controls
	if (cc == SLIDER_ONE_SINE_AMP)    sineAmp = value / 127.0;  // sine wave amplitude slider
	if (cc == SLIDER_TWO_SQUARE_AMP)  squareAmp = value / 127.0;  // square wave amplitude slider
	if (cc == SLIDER_THREE_SAW_AMP)   sawAmp = value / 127.0;  // saw wave amplitude slider
	if (cc == SLIDER_FOUR_TRI_AMP)    triAmp = value / 127.0;  // triangle wave amplitude slider
	if (cc == SLIDER_FIVE_NOISE_AMP)  noiseAmp = value / 127.0;  // white noise amplitude slider
}

void IAP::polSequencerControls(int cc, int value) {
	if (cc == PLAY_BTN) { //play button, plays poly sequence. Stops all other sequencers.
		spolyPlay = true;
		drumSequenceOne = false;
		drumSequenceTwo = false;
		drumSequenceThree = false;
		rngRandomise = false;
	}
	//Stop button. Stops all sequencers playing. Turns off oscillators.
	if (cc == STOP_BTN) {
		spolyPlay = false;
		drumSequenceOne = false;
		drumSequenceTwo = false;
		drumSequenceThree = false;
		rngRandomise = false;
		resetOscillators();
		seqTime.pop_back(); //needed to prevent a crash in polySequencerPlay() where pressing stop, then adding more notes to the sequence causes a crash due to seqTime having more elements than the other sequence vectors.
	}
	//clear all previous sequencer vector elements. Stop playback. Turn off oscillators.A Ready to record a new sequence.
	if (cc == RECORD_BTN) {
		spolyPlay = false;
		resetOscillators();
		clearSequencer();
	}

	if (value == ON) if (cc == SQUARE_ONE) polySequencerOn = true;
	if (value == OFF) if (cc == SQUARE_ONE) polySequencerOn = false;
}
void IAP::polyphonicSequencer(int note, int velocity, int channel)
{
	if (velocity > 0)
	{
		int freeChannel = 0; //reset freeChannel back to 0
		for (freeChannelIndex = 0; freeChannelIndex < channelList.size(); freeChannelIndex++)
		{
			if (channelList[freeChannelIndex] == -1)
			{
				freeChannel = freeChannelIndex;
				break;
			}
		}
		// we've found a free channel now.
		// if statement limits number of channels user can play at once to prevent going over and crashing aserve
		// playing more than four notes at one time would bring the number of simultaneous channels called past 24 and crash.
		if (freeChannelIndex < 19)
		{
			SequencerOscGroupOnOff(freeChannel, note, velocity, sineAmp, SINE, note);
			SequencerOscGroupOnOff(freeChannel + 1, note, velocity, squareAmp, SQUARE, note);
			SequencerOscGroupOnOff(freeChannel + 2, note, velocity, sawAmp, SAW, note);
			SequencerOscGroupOnOff(freeChannel + 3, note, velocity, triAmp, TRI, note);
			SequencerOscGroupOnOff(freeChannel + 4, note, velocity, noiseAmp, NOISE, note);
		}
	}
	//note off
	if (velocity == 0) {
		int busyChannel = 0; //reset busyChannel back to 0
		for (busyChannelIndex = 0; busyChannelIndex < channelList.size(); busyChannelIndex++) {

			if (channelList[busyChannelIndex] == note) // find the busy channel
			{
				busyChannel = busyChannelIndex;
				break;
			}
		}
		// we've found our busy channel now
		// Record Channel, Note, Amplitude and Time stamp for sequencer vectors
		// if statement limits number of channel user can play at once to prevent going over and crashing aserve
		// playing more than four notes at one time would bring the number of simultaneous channels called past 24 and crash.
		if (busyChannelIndex < 19) {
			SequencerOscGroupOnOff(busyChannel, note, velocity, sineAmp, SINE, oscOFF);
			SequencerOscGroupOnOff(busyChannel + 1, note, velocity, squareAmp, SQUARE, oscOFF);
			SequencerOscGroupOnOff(busyChannel + 2, note, velocity, sawAmp, SAW, oscOFF);
			SequencerOscGroupOnOff(busyChannel + 3, note, velocity, triAmp, TRI, oscOFF);
			SequencerOscGroupOnOff(busyChannel + 4, note, velocity, noiseAmp, NOISE, oscOFF);
		}
	}
}
//records note midi data note ons and note offs for sequencer.
void IAP::SequencerOscGroupOnOff(int channel, int note, int noteVelocity, float sliderAmplitude, int waveType, int channelOnOff) {
	seqChannel.push_back(channel);
	seqNote.push_back(note);
	seqAmp.push_back(amplitude(noteVelocity) * sliderAmplitude);
	seqWave.push_back(waveType);
	seqTime.push_back(aserveGetTime());
	aserveOscillator(channel, mtof(note + pbValue), (amplitude(noteVelocity) * sliderAmplitude), waveType);
	channelList[channel] = channelOnOff; // mark the channel/oscillator as free/busy
}

//function for playing the polyphonic sequencer
void IAP::polySequencerPlay() {
	if (spolyPlay == true) {
		if (seqTime.size() < seqNote.size() + 1) {
			seqTime.push_back(seqTime.back()); // This is to prevent crash in the next for loop where [i+1] counts past the total number of vector elements.
			for (int i = 0; i < seqTime.size() - 1; i++) {
				if (i < seqTime.size() - 1) seqDuration.push_back(seqTime[i + 1] - seqTime[i]);
			}
		}
		else
		{
			int limit = seqNote.size();
			for (int i = 0; i < limit; i++) {  //counter loop that adds +1 to i each time. Stops before reaching limit                
				if (spolyPlay == true) {       //if statement is so the stop button stops the sequence immediately
					aserveOscillator(seqChannel[i], mtof(seqNote[i] + pbValue), seqAmp[i], seqWave[i]); //sequencer vector elements for channel, note, amplitude and duration used.
					aserveSleep(seqDuration[i]);
				}
			}
		}
	}
}
//for loop to turn off all oscillators
void IAP::resetOscillators() {
	for (int i = 0; i < oscReset.size(); i++) {
		aserveOscillator(i, oscReset[i], oscReset[i], 0);
	}
}
//clears all polyphonic sequencer vectors
void IAP::clearSequencer()
{
	seqChannel.clear();
	seqNote.clear();
	seqAmp.clear();
	seqTime.clear();
	seqDuration.clear();
}

void IAP::arrayPrep() //initilize array
{
	for (int i = 0; i < channelList.size(); i++)
		channelList[i] = -1; // mark all oscillators as "idle, free, available"
	for (int i = 0; i < oscReset.size(); i++)
		oscReset[i] = 0.0; // array for reseting all oscillators to 0 amplitude
}

float IAP::amplitude(int velocity) //convert note midi velocity to amplitude
{
	return velocity / 127.0;
}

float IAP::mtof(int note) // convert midi note to frequency
{
	return 440.0 * pow(2.0, (note - 69) / 12.0);
}

//---------------------------------------------------------------------------------------------------------
//Random Note Generator Functions

void IAP::randomNoteGenerator()
{
	if (rngRandomise == true) {
		int note = GenerateNote();
		note = majorScale(note);

		if (note > (12 + rngPrevNote)) note -= 12; // if the new note to be played is more than 12 semitones above, then subtract 12.
		if (note < (12 - rngPrevNote)) note += 12; // If the new note to be played is less than 12 semitones bellow, then add 12

		rngPrevNote = note + rngPitchShift;
		aserveOscillator(20, mtof(note + rngPitchShift + pbValue), rngVolume, rngWave);

		int duration = GetRandomArrayIndex(rngRythm) + rngRythmShift;
		aserveSleep(duration);
	}
}

//random number generator cc controls
void IAP::rngControls(int cc, int value) {
	if (value == ON) if (cc == SQUARE_NINE_RNG) {
		rngRandomise = true;
		drumSequence = false;
	}
	if (value == OFF) if (cc == SQUARE_NINE_RNG) {
		aserveOscillator(20, 0, 0, 0);
		rngRandomise = false;
	}
	if (rngRandomise == true) {
		if (cc == KNOB_BOT_TWO_RNG_PITCH)   rngPitchShift = value - 63;
		if (cc == KNOB_BOT_THREE_RNG_RYTHM) rngRythmShift = value * 10;
		if (cc == SLIDER_NINE_RNG_AMP)      rngVolume = value / 127.0;
		if (cc == KNOB_BOT_ONE_RNG_WAVE) {
			if (value >= 0 && value < 31) rngWave = SINE;
			if (value >= 31 && value < 62) rngWave = SQUARE;
			if (value >= 62 && value < 93) rngWave = SAW;
			if (value >= 94 && value <= 127) rngWave = TRI;
		}
	}
}
int IAP::majorScale(int note) // takes midi note and converts the note to the major scale
{
	int pitchclass = note % 12;

	if (pitchclass == 1 ||
		pitchclass == 3 ||
		pitchclass == 6 ||
		pitchclass == 8 ||
		pitchclass == 10)
	{
		note++;
		return note;
	}
}

//generate random note. 
//Then check if the gap between next note is bigger than 12. 
//+-12 to note if it is
int IAP::GenerateNote()
{
	return (rand() % 49) + 24;
}

//returns random index from the rhythm array
int IAP::GetRandomArrayIndex(std::array<int, 6> rngRythm)
{
	int i = 0;
	for (int i = 0; i < rngRythm.size(); i = rand() % 7);
	return rngRythm[i];
}

//---------------------------------------------------------------------------------------------------------
//drum functions

void IAP::drumPadOn(int note, int velocity, int channel) {
	if (velocity > 0)
	{
		if (note == 67)
		{
			aservePlaySample(0, drumAmp);
		}

		else if (note == 69)
		{
			aservePlaySample(1, drumAmp);
		}
		else if (note == 71)
		{
			aservePlaySample(2, drumAmp);
		}
		else if (note == 72)
		{
			aservePlaySample(3, drumAmp);
		}

	}
}

void IAP::drumControls(int cc, int value)
{
	if (cc == KNOB_TOP_FOUR_D_TEMPO)
		drumTempo = value;         //rotary knob controling drum sequence tempo
	if (cc == SLIDER_SIX_DRUM_AMP)
		drumAmp = value / 127.0;  //slider controlling drum pad and drum sequence 

	if (value == ON)                  //the following code is the controls for turning on/off the different drum sequence presets
		if (cc == SQUARE_SIX_SDRUM) { //turning on one SQUARE button turns off the random note generator and other sequencers
			drumSequenceOne = true;
			drumSequenceTwo = false;
			drumSequenceThree = false;
			rngRandomise = false;
			spolyPlay = false;
		}
	if (value == OFF)
		if (cc == SQUARE_SIX_SDRUM) {
			drumSequenceOne = false;
		}
	if (value == ON)
		if (cc == SQUARE_SEVEN_SDRUM) {
			drumSequenceOne = false;
			drumSequenceTwo = true;
			drumSequenceThree = false;
			rngRandomise = false;
			spolyPlay = false;
		}
	if (value == OFF)
		if (cc == SQUARE_SEVEN_SDRUM) {
			drumSequenceTwo = false;
		}
	if (value == ON)
		if (cc == SQUARE_EIGHT_SDRUM) {
			drumSequenceOne = false;
			drumSequenceTwo = true;
			drumSequenceThree = true;
			rngRandomise = false;
			spolyPlay = false;
		}
	if (value == OFF)
		if (cc == SQUARE_EIGHT_SDRUM)
			drumSequenceThree = false;
}
void IAP::drumSequencer()
{
	if (drumSequenceOne == true) {
		aservePlaySample(0, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(1, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(2, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(3, drumAmp);
		aserveSleep(100 + drumTempo);
	}
	if (drumSequenceTwo == true) {
		aservePlaySample(0, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(0, drumAmp);
		aserveSleep(150 + drumTempo);
		aservePlaySample(3, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(1, drumAmp);
		aserveSleep(50 + drumTempo);
		aservePlaySample(2, drumAmp);
		aserveSleep(50 + drumTempo);
	}
	if (drumSequenceThree == true) {
		aservePlaySample(3, drumAmp);
		aservePlaySample(0, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(1, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(2, drumAmp);
		aserveSleep(50 + drumTempo);
		aservePlaySample(2, drumAmp);
		aserveSleep(100 + drumTempo);
		aservePlaySample(3, drumAmp);
		aserveSleep(100 + drumTempo);
	}
}

//---------------------------------------------------------------------------------------------------------
//txt file functions
void IAP::loginFileRead() {
	std::ifstream fileInputStream;
	fileInputStream.open("password.txt", std::fstream::in);

	if (fileInputStream.is_open())
	{
		std::string line;
		while (!fileInputStream.eof()) //Loop that reads each line until it reaches end of file (eof)
		{
			std::getline(fileInputStream, line); //get line in txt file,
			if (line.length() > 0) //if the number of characters in line of txt file is greater than zero
			{                      //this section of the code is necassary because without it filePassword = " " instead of the intended set of characters
				filePassword = line;
			}
		}
		fileInputStream.close(); // we are at the end of txt file, close the file
	}
	else
	{
		std::cout << "error opening file\n";
	}
}

//function for changing password by writing to file.
//Asks user if they would like to change password
//No breaks the loop and moves on
//if yes, user inputs their chosen password, new password is written to file.
//Any other input continues loop. User is asked if they would like to change password again.
void IAP::loginFileWrite() {
	bool loop = true;
	while (loop == true) {
		std::string answer;
		std::cout << "would you like to choose a new password? YES/NO" << std::endl;
		std::cin >> answer;
		if (answer == "YES" || answer == "yes" || answer == "Yes" || answer == "YEs" || answer == "yeS" || answer == "yES" || answer == "yEs") {
			loop = false;
			std::ofstream fileOutputStream;
			fileOutputStream.open("password.txt", std::ofstream::out);
			std::string line;
			std::cout << "please choose a new password" << std::endl;
			std::cout << "(maybe keep it as PASSWORD if someone else is going to mark this later)" << std::endl;
			std::cin >> line;
			fileOutputStream << line << "\n";
			fileOutputStream.close();
		}
		if (answer == "NO" || answer == "No" || answer == "no" || answer == "nO")
			loop = false;
	}
}

//this function asks the user for a password input
//if the password is wrong it asks the user if they have forgotten.
//yes outputs the password and breaks the loop
//no also breaks the loop
//any other input repeats the loop and the user is asked if they have forgotten the password again.
void IAP::login()
{
	std::cout << "The default password is PASSWORD" << std::endl;
	std::cout << "You can change the password after logging in." << std::endl;
	std::cout << "Please enter the password and press enter to login." << std::endl;
	std::cin >> loginPassword;
	if (loginPassword == filePassword) password = true;
	if (loginPassword != filePassword) {
		bool loop = true;
		while (loop == true) {
			std::string answer;
			std::cout << "have you forgotten the password? YES/NO" << std::endl;
			std::cin >> answer;
			if (answer == "YES" || answer == "yes" || answer == "Yes" || answer == "YEs" || answer == "yeS" || answer == "yES" || answer == "yEs") {
				loop = false;
				std::cout << "That was silly of you." << std::endl;
				std::cout << "But maybe a different lecturer already changed the password and locked you out." << std::endl;
				std::cout << "So I'll let you off this time." << std::endl;
				std::cout << "The password is: " << filePassword << std::endl;
			}
			if (answer == "NO" || answer == "No" || answer == "no" || answer == "nO")
				std::cout << "I believe in you." << std::endl;
			loop = false;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
//misc functions
void IAP::instructions()
{
	std::cout << "This programs features include: " << std::endl;
	std::cout << std::endl;
	std::cout << "---Polyphonic Synthesis-------" << std::endl;
	std::cout << "Using 5 oscillators with different wave types for infinite sound design options." << std::endl;
	std::cout << "Includes separate amplitude controls for the 5 oscillators." << std::endl;
	std::cout << "Includes pitch bend and low pass filter." << std::endl;
	std::cout << "Up to 4 notes can be played simultaneously." << std::endl;
	std::cout << std::endl;
	std::cout << "---Polyphonic Sequencer------" << std::endl;
	std::cout << "Record and playback sequences of the above 5 oscillator synthesiser." << std::endl;
	std::cout << std::endl;
	std::cout << "---Drum Sequencer-------------" << std::endl;
	std::cout << "Featuring 3 preset sequences to choose from" << std::endl;
	std::cout << "Tempo and amplitude controls" << std::endl;
	std::cout << std::endl;
	std::cout << "---Random Note Generator---" << std::endl;
	std::cout << "Switch between 4 wave types" << std::endl;
	std::cout << "Tempo controls" << std::endl;
	std::cout << "pitch range controls" << std::endl;
	std::cout << "Amplitude controls" << std::endl;
	std::cout << std::endl;
	std::cout << "---Instructions---" << std::endl;
	std::cout << std::endl;
	std::cout << "---Polyphonic Synthesiser---" << std::endl;
	std::cout << "Turn on the polyphonic synthesiser by pressing the first square button in the bottom left." << std::endl;
	std::cout << "Control the 5 oscillator amplitudes using the first 5 sliders from the left." << std::endl;
	std::cout << "From left to right:" << std::endl;
	std::cout << "Slider 1 controls Sine Wave Amplitude." << std::endl;
	std::cout << "Slider 2 controls Square Wave Amplitude." << std::endl;
	std::cout << "Slider 3 controls Saw Wave Amplitude." << std::endl;
	std::cout << "Slider 4 controls Triangle Wave Amplitude." << std::endl;
	std::cout << "Slider 5 controls White Noise Amplitude." << std::endl;
	std::cout << "Turning on the polyphonic synthesiser will turn off the drum pads." << std::endl;
	std::cout << "However you can still play sequences while using either the polyphonic synthesiser or drum pads" << std::endl;
	std::cout << std::endl;
	std::cout << "---Drum pad---" << std::endl;
	std::cout << "Control the drum pad amplitude using the fourth slider from the right." << std::endl;
	std::cout << "Turning on the polyphonic synthesiser will turn off the Drum Pads." << std::endl;
	std::cout << "However you can still play sequences while using the Drum pads (or poly synth)." << std::endl;
	std::cout << std::endl;
	std::cout << "---Polyphonic Sequencer------" << std::endl;
	std::cout << "Turn on/off the polyphonic sequencer with the first square button in the bottom left." << std::endl;
	std::cout << "Press the record button and begin playing notes to record a sequence." << std::endl;
	std::cout << "Be careful. This will erase the previous sequence data." << std::endl;
	std::cout << "Press the play button the playback the sequence." << std::endl;
	std::cout << "Press the stop button to stop playback." << std::endl;
	std::cout << std::endl;
	std::cout << "---Drum Sequencer-------------" << std::endl;
	std::cout << "Choose between the three drum sequence presets using three square buttons" << std::endl;
	std::cout << "Second square button from the right." << std::endl;
	std::cout << "Third square button from the right." << std::endl;
	std::cout << "Fourth square button from the right." << std::endl;
	std::cout << "Control the amplitude of the drums using the fourth slider from the right." << std::endl;
	std::cout << "Control Tempo of the drum sequence using the top right rotary knob." << std::endl;
	std::cout << "Use the third square buttom from the left to turn off the sequencer." << std::endl;
	std::cout << std::endl;
	std::cout << "---Random Note Generator---" << std::endl;
	std::cout << "Turn on/off the Random Note Generator using the first square bottom from the right." << std::endl;
	std::cout << "Control the amplitude using the above slider, first one from the right." << std::endl;
	std::cout << "Switch between the four wave types using the bottom left rotary knob." << std::endl;
	std::cout << "Control the pitch range using the knob second from bottom left." << std::endl;
	std::cout << "Control the Tempo/rhythm using the knob third from bottom left." << std::endl;
	std::cout << std::endl;
	std::cout << "---Pitch Bend Wheel---" << std::endl;
	std::cout << "The pitch bend wheel is located in the bottom left." << std::endl;
	std::cout << "It can be used to modulate pitch of the polyphonic synthesiser." << std::endl;
	std::cout << "It can be used to pitch bend the sequencers live as they are playing." << std::endl;
	std::cout << "It affects both the random note generator and polyphonic sequencer." << std::endl;
	std::cout << "The effects of pitch bend wheel are not recorded by any of the sequencers during initial recording." << std::endl;
	std::cout << std::endl;
	std::cout << "---Low Pass Filter Mod Wheel---" << std::endl;
	std::cout << "The mod wheel is located is the second wheel located in the bottom right." << std::endl;
	std::cout << "It controls the low pass filter." << std::endl;
	std::cout << "The low pass filter affects all sound outputs including the drum samples and all sequencers." << std::endl;
	std::cout << std::endl;
}

