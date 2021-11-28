#include "main.h"
const uint max_fps = 100;
const uint kStandardResizeOfs = 3;
const uint kStandardTitlebarHeight = 34;
const uint kStandardButtonWidth = 25;
const uint kStandardFrameWidth = kStandardButtonWidth / 10;
const Rectangle kStandardFramedButtonPos = {{kStandardFrameWidth, kStandardFrameWidth}, kStandardButtonWidth - 2 * kStandardFrameWidth, kStandardButtonWidth - 2 * kStandardFrameWidth};
const Rectangle kStandardMoveBounds = {{-5000, kStandardTitlebarHeight}, 5000 * 2 + 2000, 5000};