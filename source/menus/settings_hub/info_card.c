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

static bool yes_exit = false;

static UIScreen screen = {
    .isBottom = true
};
static UILabel *content;

void set_info_content(const char *text) {
    content = (UILabel *) ui_get_element_by_tag(&screen, "content");
    ui_label_set_text(content, text);
}

void info_card_init() {
    ui_load_screen_old(&screen, NULL, 0, "romfs:/menus/setting_hub/info_card.txt");
    ui_screen_open(&screen, ANIM_ZOOM);
    yes_exit = false;
}

int info_card_loop() {
    if (yes_exit) {
        ui_unload_screen(&screen);
        return true;
    }

    UIInput touch;
    touchPosition touchPos;
    hidTouchRead(&touchPos);
    touch.touchPosition = touchPos;
    touch.interacted = false;
    ui_screen_update(&screen, &touch);

    ui_screen_draw(&screen);

    return false;
}
