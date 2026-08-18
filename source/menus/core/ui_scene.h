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
    UIScene stack[8];
    size_t scene_count;
    
    //used for fade to/from black when changing menus
    UIScene next;
    UITransitionState transition;
    float fade_time;
} UIStack;

void ui_stack_update(UIInput *input);
void ui_stack_draw(UIScreenDrawTarget target);
void ui_stack_push(
    const UIScreenDefinition* top_def, 
    const UIScreenDefinition* btm_def, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
);
void ui_stack_clear_and_push_next(
    const UIScreenDefinition* top_def, 
    const UIScreenDefinition* btm_def, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
);
void ui_stack_pop();