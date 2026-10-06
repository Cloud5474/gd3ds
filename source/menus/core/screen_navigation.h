#pragma once
#include "menus/core/ui_screen.h"
#include "stdbool.h"

extern bool navigation_switching_screen;
extern bool is_navigating;

void ui_navigation_update(UIInput *input);
void ui_navigation_draw();
void ui_reset_navigation();
void ui_switch_navigation_screen(UIScreen *s);

NavigationEntry get_empty_navigation_entry();
