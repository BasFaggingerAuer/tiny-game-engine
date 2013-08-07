#pragma once

#include <string>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 800
#define DATA_DIRECTORY std::string("${TINY_SOURCE_DIR}/data/")

#ifndef NDEBUG
#cmakedefine DEBUG
#endif

