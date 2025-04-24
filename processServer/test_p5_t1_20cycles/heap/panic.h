#pragma once

extern void panic(const char* msg, ...);

#define MISSING() panic("*** code missing at %s:%d\n", __FILE__, __LINE__)

