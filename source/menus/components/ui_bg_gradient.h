#pragma once
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"

#define BG_GRADIENT_XSCALE (400.f / 30)
#define BG_GRADIENT_YSCALE (240.f / 320)

UIImage *ui_create_bg_gradient(const UIScreen *screen);
UIElement *ui_create_bg_gradient_from_props(const UIScreen *screen, const UIPropertyList *props);