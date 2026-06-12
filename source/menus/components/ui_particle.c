#include "ui_element.h"
#include "particles/particle_definitions.h"
#include <citro2d.h>
#include "ui_screen.h"
#include "graphics.h"

void ui_particle_set_pos(UIElement* e, float x, float y){
    ParticleSystem *p = &(e->particle.particle);
    p->emitterX = x;
    p->emitterY = y;
}

void ui_particle_emit(UIElement* e, int emitCount){
    spawnMultipleParticles(&(e->particle.particle), emitCount);
}

static void ui_particle_update(UIElement* e, UIInput* touch) {
    ParticleSystem *p = &(e->particle.particle);
    updateParticleSystem(p, delta);
}

static void ui_particle_draw(UIElement* e) {
    change_blending(true);
    drawParticleSystem(&(e->particle.particle), 0, 0, 1);
    change_blending(false);
}

UIElement ui_create_particle(int x, int y, float scale, float r, float g, float b, ParticleDefinition* const def, char (*tag)[TAG_LENGTH]) {
    UIElement e = {0};
    ParticleSystem *p = &(e.particle.particle);

    initParticleSystem(p, def);

    e.type = UI_PARTICLE;
    e.x = x;
    e.y = y;
    e.enabled = true;
    
    //Do not override colors if invalid
    if(r >= 0.f && g >= 0.f && b >= 0.f && r <= 1.f && g <= 1.f && b <= 1.f){
        p->cfg.startColorRed = r;
        p->cfg.startColorGreen = g;
        p->cfg.startColorBlue = b;
        p->cfg.finishColorRed = r;
        p->cfg.finishColorGreen = g;
        p->cfg.finishColorBlue = b;
    }

    p->emitterX = x;
    p->emitterY = y;
    p->scale = scale;
    p->affectedByMirror = false;
    p->stationary = true;

    e.update = ui_particle_update;
    e.draw = ui_particle_draw;

    copy_tag_array(&e, tag);

    add_ui_particle_system(p);

    return e;
}