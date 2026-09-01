#include "ui_element.h"
#include "ui_actions.h"

const UIIntEnumEntry anim_enum[] = {
    { "none", ANIM_NONE },
    { "slide_down", ANIM_SLIDE_DOWN },
    { "slide_right", ANIM_SLIDE_RIGHT },
    { "zoom", ANIM_ZOOM },
    { "zoom_subtle", ANIM_ZOOM_SUBTLE },
};

const UIIntEnumEntry screen_enum[] = {
    { "top", SCREEN_TOP },
    { "btm", SCREEN_BTM }
};

static void action_open_menu(UIElement *e, const UIPropertyList *args){
    bool anchor = ui_prop_bool(args, "anchor", false);
    const UIScreenDefPair *def = ui_get_screen_def(ui_prop_string(args, "screen", "soggy"));
    if(anchor){
        ui_stack_push_anchor(def, false);
    } else{
        UIAnimation top_anim = ui_prop_int_enum(args, "top_anim", anim_enum, ARRAY_LEN(anim_enum), ANIM_NONE);
        UIAnimation btm_anim = ui_prop_int_enum(args, "btm_anim", anim_enum, ARRAY_LEN(anim_enum), ANIM_NONE);
        ui_stack_push(def, top_anim, btm_anim);
    }
}

static void action_close_menu(UIElement *e, const UIPropertyList *args){
    ui_stack_pop();
}

static void action_disable_tag(UIElement *e, const UIPropertyList *args){
    int screen = ui_prop_int_enum(args, "screen", screen_enum, ARRAY_LEN(screen_enum), -1);
    if(screen >= 0 && screen < 2){
        ui_run_func_on_tag(&e->screen->scene->screens[screen], ui_prop_string(args, "tag", ";,[]{}Nope"), ui_disable_element);
        return;
    }
    ui_run_func_on_tag(e->screen, ui_prop_string(args, "tag", "unused"), ui_disable_element);
}

static void action_enable_tag(UIElement *e, const UIPropertyList *args){
    int screen = ui_prop_int_enum(args, "screen", screen_enum, ARRAY_LEN(screen_enum), -1);
    if(screen >= 0 && screen < 2){
        ui_run_func_on_tag(&e->screen->scene->screens[screen], ui_prop_string(args, "tag", ";,[]{}Nope"), ui_enable_element);
        return;
    }
    ui_run_func_on_tag(e->screen, ui_prop_string(args, "tag", "unused"), ui_enable_element);
}

static void action_play_sound(UIElement *e, const UIPropertyList *args){
    play_sfx(get_sound(ui_prop_string(args, "sound", "boop")), ui_prop_int(args, "channel", 1));
}

const UIActionDef base_actions[] = {
    { "open_menu", action_open_menu },
    { "close_menu", action_close_menu },
    { "disable_tag", action_disable_tag },
    { "enable_tag", action_enable_tag },
    { "play_sound", action_play_sound }
};
