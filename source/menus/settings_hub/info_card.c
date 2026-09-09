#include <3ds.h>
#include <citro2d.h>

#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "math_helpers.h"

#include "menus/core/ui_screen.h"
#include "menus/components/ui_list.h"
#include "menus/components/ui_window.h"
#include "menus/components/ui_textbox.h"
#include "menus/components/ui_image.h"
#include "menus/components/ui_label.h"

#include "menus/level_select.h"
#include "menus/main_menu.h"
#include "menus/settings_hub/info_card.h"

#include "fonts/goldFont.h"
#include "fonts/chatFont.h"
#include "fonts/bigFont.h"

void set_info_content(const char *text, UIScreen *s) {
    UILabel *content = (UILabel *) ui_get_element_by_tag(s, "content");
    ui_label_set_text(content, text);
}

void info_card_init(UIScreen *s) {
    InfoCardData *info_card_data = s->pair->data;
    if(!info_card_data) return;
    set_info_content(info_card_data->text, s);
}

void info_card_free_data(void *data){
    InfoCardData *info_card_data = data;
    if(info_card_data) free(info_card_data);
}

const UIScreenDefPair info_card_def = {
    .name = "info_card",
    .btm = {
        .path = "romfs:/menus/settings_hub/info_card.txt",
        .init = info_card_init
    },
    .free_data = info_card_free_data
};

