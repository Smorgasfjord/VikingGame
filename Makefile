CC=g++
CFLAGS=-g -std=c++11 -c -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL
LDFLAGS=-g -std=c++11 -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL -lfmodex64
SOURCES=Audio/Sound.cpp Audio/Jukebox.cpp Utils/chunks.cpp Utils/CMeshLoaderSimple.cpp Utils/World.cpp Components/Bjorn.cpp Components/Hammer.cpp Components/Mountain.cpp Components/Platform.cpp Models/Model.cpp Utils/GLSL_helper.cpp Components/GameObject.cpp Models/GameModel.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=smorgasfjord

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf *.o $(EXECUTABLE)

