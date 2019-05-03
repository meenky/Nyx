#pragma once

#define NYX_BUILD_VERSION(a,b,c) (((a) << 24) | ((b) << 16) | (c))
#define NYX_VERSION_INT    NYX_BUILD_VERSION(0, 0, 2)

#define NYX_MAJOR_VERSION(i) ((i >> 24) & 0xFF)
#define NYX_MINOR_VERSION(i) ((i >> 16) & 0xFF)
#define NYX_MICRO_VERSION(i) (i & 0xFFFF)

#define NYX_VERSION_STRING "0.0.2"

#include <nyx/plan.h>
#include <nyx/plugin.h>
#include <nyx/registry.h>

