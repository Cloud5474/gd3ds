#pragma once
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"

void ui_label_set_text(UILabel *e, const char *text);
UILabel *ui_create_label(const UIScreen *screen);
UIElement *ui_create_label_from_props(const UIScreen *screen, const UIPropertyList *props);