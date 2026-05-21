#pragma once

#include <stdbool.h>
#include "player/player.h"
#include "graphics.h"

#define MAX_LEVEL_HEIGHT 2400.f
#define HITBOX_TRAIL_SIZE 64

#define MIRROR_DURATION 0.4f

typedef struct {
    float x;
    float y;
    float width;
    float height;
    float rotation;
    InternalHitbox internal_hitbox;
} PlayerHitboxTrail;

typedef struct {
    u8 pressedJump:1;
    u8 holdJump:1;
} KeyInput;
// First flash
#define FLASH_TIME_1 (20.f / STEPS_HZ)
// Rest of timers
#define FLASH_TIME_2 (14.f / STEPS_HZ)

typedef enum {
    FLASH_NONE,
    FLASH_FIRST_LIGHT,
    FLASH_UNLIGHTED,
    FLASH_SECOND_LIGHT
} FlashState;

typedef struct {
    bool flashing;
    FlashState state;
    float timer;
    bool use_lbg;
} BGFlashData;

typedef struct {
    float camera_x;
    float camera_y;

    float old_camera_x;
    float old_camera_y;

    float camera_intended_y;

    float camera_x_lerp;
    float camera_y_lerp;

    float camera_x_middle;
    float camera_y_middle;

    float intermediate_camera_y;
    
    float camera_wall_timer;
    float camera_wall_initial_y;

    float ground_x;
    float ground_wall_initial_x;

    float background_x;
    float background_wall_initial_x;
    
    Player player;
    Player player2;

    unsigned char current_player;

    Player old_player;

    bool mirroring;
    int mirror_mult;    
    float mirror_timer;

    float original_mirror_factor;
    float intended_mirror_factor;

    float mirror_speed_factor;
    float mirror_factor;

    float ground_y;
    float ceiling_y;
    float ground_y_gfx;

    float dual_portal_y;
    unsigned char speed;

    bool end_wall_anim_playing;
    bool completion_shake;
    
    bool noclip;
    int hitbox_display;
    bool dual;
    bool dead;

    bool profiling;

    float level_progress;

    float timer;
    float death_timer;

    bool hitbox_enabled_when_dead;

    bool custom_level;
    char custom_level_path[256];

    BGFlashData flash_data;

    int last_hitbox_trail;
    PlayerHitboxTrail hitbox_trail_players[2][HITBOX_TRAIL_SIZE];

    KeyInput input;
    KeyInput old_input;
} GameState;

#define CEILING_INVUL_TIME 0.15f

extern GameState state;

void set_intended_ceiling();
void set_gamemode(Player *player, int gamemode);
void set_mini(Player *player, bool mini);
void init_variables();
void run_camera();

void handle_death();

void start_bg_flash();
void handle_bg_flash();
void clear_bg_flash();

extern bool disableHitboxesAfterRespawn;