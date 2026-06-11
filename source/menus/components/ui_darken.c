#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

static void ui_darken_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void ui_darken_draw(UIElement* e) {
    C2D_SpriteSetPos(&e->darken.sprite, e->x, e->y);
    C2D_SpriteSetScale(&e->darken.sprite, e->w/16.f, e->h/16.f);
    C2D_DrawSpriteTinted(&e->darken.sprite, &e->darken.tint);
}

void ui_darken_reset_opacity(UIElement* e){
    C2D_PlainImageTint(&e->darken.tint, C2D_Color32f(0, 0, 0, e->opacity), 1.0f);
}

UIElement ui_create_darken(float x, float y, float width, float height, float opacity, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};

    e.type = UI_DARKEN;
    e.x = x;
    e.y = y;
    e.w = width;
    e.h = height;
    e.enabled = true;
    e.opacity = opacity;

    // Copy tag
    copy_tag_array(&e, tag);

    C2D_PlainImageTint(&e.darken.tint, C2D_Color32f(0, 0, 0, opacity), 1.0f);
    C2D_SpriteFromSheet(&e.darken.sprite, ui_sheet, 416);
    C2D_SpriteSetCenter(&e.darken.sprite, 0.5f, 0.5f);

    e.update = ui_darken_update;
    e.draw = ui_darken_draw;

    return e;
}