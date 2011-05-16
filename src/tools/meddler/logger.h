#pragma once

#ifndef NDEBUG
#define L(...) __VA_ARGS__
#else
#define L(...)
#endif
