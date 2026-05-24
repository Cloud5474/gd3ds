#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"

static void ui_progress_bar_update(UIElement* e, UIInput* touch) {
    bool inside = touch->touchPosition.px >= e->x - (e->w / 2) && touch->touchPosition.px < e->x + (e->w / 2) &&
                  touch->touchPosition.py >= e->y - (e->h / 2) && touch->touchPosition.py < e->y + (e->h / 2);
    
    // Mask background elements
    if (inside) touch->did_something = true;
}

static void draw_frame(UIElement *e) {
    C2D_SpriteSetCenter(&e->progress_bar.sprite_frame, 0.5f, 0.5f);
    C2D_SpriteSetPos(&e->progress_bar.sprite_frame, e->x, e->y);
    C2D_SpriteSetScale(&e->progress_bar.sprite_frame, e->progress_bar.scale, e->progress_bar.scale);
    C2D_DrawSpriteTinted(&e->progress_bar.sprite_frame, &e->progress_bar.tint_frame);
}

static void draw_bar(UIElement *e) {
    float bar_width = e->progress_bar.sprite.image.subtex->width;
    
    int pixels = (e->progress_bar.value / e->progress_bar.max_value) * bar_width;

    if (pixels > 0) {
        if (pixels >= bar_width) {
            pixels = bar_width - 1;
        }

        C2D_Sprite spr;
        Tex3DS_SubTexture sub;
        C2D_Image img;

        sub = select_box(&e->progress_bar.sprite.image, 0, 0, pixels, e->progress_bar.sprite.image.subtex->height);
        img = e->progress_bar.sprite.image; img.subtex = &sub;
        C2D_SpriteFromImage(&spr, img);
        C2D_SpriteSetCenter(&spr, 0.f, 0.5f);
        C2D_SpriteSetPos(&spr, e->x - e->w / 2, e->y);
        C2D_SpriteSetScale(&spr, e->progress_bar.scale, e->progress_bar.scale);
        if (e->progress_bar.useTint) {
            C2D_DrawSpriteTinted(&spr, &e->progress_bar.tint);
        } else {
            C2D_DrawSprite(&spr);
        }
    }
}

static void ui_progress_bar_draw(UIElement* e) {
    if (e->progress_bar.flip_order) {
        draw_bar(e);
        draw_frame(e);
    } else {
        draw_frame(e);
        draw_bar(e);
    }
    
}

void ui_progress_bar_set_tint(UIElement* e, u32 color) {
    if (e->type != UI_PROGRESS_BAR) return;

    C2D_PlainImageTint(&e->progress_bar.tint, color, 1.0f);
    e->progress_bar.useTint = true;
}

void ui_progress_bar_clear_tint(UIElement* e) {
    if (e->type != UI_PROGRESS_BAR) return;
    
    e->progress_bar.useTint = false;
}

void ui_progress_bar_set_images(UIElement *e, int style, float scale) {
    switch (style) {
        case 0:
            C2D_SpriteFromSheet(&e->progress_bar.sprite, bar_sheet, 0);
            C3D_TexSetFilter(e->progress_bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->progress_bar.tint_frame, C2D_Color32(0, 0, 0, 127), 1.0f);

            e->progress_bar.sprite_frame = e->progress_bar.sprite;

            e->w = e->progress_bar.sprite.image.subtex->width * scale;
            e->h = e->progress_bar.sprite.image.subtex->height * scale;

            e->progress_bar.flip_order = false;
            break;
        case 1:
            C2D_SpriteFromSheet(&e->progress_bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->progress_bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->progress_bar.sprite_frame, bar_sheet, 2);
            C3D_TexSetFilter(e->progress_bar.sprite_frame.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->progress_bar.tint_frame, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->w = e->progress_bar.sprite.image.subtex->width * scale;
            e->h = e->progress_bar.sprite.image.subtex->height * scale;

            e->progress_bar.flip_order = true;
            break;
        case 2:
            C2D_SpriteFromSheet(&e->progress_bar.sprite, bar_sheet, 3);
            C3D_TexSetFilter(e->progress_bar.sprite.image.tex, GPU_LINEAR, GPU_LINEAR);

            C2D_SpriteFromSheet(&e->progress_bar.sprite_frame, bar_sheet, 1);
            C3D_TexSetFilter(e->progress_bar.sprite_frame.image.tex, GPU_LINEAR, GPU_LINEAR);
            
            C2D_PlainImageTint(&e->progress_bar.tint_frame, C2D_Color32(255, 255, 255, 255), 1.0f);

            e->w = e->progress_bar.sprite.image.subtex->width * scale;
            e->h = e->progress_bar.sprite.image.subtex->height * scale;

            e->progress_bar.flip_order = true;
            break;
    }
}

UIElement ui_create_progress_bar(int x, int y, int style, float scale, float max_value, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};

    e.type = UI_PROGRESS_BAR;
    e.x = x;
    e.y = y;
    e.enabled = true;
    e.progress_bar.useTint = false;

    e.progress_bar.style = style;

    // Copy tag
    copy_tag_array(&e, tag);

    e.progress_bar.scale = scale;

    ui_progress_bar_set_images(&e, style, scale);

    // Please no divisions by zero, thanks
    e.progress_bar.max_value = (max_value == 0 ? 100 : max_value);
    e.progress_bar.value = 0;

    e.update = ui_progress_bar_update;
    e.draw = ui_progress_bar_draw;

    return e;
}