#pragma once

#include <string>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define DATA_DIRECTORY std::string("${TINY_SOURCE_DIR}/data/")

#ifndef NDEBUG
#cmakedefine DEBUG
#endif

