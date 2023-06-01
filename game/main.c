#include "flecs.h"
#include <SDL.h>
#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

ecs_entity_t input;

typedef struct Position {
    float x;
    float y;
} Position;
ECS_COMPONENT_DECLARE(Position);

typedef struct Sprite {
    SDL_Texture* texture;
    int width;
    int height;
    bool visible;
} Sprite;
ECS_COMPONENT_DECLARE(Sprite);

typedef struct Movable {
    bool is_grabbed;
    float offset_x; // Offset between mouse cursor and sprite's top-left corner
    float offset_y;
} Movable;
ECS_COMPONENT_DECLARE(Movable);


typedef struct Agent {
    char* name;
} Agent;
ECS_COMPONENT_DECLARE(Agent);

typedef struct Stats {
    int read;
    int lut;
} Stats;
ECS_COMPONENT_DECLARE(Stats);

typedef struct EventMouseMotion {
    int x, y;
} EventMouseMotion;
ECS_COMPONENT_DECLARE(EventMouseMotion);

typedef struct EventMouseClick {
    int x, y;
    Uint8 button; // 1: left, 2: middle, 3: right, etc.
    Uint8 state;
} EventMouseClick;
ECS_COMPONENT_DECLARE(EventMouseClick);

typedef struct ConsumeEvent {
    bool mock;
} ConsumeEvent;
ECS_COMPONENT_DECLARE(ConsumeEvent);

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void Render(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 1);
    Sprite *s = ecs_field(it, Sprite, 2);

    for (int i = 0; i < it->count; i++) {
        SDL_Rect dst;
        dst.x = (int)p[i].x;
        dst.y = (int)p[i].y;
        dst.w = s[i].width;
        dst.h = s[i].height;
        SDL_RenderCopy(renderer, s[i].texture, NULL, &dst);
    }
}

void Input(ecs_iter_t *it) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            ecs_quit(it->world);
        } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            ecs_quit(it->world);
        } else if (e.type == SDL_MOUSEMOTION) {
            ecs_set(it->world, input, EventMouseMotion, {e.motion.x, e.motion.y});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventMouseMotion), {});
        } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            ecs_set(it->world, input, EventMouseClick, {e.button.x, e.button.y, e.button.button, e.button.state});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventMouseClick), {});
        }
    }
}


void MouseMovableSelection(ecs_iter_t *it) {
    Movable *m = ecs_field(it, Movable, 1);
    Position *p = ecs_field(it, Position, 2);
    Sprite* s = ecs_field(it, Sprite, 3);
    EventMouseClick *click_event = ecs_field(it, EventMouseClick, 4);

    for (int i = 0; i < it->count; i++) {
        if (click_event->button == SDL_BUTTON_LEFT) {
            bool overlaps_sprite = click_event->x >= p[i].x && click_event->x <= p[i].x + s[i].width &&
                click_event->y >= p[i].y && click_event->y <= p[i].y + s[i].height;
            if (click_event->state == SDL_PRESSED) {
                if (overlaps_sprite)
                {
                    m[i].is_grabbed = true;
                    m[i].offset_x = click_event->x - p[i].x; // Remember the offset
                    m[i].offset_y = click_event->y - p[i].y;
                    printf("Mouse grabbed movable!\n");
                }
            } else {
                if (m[i].is_grabbed)
                {
                    printf("Mouse released movable!\n");
                }
                m[i].is_grabbed = false;
            }
        }
    }
}

void MouseMoveGrabbed(ecs_iter_t *it) {
    Movable *m = ecs_field(it, Movable, 1);
    Position *p = ecs_field(it, Position, 2);
    Sprite* s = ecs_field(it, Sprite, 3);
    EventMouseMotion *move_event = ecs_field(it, EventMouseMotion, 4);


    for (int i = 0; i < it->count; i++) {
        if (m[i].is_grabbed)
        {
            p[i].x = move_event->x - m[i].offset_x; // Subtract the offset when moving the sprite
            p[i].y = move_event->y - m[i].offset_y;
        }
    }
}

void ConsumeEvents(ecs_iter_t* it)
{
    ecs_entity_t pair = ecs_field_id(it, 1);
    ecs_entity_t comp = ecs_pair_object(it->world, pair);
    for (int32_t i = 0; i < it->count; i++)
    {
        ecs_remove_id(it->world, it->entities[i], comp);
        ecs_remove_id(it->world, it->entities[i], pair);
    }
}

bool hasNamedParent(ase_layer_t* layer, const char* name) {
    // Base case: if there's no parent, return false
    if (layer->parent == NULL) {
        return false;
    }
    // Base case: if the parent's name matches, return true
    if (strcmp(layer->parent->name, name) == 0) {
        return true;
    }
    // Recursive case: check the next parent
    return hasNamedParent(layer->parent, name);
}


int main(int argc, char *argv[]) {


    ecs_world_t *world = ecs_init();
    input = ecs_set_name(world, 0, "input");

    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Movable);
    ECS_COMPONENT_DEFINE(world, EventMouseClick);
    ECS_COMPONENT_DEFINE(world, Agent);
    ECS_COMPONENT_DEFINE(world, Stats);
    ECS_COMPONENT_DEFINE(world, ConsumeEvent);
    ECS_COMPONENT_DEFINE(world, EventMouseMotion);

   ase_t* ase = cute_aseprite_load_from_file("table.ase", NULL);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    window = SDL_CreateWindow("Book Simulator Online", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ase->w, ase->h, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    for (int i = 0; i < ase->frame_count; i++) {
        ase_frame_t* frame = ase->frames + i;
        for (int j = 0; j < ase->layer_count; j++) {
            ase_cel_t* cel = frame->cels + j;
            if (cel) {
                if (cel->layer && cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) // TODO: Check if group parent is visible
                {
                    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, cel->w, cel->h);
                    if (texture == NULL) {
                        printf("Failed to create texture: %s\n", SDL_GetError());
                        continue;
                    }
                    
                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                    if (SDL_UpdateTexture(texture, NULL, cel->pixels, cel->w * sizeof(Uint32)) < 0) {
                        printf("Failed to update texture: %s\n", SDL_GetError());
                        SDL_DestroyTexture(texture);
                        continue;
                    }

                    ecs_entity_t e = ecs_new(world, 0);
                    ecs_set(world, e, Position, {cel->x, cel->y});
                    ecs_set(world, e, Sprite, {texture, cel->w, cel->h});

                    // printf("Add movable\n");
                    // if (cel->layer->parent) {
                    //     printf("%s\n", cel->layer->parent->name);
                    // }
                    if (hasNamedParent(cel->layer, "agents")) {
                        ecs_set(world, e, Movable, {false});
                        ecs_set(world, e, Agent, {"Agent Name"});
                        ecs_set(world, e, Stats, {0, 0});
                    }
                }
            }
        }
    }

    ECS_SYSTEM(world, MouseMovableSelection, EcsPostUpdate, Movable, Position, Sprite, EventMouseClick(input));
    ECS_SYSTEM(world, MouseMoveGrabbed, EcsPostUpdate, Movable, Position, Sprite, EventMouseMotion(input));
    // ECS_OBSERVER(world, MouseMove, EcsOnSet, EventMouseMotion(input));
    ECS_SYSTEM(world, Input, EcsPreUpdate, [inout] *());
    ECS_SYSTEM(world, Render, EcsOnUpdate, Position, Sprite);
    ECS_SYSTEM(world, ConsumeEvents, EcsPostFrame, (ConsumeEvent, *));

    while (ecs_progress(world, 0)) {
        SDL_RenderPresent(renderer);
    }

    return ecs_fini(world);
}