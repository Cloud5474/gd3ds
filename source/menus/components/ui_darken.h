#pragma once
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"

void ui_darken_set_opacity(UIDarken* e, float opacity);
void ui_darken_reset_opacity(UIDarken *e);
UIDarken *ui_create_darken(const UIScreen *screen);
UIElement *ui_create_darken_from_props(const UIScreen *screen, const UIPropertyList *props);