Flags = -std=c++17 -g -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-switch -O2

CXXFLAGS = $(Flags) $(pkg-config sdl2 SDL2_ttf SDL2_image --cflags)
LXXFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image #$(pkg-config sdl2 SDL2_ttf SDL2_image --libs)

Include = include
Src = src
Bin = bin

Cpp = $(notdir $(wildcard $(Src)/*.cpp))
Headers = $(Include)/List.h
Objects = $(addprefix $(Bin)/, $(Cpp:.cpp=.o))

out: $(Objects)
	g++ -o out $(Objects) $(LXXFLAGS)

vpath %.cpp $(Src)
$(Bin)/%.o: %.cpp $(Headers) Makefile
	g++ -c $< $(CXXFLAGS) -o $@
