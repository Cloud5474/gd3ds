#pragma once
#include "ui_screen.h"
#include "graphics.h"

typedef struct UIScreenPair {
    const char* name;
    //top and bottom screen
    UIScreen screens[2];  
    //whether this ScreenPair will be treated as the backmost ScreenPair of the current UI view (menus such as the main menu, level select, etc; NOT popup ScreenPairs like settings or color select)
    bool anchor;
} UIScreenPair;

typedef enum {
    PUSH_NONE,
    PUSH_NEXT,
    PUSH_AFTER_CLOSE,
    PUSH_ANCHOR
} UIStackPushType;

typedef struct {
    const UIScreenDefPair *defs;
    UIAnimation top_anim;
    UIAnimation btm_anim;
    UIStackPushType type;
    bool push_now;
} UIStackPush;

typedef struct {
    UIScreenPair *screen_stack;
    size_t stack_capacity;

    //"anchor" ScreenPairs represent the lowest screen of the current UI view, such as the main menu, creator menu, or level select (as opposed to popups like settings, the color select, etc)
    //ScreenPairs which are below the current anchor will be unloaded and not updated; reloaded when the current anchor is exited
    size_t current_anchor;
    //amount of ScreenPairs to update (starts at anchor and goes up)
    size_t active_count;

    //used for fade to/from black when changing menus
    UITransitionState transition;
    FadeStatus fade;
    float fade_time;

    //used when not immediately transitioning to a new screen
    UIStackPush push;
} UIStack;

void ui_stack_update(UIInput *input);
void ui_stack_draw(Screens target);

void ui_stack_push(const UIScreenDefPair *defs, UIAnimation top_anim, UIAnimation btm_anim, UIStackPushType type);
void ui_stack_push_name(const char *name, UIAnimation top_anim, UIAnimation btm_anim, UIStackPushType type);

void ui_stack_push_anchor_instant(const UIScreenDefPair *defs);

void ui_stack_pop();

void ui_stack_fini();

UIScreen *ui_stack_get_screen(const char *name, Screens screen);
UIScreen *ui_stack_get_screen_from_pair(UIScreenPair *pair, Screens screen);
UIScreen *ui_stack_get_screen_relative(UIScreen *screen, Screens which);