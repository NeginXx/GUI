#include "main.h"
static const char* kFontsDir = "fonts";
static const char* kSkinsDirName = "skins";
static const char* kPluginsDirName = "plugins";

static const char* kFontName = "OpenSans-Bold.ttf";
static const uint kFontSize = 20;
static const uint kFontHeight = 28;
static const uint kTextWidthOfs = 5;
static const uint kTextHeightOfs = 3;

static const uint kStandardResizeOfs = 3;
static const uint kStandardButtonHeight = kFontHeight + 2 * kTextHeightOfs;
static const uint kStandardTitlebarHeight = kStandardButtonHeight;
static const uint kStandardButtonWidth = 25;
static const uint kStandardFrameWidth = kStandardButtonWidth / 10;
static const Rectangle kStandardMoveBounds = {{-5000, kStandardTitlebarHeight}, 5000 * 2 + 2000, 5000};
static const uint max_fps = 100;