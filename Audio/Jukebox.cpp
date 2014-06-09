//
//  Jukebox.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/13/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Jukebox.h"
#include <functional>
#include <chrono>
#include <future>
#include <iostream>
#include <cstdio>
using namespace std;

float volume;

//-----------------------------Timer Callback Class----------------------------
class later
{
public:
   template <class callable, class... arguments>
   later(int after, bool async, callable&& f, arguments&&... args)
   {
      std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
      if (async)
      {
         std::thread([after, task]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(after));
            task();
         }).detach();
      }
      else
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(after));
         task();
      }
   }
   
};

void nextSong()
{
   int msDuration = Sound::nextSong(volume);
   //Will play the next song after the duration has passed
   later playNext(msDuration, true, &nextSong);
   return;
}

//------------------------------Jukebox Stuff-----------------------------------

Jukebox::Jukebox()
{
   volume = 0.3f;
}

void Jukebox::start()
{
   Sound::initialise();
   Sound::loadAll();
   nextSong();
   Sound::setJukeboxVolume(volume);
   return;
}

void Jukebox::pause()
{
   Sound::pauseJukebox();
}

void Jukebox::volumeDown()
{
   if (volume > 0)
      volume -= .05;
   Sound::setJukeboxVolume(volume);
}

void Jukebox::volumeUp()
{
   if (volume < 1)
      volume += .05;
   Sound::setJukeboxVolume(volume);
}