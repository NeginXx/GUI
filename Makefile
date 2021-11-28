Compiler = clang++

Flags = -std=c++17 -O2 -Wall \
-Wextra -Wpedantic -Wno-unused-parameter \
-Wno-dollar-in-identifier-extension     \
-Wno-unused-variable -Wno-switch -fsanitize=address -g

CXXFLAGS = $(Flags) -I/usr/include/SDL2
LXXFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image -fsanitize=address

Include = include
Src = src
Bin = bin

Cpp = $(notdir $(wildcard $(Src)/*.cpp))
Headers = $(Include)/List.h
Objects = $(addprefix $(Bin)/, $(Cpp:.cpp=.o))

out: $(Objects)
	$(Compiler) -o out $(Objects) $(LXXFLAGS)


vpath %.cpp $(Src)
$(Bin)/%.o: %.cpp $(Headers) Makefile
	$(Compiler) -c $< $(CXXFLAGS) -o $@

# .PHONY: run
# run:
# 	ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-10/bin/llvm-symbolizer ./out
