#include "screen_definitions.h"

const UIScreenDefPair soggy_def = {
    .name = "Soggy",
    .top = {
        .path = "romfs:/menus/soggy_top.txt"
    },
    .btm = {
        .path = "romfs:/menus/soggy.txt"
    },
};

const UIScreenDefPair credits_def = {
    .name = "Credits",
    .top = {
        0
    },
    .btm = {
        .path = "romfs:/menus/credits.txt"
    },
};

