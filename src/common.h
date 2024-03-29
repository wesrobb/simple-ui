#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define array_size(a) (sizeof(a) / sizeof((a)[0]))
