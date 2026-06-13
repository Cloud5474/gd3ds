#include "ui_element.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "graphics.h"
#include "particles/circles.h"

void ui_use_effect_clear(UIElement* e){
    UseEffectPool *pool = &(e->use_effect.useEffects);
    clear_use_effects(pool);
}

void ui_use_effect_update_pos(UIElement* e) {
    UseEffectPool *pool = &(e->use_effect.useEffects);
    for(int i = 0; i < MAX_USE_EFFECTS; i++){
        pool->pool[i].x = e->x + e->use_effect.xPos[i];
        pool->pool[i].y = e->y + e->use_effect.yPos[i];
    }
}

UseEffect *ui_add_use_effect(UIElement* e, float x, float y, const UseEffectDefinition *def) {
    UseEffectPool *pool = &(e->use_effect.useEffects);
    UseEffect *effect = add_use_effect(x, y, USE_EFFECT_OBJ_NOTHING, def, pool);
    e->use_effect.xPos[effect->index] = x - e->x;
    e->use_effect.yPos[effect->index] = y - e->y;
    return effect;
}

void ui_set_use_effect_col(UseEffect *effect, float r, float g, float b) {
    effect->def.colorR = r;
    effect->def.colorG = g;
    effect->def.colorB = b;
}

static void ui_use_effect_update(UIElement* e, UIInput* touch) {
    UseEffectPool *pool = &(e->use_effect.useEffects);
    update_use_effects(delta, pool);
}

static void ui_use_effect_draw(UIElement* e) {
    UseEffectPool *pool = &(e->use_effect.useEffects);
    change_blending(true);
    draw_use_effects(pool);
    change_blending(false);
}

UIElement ui_create_use_effect(char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};
    UseEffectPool *pool = &(e.use_effect.useEffects);
    pool->stationary = true;
    for (size_t i = 0; i < MAX_USE_EFFECTS; i++) {
        pool->pool[i].active = false;
        e.use_effect.xPos[i] = 0;
        e.use_effect.yPos[i] = 0;
    }

    e.x = 0;
    e.y = 0;

    e.type = UI_USE_EFFECT;
    e.enabled = true;

    e.update = ui_use_effect_update;
    e.draw = ui_use_effect_draw;

    copy_tag_array(&e, tag);

    return e;
}