// declaring all skins
#define DEFINE_SKIN(Scalability, Name, file_name)     \
  Texture* kTexture##Name;                            \
  DrawFunctor::Scalability##Texture* kFuncDraw##Name; \
  DrawFunctor::MultipleFunctors* kFuncDraw##Name##Framed;
  #include "../include/DEFINE_SKIN.h"
#undef DEFINE_SKIN