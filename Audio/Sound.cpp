//
//  Sound.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/12/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//  Taken from http://www.roguebasin.com/index.php?title=Implementing_sound_in_C_and_C_Plus_Plus
//

#include "Sound.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;

//Add new Sound Effect files here, Do not add background music here
string files[] = {
   "Audio/Music/EverythingFadesToGray.mp3",
   "Audio/Effects/footsteps.mp3",
   "Audio/Effects/hammerSmash.mp3",
   "Audio/Effects/scream.mp3"
};
#define numChannels 4

enum channelUsage {
   BACKGROUND_CHANNEL = 0,
   WALK_CHANNEL,
   HAMMER_CHANNEL,
   SCREAM_CHANNEL,
};

//Add background music here
string backgroundMusic[] = {
   "Audio/Music/Epiphany.mp3",
   "Audio/Music/LetItGo.mp3",
   "Audio/Music/Orion.mp3",
   "Audio/Music/EverythingFadesToGray.mp3",
   "Audio/Music/LastOfTheWilds.mp3",
};
#define backgroundFiles 4
int jukeboxIndex = 0;

/******** CLASS VARIABLE DECLARATIONS ********/

bool Sound::on = true; //is sound on?
bool Sound::possible = true; //is it possible to play sound?
//FMOD-specific stuff
FMOD_RESULT Sound::result;
FMOD_SYSTEM * Sound::fmodsystem;
FMOD_SOUND * Sound::sounds[numChannels];
FMOD_CHANNEL * Sound::channels[numChannels];

/******** METHODS' IMPLEMENTATIONS ********/

//-------------------------------Initialization--------------------------------

//initialises sound
void Sound::initialise (void) {
   //create the sound system. If fails, sound is set to impossible
   result = FMOD_System_Create(&fmodsystem);
   if (result != FMOD_OK)
   {
      possible = false;
      cout << "Unable to create FMOD System \n";
   }
   //if initialise the sound system. If fails, sound is set to impossible
   if (possible)
   {
      //Initialize the system with #channels = total audio files
      result = FMOD_System_Init(fmodsystem, numChannels, FMOD_INIT_NORMAL, 0);
   }
   if (result != FMOD_OK)
   {
      possible = false;
      cout << "Unable to initialize FMOD System \n";
   }
   //Mute all channels
   if (possible)
   {
      for(int i = 0; i < numChannels; i++)
         FMOD_Channel_SetVolume(channels[i],0.0f);
   }
}

//loads all soundfiles
void Sound::loadAll () {
   if (possible && on) {
      for (int i = 0; i < numChannels; i++) {
         result = FMOD_System_CreateStream(fmodsystem, files[i].c_str(), FMOD_SOFTWARE, 0, &sounds[i]);
         if (result != FMOD_OK)
         {
            cout << "Unable to load " << files[i] << "\n";
            possible = false;
         }
      }
   }
}

//-----------------------------------Jukebox-----------------------------------

unsigned int Sound::nextSong(float volume)
{
   unsigned int duration;
   if (jukeboxIndex == backgroundFiles) {
      jukeboxIndex = 0;
   }
   loadOnBackgroundChannel(jukeboxIndex++);
   play(BACKGROUND_CHANNEL);
   setJukeboxVolume(volume);
   FMOD_Sound_GetLength(sounds[BACKGROUND_CHANNEL], &duration, FMOD_TIMEUNIT_MS);
   return duration;
}

void Sound::pauseJukebox()
{
   togglePause(BACKGROUND_CHANNEL);
}

void Sound::setJukeboxVolume(float volume)
{
   setVolume(channels[BACKGROUND_CHANNEL], volume);
}

//-------------------------------------Walking---------------------------------
void Sound::walk()
{
   FMOD_BOOL paused;
   FMOD_Channel_GetPaused(channels[WALK_CHANNEL], &paused);
   if (paused) {
      play(WALK_CHANNEL);
   }
}

void Sound::stopWalk()
{
   FMOD_Channel_SetPaused(channels[WALK_CHANNEL], true);
}

//---------------------------------Hammer--------------------------------------

void Sound::hammerSmash()
{
   play(HAMMER_CHANNEL);
}

//-----------------------------------Scream------------------------------------

void Sound::scream()
{
   FMOD_BOOL paused;
   FMOD_Channel_GetPaused(channels[SCREAM_CHANNEL], &paused);
   if (paused) {
      FMOD_Channel_SetMode(channels[SCREAM_CHANNEL], FMOD_LOOP_OFF);
      play(SCREAM_CHANNEL);
   }
}

void Sound::stopScream()
{
   FMOD_Channel_SetPaused(channels[SCREAM_CHANNEL], true);
}

//---------------------------------Private Methods------------------------------

//Load a new song onto the background channel
void Sound::loadOnBackgroundChannel(int index)
{
   if (possible) {
      //Release the current sound
      result = FMOD_Sound_Release(sounds[BACKGROUND_CHANNEL]);
      if (result == FMOD_OK) {
         //load the new one
         result = FMOD_System_CreateStream(fmodsystem, backgroundMusic[index].c_str(), FMOD_SOFTWARE, 0, &sounds[BACKGROUND_CHANNEL]);
         if (result != FMOD_OK)
         {
            possible = false;
            cout << "Unable to load: " << backgroundMusic[index] << "\n";
         }
      }
   }
}

//Set the volume of channel to the given value
void Sound::setVolume (FMOD_CHANNEL * channel, float v) {
   if (possible && on && v >= 0.0f && v <= 1.0f) {
      FMOD_Channel_SetVolume(channel,v);
   }
}

//Start playing the sound at index on channel at index
void Sound::play (int index) {
   if (possible && on) {
      result = FMOD_System_PlaySound(fmodsystem, FMOD_CHANNEL_REUSE, sounds[index], false, &channels[index]);
   }
}

//pause or unpause the sound on channel at index
void Sound::setPause (bool pause, int index) {
   FMOD_Channel_SetPaused (channels[index], pause);
}

//turn sound on or off
void Sound::setSound (bool s) {
   on = s;
}

//toggle pause of a channel
void Sound::togglePause (int index) {
   FMOD_BOOL p;
   FMOD_Channel_GetPaused(channels[index],&p);
   FMOD_Channel_SetPaused (channels[index],!p);
}

//tells whether the sound is on or off
bool Sound::getSound (void) {
   return on;
}