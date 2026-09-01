#include <3ds.h>
#include <citro2d.h>
#include "menus/core/ui_element.h"
#include "menus/core/ui_screen.h"
#include "menus/components/ui_list.h"
#include "main.h"
#include "mp3_player.h"
#include "graphics.h"

#include "save/config.h"

bool gotSogged = false;

void soggy_menu_loop(UIScreen *s) {
    gotSogged = true;
    cfg_save(); // You got sogged

    stop_mp3();
    play_mp3("romfs:/songs/SogLoop.mp3", true, 0);
}