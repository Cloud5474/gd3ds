#pragma once
#include "ui_screen.h"
#include "graphics.h"

typedef struct UIScene {
    const char* name;
    //top and bottom screen
    UIScreen screens[2];  
    //whether this scene will be treated as the backmost scene of the current UI view (menus such as the main menu, level select, etc; NOT popup scenes like settings or color select)
    bool anchor;
} UIScene;

typedef struct {
    const char *name;
    UIScreenDefinition top;
    UIScreenDefinition btm;
} UIScreenDefPair;

typedef struct {
    UIScene *scenes;
    size_t stack_capacity;

    //"anchor" screens represent the lowest screen of the current UI view, such as the main menu, creator menu, or level select (as opposed to popups like settings, the color select, etc)
    //screens which are below the current anchor will be unloaded and not updated; reloaded when the current anchor is exited
    size_t current_anchor;
    //amount of scenes to update (starts at anchor and goes up)
    size_t active_scene_count;

    //used for fade to/from black when changing menus
    UITransitionState transition;
    FadeStatus fade;
    float fade_time;
} UIStack;

void ui_stack_update(UIInput *input);
void ui_stack_draw(Screens target);

void ui_stack_push(
    const UIScreenDefPair* defs, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
);
void ui_stack_push_anchor(const UIScreenDefPair* defs, bool instant);
void ui_stack_pop();

void ui_stack_fini();

void test_loop();