#pragma once

#include <string>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 600
#define DATA_DIRECTORY std::string("${TINY_SOURCE_DIR}/data/")

#ifndef NDEBUG
#cmakedefine DEBUG
#endif

