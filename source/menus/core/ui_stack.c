#include "ui_stack.h"
#include "ui_screen.h"

static UIStack stack = { 0 };

void free_scene(){
    
}

//eventually the stack will be dynamically allocated but not yet
void ui_stack_init(){

}
void ui_stack_fini(){

}

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
        ui_screen_close(&stack.scenes[stack.scene_count].screens[i]);
    }
    stack.scene_count--;
}

void test_loop(){

}