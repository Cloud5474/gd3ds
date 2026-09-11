#pragma once
#include "menus/core/ui_stack.h"

typedef struct {
    const char *text;
    const char *title;
    const char *cancel_text;
    const char *proceed_text;
    void (*callback_func)(UIScreen *);
} PopUpData;

extern int pop_up_result;

extern const UIScreenDefPair two_option_pop_up_def;