#pragma once
#define DEFINE_SKIN(Scalability, Name, file_name)     \
  extern Texture* kTexture##Name;                            \
  extern DrawFunctor::Scalability##Texture* kFuncDraw##Name; \
  extern DrawFunctor::MultipleFunctors* kFuncDraw##Name##Framed;
  #include "../include/DEFINE_SKIN.h"
#undef DEFINE_SKIN