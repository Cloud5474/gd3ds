#include "ui_scene.h"
#include "ui_screen.h"

static UIStack stack = {
    .stack = {0},
    .scene_count = 0,
    .next = { 0 },
    .fade_time = 0.f,
    .transition = UI_TRANSITION_NONE
};

void ui_stack_update(UIInput *input){
    for(int i = 0; i < stack.scene_count; i++){

    }
}
void ui_stack_draw(UIScreenDrawTarget target){

}
void ui_stack_push(
    const UIScreenDefinition* top_def, 
    const UIScreenDefinition* btm_def, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
){

}
void ui_stack_clear_and_push_next(
    const UIScreenDefinition* top_def, 
    const UIScreenDefinition* btm_def, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
){
    
}
void ui_stack_pop(){
    for(int i = 0; i < 2; i++){
        ui_screen_close(&stack.stack[stack.scene_count].screens[i]);
    }
}