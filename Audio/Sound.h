//
//  Sound.h
//  MyGame
//
//  Created by Taylor Woods on 5/12/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//  Taken from http://www.roguebasin.com/index.php?title=Implementing_sound_in_C_and_C_Plus_Plus
//

#ifndef __MyGame__Sound__
#define __MyGame__Sound__

#include "inc/fmod.h" //FMOD Ex
#include <vector>

/******** CLASS DEFINITION ********/

class Sound {
private:
   static bool on; //is sound on?
   static bool possible; //is it possible to play sound?
   //FMOD-specific stuff
   static FMOD_RESULT result;
   static FMOD_SYSTEM * fmodsystem;
   static FMOD_SOUND * sounds[];
   static FMOD_CHANNEL * channels[];
   
   static void loadOnBackgroundChannel(int backgroundMusicIndex);
   static void play (int index); //plays a sound (may be started paused; no argument for unpaused)
   
   //sound control
   static void setVolume (FMOD_CHANNEL * channel, float v); //sets the actual playing sound's volume
   
   //getters
   static bool getSound (void); //checks whether the sound is on
   
   //setters
   static void setPause (bool pause, int index); //pause or unpause the sound
   static void setSound (bool sound); //set the sound on or off
   
   //toggles
   static void togglePause (int index); //toggle pause on/off
   
public:
   //Initialization
   static void initialise (void); //initialises sound
   static void loadAll(); //loads all soundfiles
   
   //Background music jukebox
   static unsigned int nextSong(float volume);
   static void pauseJukebox();
   static void setJukeboxVolume(float volume);

   static void toggleZoeMode();

   //Walk sounds
   static void walk();
   static void stopWalk();
   
   //Hammer
   static void hammerSmash(float v);
   static void pickStrike(float v);
   
   //Scream
   static void scream();
   static void stopScream();
};

#endif /* defined(__MyGame__Sound__) */
