CXX=g++
CPPFLAGS= -g -std=c++0x \
    -I /home/nclarke/Desktop/deps/glm/include \
    -I /home/nclarke/Desktop/deps/glew/include \
    -I /home/nclarke/Desktop/deps/glfw/include \
    -I /home/nclarke/Desktop/deps/stb_image/include \
    -I /home/nclarke/Desktop/deps/freetype/include/freetype \
    -D_COMPILE_WITHOUT_OVR
LDFLAGS=-lGL -lGLU \
    -L /home/nclarke/Desktop/deps/lib \
    -lfreetype -lGLEW -lglfw3 \
SRCS=$(shell find . -name "*.cpp")
OBJECTS=$(SRCS:.c=.o)
EXECUTABLE=smorgasfjord

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) 

clean:
	-rm -rf *.o $(EXECUTABLE)

remake: clean all

