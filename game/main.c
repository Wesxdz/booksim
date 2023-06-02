#include "flecs.h"
#include <SDL.h>
#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

ecs_entity_t input;


typedef struct Transform {
    float x, y;
    float r_x, r_y;
} Transform;
ECS_COMPONENT_DECLARE(Transform);

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

typedef struct Layer {
    ecs_id_t parent;
    bool visible;
} Layer;

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
    Transform *p = ecs_field(it, Transform, 1);
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

void PropagateTransforms(ecs_iter_t *it) {
    Transform *t = ecs_field(it, Transform, 1);
    Transform *t_parent = ecs_field(it, Transform, 2);
    for (int i = 0; i < it->count; i++) {
        t[i].x = t[i].r_x + t_parent[i].r_x;
        t[i].y = t[i].r_y + t_parent[i].r_y;
    }
}

void MouseMovableSelection(ecs_iter_t *it) {
    Movable *m = ecs_field(it, Movable, 1);
    Transform *p = ecs_field(it, Transform, 2);
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
    Transform *p = ecs_field(it, Transform, 2);
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

// TODO: Refactor to util lib (ie keep observer/systems/util spatially separate)
bool isLayerVisible(Layer* layer) {
    // Base case: if there's no parent, return the layer's visibility
    if (layer->parent == NULL) {
        return layer->visible;
    }
    // Recursive case: check the next parent (if the current layer or any of its parents are invisible, the function returns false)
    return layer->visible && isLayerVisible(layer->parent);
}

void parseAsepriteFile(ase_t* ase, ecs_world_t* world, SDL_Renderer* renderer) {
    for (int f = 0; f < ase->frame_count; ++f) {
        ase_frame_t* frame = ase->frames + f;
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_cel_t* cel = frame->cels + i;
            if (cel && cel->layer)
            {
                printf("Cel has layer %s\n", cel->layer->name);
                if (cel->layer->parent)
                {
                    printf("Cel has parent layer %s\n", cel->layer->parent->name);
                }
                // TODO: These are all of the NORMAL layers!
            }
            ase_layer_t* layer = ase->layers + i;
            if (layer->type == ASE_LAYER_TYPE_GROUP)
            {
                printf("Layer %s is group\n", layer->name);
            }
            
            // ecs_entity_t e = ecs_set_name(world, 0, layer->name);
            // printf("%s entity has %d layer type\n", ecs_get_name(world, e), (int)layer->type);
            // if (layer->type == ASE_LAYER_TYPE_GROUP)
            // {
            //     printf("GROUP: %s\n", layer->name);
            //     ecs_set(world, e, Transform, {0, 0, 0, 0}); // TODO: Eventually add relative positions/WYSIWYG
            // }
            // // if (layer->parent) {
            // //     ecs_entity_t parent_entity = ecs_lookup(world, layer->parent->name);
            // //     if (parent_entity && ecs_is_alive(world, parent_entity)) 
            // //     {
            // //         printf("%s Child Of %s\n", ecs_get_name(world, e), ecs_get_name(world, parent_entity));
            // //         ecs_add_pair(world, e, EcsChildOf, parent_entity);
            // //     }
            // // }

            // ase_cel_t* cel = frame->cels + i; // Frame one
            // if (cel) {
            //     if (!cel->layer)
            //     {
            //         printf("%s has no cel layer\n", layer->name);
            //     }
            //     // printf("Layer: %d Cel Layer %d\n", layer, cel->layer);
            //     // printf("Layer Parent: %s Cel Layer Parent %s\n", layer->parent->name, cel->layer->parent->name);
            //     if (layer && (layer->flags & ASE_LAYER_FLAGS_VISIBLE)) // TODO: Check if group parent is visible
            //     {
            //         if (layer->type == ASE_LAYER_TYPE_NORMAL)
            //         {
            //             printf("NORMAL: %s\n", layer->name);
            //             ecs_set(world, e, Transform, {cel->x, cel->y, 0, 0});
            //             SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, cel->w, cel->h);
            //             if (texture == NULL) {
            //                 printf("Failed to create texture: %s\n", SDL_GetError());
            //                 continue;
            //             }
                        
            //             SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

            //             if (SDL_UpdateTexture(texture, NULL, cel->pixels, cel->w * sizeof(Uint32)) < 0) {
            //                 printf("Failed to update texture: %s\n", SDL_GetError());
            //                 SDL_DestroyTexture(texture);
            //                 continue;
            //             }

            //             ecs_set(world, e, Sprite, {texture, cel->w, cel->h});

            //             if (hasNamedParent(layer, "agents")) {
            //                 ecs_set(world, e, Movable, {false});
            //                 ecs_set(world, e, Agent, {"Agent Name"});
            //                 ecs_set(world, e, Stats, {0, 0});
            //             }
            //         }
            //     }
            // }
        }
    }
}

int main(int argc, char *argv[]) {


    ecs_world_t *world = ecs_init();
    input = ecs_set_name(world, 0, "input");

    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Movable);
    ECS_COMPONENT_DEFINE(world, EventMouseClick);
    ECS_COMPONENT_DEFINE(world, Agent);
    ECS_COMPONENT_DEFINE(world, Stats);
    ECS_COMPONENT_DEFINE(world, ConsumeEvent);
    ECS_COMPONENT_DEFINE(world, EventMouseMotion);

   ase_t* ase = cute_aseprite_load_from_file("shapes.ase", NULL);

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

    parseAsepriteFile(ase, world, renderer);

    ECS_SYSTEM(world, MouseMovableSelection, EcsPostUpdate, Movable, Transform, Sprite, EventMouseClick(input));
    ECS_SYSTEM(world, MouseMoveGrabbed, EcsPostUpdate, Movable, Transform, Sprite, EventMouseMotion(input));
    // ECS_OBSERVER(world, MouseMove, EcsOnSet, EventMouseMotion(input));
    ECS_SYSTEM(world, Input, EcsPreUpdate, [inout] *());
    ECS_SYSTEM(world, Render, EcsPostFrame, Transform, Sprite);
    ECS_SYSTEM(world, ConsumeEvents, EcsPostFrame, (ConsumeEvent, *));

    ecs_entity_t move_sys = ecs_system(world, {
    .entity = ecs_entity(world, { .name = "MoveSystem", .add = { ecs_dependson(EcsPreStore) } }),
    .query.filter.terms = {
        {ecs_id(Transform)},
        { ecs_id(Transform), .src = {
            .flags = EcsCascade,
            .trav = EcsChildOf
        }},
        },  
    .callback = PropagateTransforms
    });
    

    while (ecs_progress(world, 0)) {
        SDL_RenderPresent(renderer);
    }

    return ecs_fini(world);
}