#include <3ds.h>
#include <citro2d.h>

#include "save/saving.h"

#include "menus/core/ui_screen.h"
#include "menus/components/ui_label.h"
#include "menus/components/ui_window_button.h"
#include "menus/creator_menu/online/online_level_menu.h"
#include "menus/creator_menu/online/two_option_pop_up.h"

#include "fonts/goldFont.h"

int pop_up_result = 0;

void action_yes(UIElement* e, const UIPropertyList *args) {
    pop_up_result = 1;
    ui_stack_pop();
}

void action_no(UIElement* e, const UIPropertyList *args) {
    pop_up_result = 2;
    ui_stack_pop();
}

static UIActionDef two_option_pop_up_actions[] = {
    { "yes", action_yes },
    { "no", action_no },
};

static void two_option_pop_up_init(UIScreen *s) {
    PopUpData *pop_up_data = s->pair->data;
    if(!pop_up_data) return;

    UILabel *title = (UILabel *) ui_get_element_by_tag(s, "title");
    UILabel *content = (UILabel *) ui_get_element_by_tag(s, "content");
    UIWindowButton *yes_button = (UIWindowButton *) ui_get_element_by_tag(s, "yesbutton");
    UIWindowButton *no_button = (UIWindowButton *) ui_get_element_by_tag(s, "nobutton");

    ui_label_set_text(title, pop_up_data->title);
    ui_label_set_text(content, pop_up_data->text);

    ui_button_set_text((UIButton *)yes_button, pop_up_data->proceed_text);
    ui_button_set_text((UIButton *)no_button, pop_up_data->cancel_text);

    float yes_button_text_width = get_text_length(&goldFont_fontCharset, yes_button->base.textScale, false, yes_button->base.text) + 20;
    float no_button_text_width = get_text_length(&goldFont_fontCharset, no_button->base.textScale, false, no_button->base.text) + 20;

    yes_button->base.base.w = yes_button_text_width;
    yes_button->base.base.x = 0 + (((yes_button_text_width / 2) + (no_button_text_width / 2)) / 2) - 4;

    no_button->base.base.w = no_button_text_width;
    no_button->base.base.x = 0 - (((yes_button_text_width / 2) + (no_button_text_width / 2)) / 2) + 4;

    pop_up_result = 0;
}

static void two_option_pop_up_free_data(void *data){
    PopUpData *pop_up_data = data;
    if(!pop_up_data) return;
    free((void *)pop_up_data->text);
    free((void *)pop_up_data->title);
    free((void *)pop_up_data->proceed_text);
    free((void *)pop_up_data->cancel_text);
    free(pop_up_data);
}

static void two_option_pop_up_exit(UIScreen *s){
    // PopUpData *pop_up_data = s->pair->data;
    // if (pop_up_data->callback_func){
    //     pop_up_data->callback_func(s);
    // }
}

const UIScreenDefPair two_option_pop_up_def = {
    .name = "two_option_popup",
    .btm = {
        .path = "romfs:/menus/creator_menu/online/two_option_pop_up.txt",
        .init = two_option_pop_up_init,
        .exit = two_option_pop_up_exit,
        .action_list = {
            .action_count = ARRAY_LEN(two_option_pop_up_actions),
            .actions = two_option_pop_up_actions
        }
    },
    .free_data = two_option_pop_up_free_data,
};