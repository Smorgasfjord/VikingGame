//
//  Jukebox.h
//  MyGame
//
//  Created by Taylor Woods on 5/13/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __MyGame__Jukebox__
#define __MyGame__Jukebox__

#include <iostream>
#include "Sound.h"

class Jukebox
{
   private:
      float volume;
   public:
      Jukebox();
      void start();
      void pause();
      void volumeUp();
      void volumeDown();
};

#endif /* defined(__MyGame__Jukebox__) */
