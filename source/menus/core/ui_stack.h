#pragma once
#include "ui_screen.h"

typedef enum {
    SCREEN_TOP,
    SCREEN_BOTTOM
} UIScreenDrawTarget;

typedef struct {
    //top and bottom screen
    UIScreen screens[2];  
} UIScene;

typedef struct {
    UIScene scenes[8];
    size_t scene_count;
    size_t stack_capacity;
    
    //tells whether the next screen will be an "anchor" screen; basically just the old default_screen but stackified
    //"anchor" screens represent the lowest screen of the current UI view, such as the main menu, creator menu, or level select (as opposed to popups like settings, the color select, etc)
    bool next_anchor;
    //screens which are below the current anchor will be unloaded and not updated; reloaded when the current anchor is exited
    int current_anchor;

    //used for fade to/from black when changing menus
    UITransitionState transition;
} UIStack;

void ui_stack_init();
void ui_stack_fini();
void ui_stack_update(UIInput *input);
void ui_stack_draw(UIScreenDrawTarget target);
void ui_stack_push(
    const UIScreenDefinition* top_def, 
    const UIScreenDefinition* btm_def, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
);
//sets the next screen to be pushed to be an "anchor" screen
void ui_stack_next_anchor();
void ui_stack_pop();

void test_loop();