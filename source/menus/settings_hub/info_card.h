#pragma once
#include <3ds.h>

typedef struct {
    bool copied;
    const char *text;
} InfoCardData;

extern const UIScreenDefPair info_card_def;