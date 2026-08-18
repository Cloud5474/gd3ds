#pragma once
#include "menus/core/ui_element.h"
#include "ui_button.h"


#define ICON_HOVER_SCALE BUTTON_HOVER_SCALE
#define ICON_HOVER_ANIM_TIME BUTTON_HOVER_ANIM_TIME

void ui_icon_set_gamemode_index(UIIcon *e, int gamemode, int index);
UIIcon *ui_create_icon(const UIScreen *screen);
UIElement *ui_create_icon_from_props(const UIScreen *screen, const UIPropertyList *props);