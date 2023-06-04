#include "flecs.h"
#include <SDL.h>
#include <SDL_ttf.h>
#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"
#include <string.h>

ecs_entity_t input;

ECS_STRUCT(Test, {
    float x;
});

ECS_STRUCT(Transform, {
    float x;
    float y;
    float r_x;
    float r_y;
});

ECS_STRUCT(Position, {
    int32_t x;
    int32_t y;
});

ECS_STRUCT(Size, {
    int32_t width;
    int32_t height;
});

typedef struct TestNormal {
    float value;
} TestNormal;
ECS_COMPONENT_DECLARE(TestNormal);


// ECS_STRUCT(Sprite, {
//     SDL_Texture* texture;
//     int32_t width;
//     int32_t height;
//     bool visible;
// });

typedef struct Sprite {
    SDL_Texture* texture;
    int32_t width;
    int32_t height;
    bool visible;
} Sprite;
ECS_COMPONENT_DECLARE(Sprite);

ECS_STRUCT(Movable, {
    bool is_grabbed;
    float offset_x; // Offset between mouse cursor and sprite's top-left corner
    float offset_y;
});

ECS_STRUCT(Layer, {
    ecs_id_t parent;
    bool visible;
});

// ECS_STRUCT(Agent, {
//     char* name;
// });

ECS_STRUCT(Stats, {
    int32_t read;
    int32_t lut;
});

ECS_STRUCT(EventMouseMotion, {
    int32_t x;
    int32_t y;
});

ECS_STRUCT(EventMouseClick, {
    int32_t x;
    int32_t y;
    uint8_t button; // 1: left, 2: middle, 3: right, etc.
    uint8_t state;
});

ECS_STRUCT(EventTextInput, {
    char text[33]; // 32 characters + null-terminating character
});

// ECS_STRUCT(EventKeyInput, {
//     SDL_Keycode keycode;
// });

typedef struct EventKeyInput  {
    SDL_Keycode keycode;
} EventKeyInput;
ECS_COMPONENT_DECLARE(EventKeyInput);


ECS_STRUCT(Cursor, {
    uint32_t lastToggle;
    bool visible;
});

ECS_STRUCT(ConsumeEvent, {
    bool mock;
});

ECS_STRUCT(Text, {
    char str[256];
    SDL_Surface *surface;
    SDL_Texture *texture;
    int32_t changed;
});


ECS_STRUCT(Textbox, {
    int32_t cursorPosition;
    int32_t active;
});

typedef struct Font {
    TTF_Font* font;
} Font;
ECS_COMPONENT_DECLARE(Font);

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
        }
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                ecs_quit(it->world);
            }
            ecs_set(it->world, input, EventKeyInput, {e.key.keysym.sym});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventKeyInput), {});
        } else if (e.type == SDL_MOUSEMOTION) {
            ecs_set(it->world, input, EventMouseMotion, {e.motion.x, e.motion.y});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventMouseMotion), {});
        } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            ecs_set(it->world, input, EventMouseClick, {e.button.x, e.button.y, e.button.button, e.button.state});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventMouseClick), {});
        }  else if (e.type == SDL_TEXTINPUT) {
            // printf("%s\n", e.text.text);
            ecs_set(it->world, input, EventTextInput, {""});
            EventTextInput* text_input = ecs_get_mut(it->world, input, EventTextInput);
            memset(text_input->text, 0, sizeof(text_input->text));
            strcat(text_input->text, e.text.text);
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventTextInput), {});
        }
    }
}

void TransformCascadeHierarchy(ecs_iter_t *it) {
    Transform *t_parent = ecs_field(it, Transform, 1);
    Transform *t = ecs_field(it, Transform, 2);
    if (t_parent)
    {
        for (int i = 0; i < it->count; i++) 
        {
            t[i].x = t[i].r_x + t_parent->x;
            t[i].y = t[i].r_y + t_parent->y;
        }
    } else 
    {
        for (int i = 0; i < it->count; i++) 
        {

            t[i].x = t[i].r_x;
            t[i].y = t[i].r_y;
            
        }
    }
}

void MouseMovableSelection(ecs_iter_t *it) {
    Movable *m = ecs_field(it, Movable, 1); //parent
    Transform *t_p = ecs_field(it, Transform, 2);
    Transform *p = ecs_field(it, Transform, 3);
    Sprite* s = ecs_field(it, Sprite, 4);
    EventMouseClick *click_event = ecs_field(it, EventMouseClick, 5);

    for (int i = 0; i < it->count; i++) {
        if (click_event->button == SDL_BUTTON_LEFT) {
            bool overlaps_sprite = click_event->x >= p[i].x && click_event->x <= p[i].x + s[i].width &&
                click_event->y >= p[i].y && click_event->y <= p[i].y + s[i].height;
            if (click_event->state == SDL_PRESSED) {
                if (overlaps_sprite)
                {
                    m[i].is_grabbed = true;
                    m[i].offset_x = click_event->x - t_p[i].x; // Remember the offset
                    m[i].offset_y = click_event->y - t_p[i].y;
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
    EventMouseMotion *move_event = ecs_field(it, EventMouseMotion, 3);

    for (int i = 0; i < it->count; i++) {
        if (m[i].is_grabbed)
        {
            // p[i].r_x += 1;
            p[i].r_x = move_event->x - (int)m[i].offset_x; // Subtract the offset when moving the sprite
            p[i].r_y = move_event->y - (int)m[i].offset_y;
            // printf("(%d, %d)\n", move_event->x - (int)m[i].offset_x, move_event->y - (int)m[i].offset_y);
            // printf("(%d, %d)\n", p[i].x, p[i].y);
        }
    }
}

void TextboxEntry(ecs_iter_t* it) {
    Textbox* tb = ecs_field(it, Textbox, 1);
    Text* txt = ecs_field(it, Text, 2);
    EventTextInput* input = ecs_field(it, EventTextInput, 3);
    for (int i = 0; i < it->count; i++) {
        // Check if the textbox is active
        if (tb[i].active) {
            // Append the input text to the textbox's text, but ensure that you do not exceed the textbox's max length
            int available_space = 255 - strlen(txt[i].str);
            int append_length = strlen(input->text);
            if (append_length < available_space) {
                strcat(txt[i].str, input->text);
            } else {
                strncat(txt[i].str, input->text, available_space);
                txt[i].str[255] = '\0';  // Ensure null termination
            }
            printf("%s\n", txt[i].str);
            txt->changed = true; // TODO: Refactor to OBSERVER event for performance
        }
    }
}

void HandleBackspace(ecs_iter_t* it) {
    Textbox* tb = ecs_field(it, Textbox, 1);
    Text* txt = ecs_field(it, Text, 2);
    EventKeyInput* key_input = ecs_field(it, EventKeyInput, 3);
    for (int i = 0; i < it->count; i++) {
        if (tb[i].active && key_input->keycode == SDLK_BACKSPACE && strlen(txt[i].str) > 0) {
            txt[i].str[strlen(txt[i].str) - 1] = '\0';  // Remove the last character
            printf("%s\n", txt[i].str);
            txt->changed = true; // TODO: Refactor to OBSERVER event for performance
        }
    }
}

void FontSet(ecs_iter_t *it) {
    Font *font = ecs_field(it, Font, 1);
    for (int i = 0; i < it->count; i++) {
        printf("TTF_OpenFont\n");
        font[i].font = TTF_OpenFont("../res/ATARISTOCRAT.ttf", 16);
        // font[i].font = TTF_OpenFont("../res/madness.ttf", 16);
        if (font[i].font == NULL) {
            printf("TTF_OpenFont: %s\n", TTF_GetError());
            // handle error
        }
    }
}

void FontRemove(ecs_iter_t *it) {
    Font *font = ecs_field(it, Font, 1);
    for (int i = 0; i < it->count; i++) {
        TTF_CloseFont(font[i].font);
    }
}


void TextboxClick(ecs_iter_t* it) {
    Textbox* tb = ecs_field(it, Textbox, 1);
    Position* p = ecs_field(it, Position, 2);
    Size* s = ecs_field(it, Size, 3);
    EventMouseClick* click = ecs_field(it, EventMouseClick, 4);

    for (int i = 0; i < it->count; i++) {
        // Calculate the boundaries of the textbox
        int tbLeft = p[i].x;
        int tbRight = p[i].x + s[i].width;
        int tbTop = p[i].y;
        int tbBottom = p[i].y + s[i].height;

        // Check if the click was within the boundaries of the textbox
        if (click[i].x >= tbLeft && click[i].x <= tbRight && click[i].y >= tbTop && click[i].y <= tbBottom) {
            // Click was within the textbox, so activate it
            tb[i].active = 1;
        } else {
            // Click was outside the textbox, so deactivate it
            tb[i].active = 0;
        }
    }
}

void TextboxCursorBlink(ecs_iter_t* it) {
    Textbox* tb = ecs_field(it, Textbox, 1);
    Cursor* cursor = ecs_field(it, Cursor, 2);
    
    // Get the current time in milliseconds
    Uint32 currentTime = SDL_GetTicks();

    for (int i = 0; i < it->count; i++) {
        if (tb[i].active && currentTime - cursor[i].lastToggle > 500) { // 500ms blink interval
            // Toggle the cursor visibility
            cursor[i].visible = !cursor[i].visible;
            
            // Update the last toggle time
            cursor[i].lastToggle = currentTime;
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

void RenderText(ecs_iter_t *it) {
    Transform *t = ecs_field(it, Transform, 1);
    Text *text = ecs_field(it, Text, 2);
    Font *font = ecs_field(it, Font, 3);

    // printf("RENDER TEXT\n");
    for (int i = 0; i < it->count; i++) {
        if (text[i].changed) {
            // Free old surface and texture if they exist
            if (text[i].surface) {
                SDL_FreeSurface(text[i].surface);
            }
            if (text[i].texture) {
                SDL_DestroyTexture(text[i].texture);
            }

            // Create new surface and texture
            text[i].surface = TTF_RenderText_Solid(font->font, text[i].str, (SDL_Color){255, 255, 255, 255});
            text[i].texture = SDL_CreateTextureFromSurface(renderer, text[i].surface);
            text[i].changed = 0;  // Mark text as unchanged
        }

        SDL_Rect dst;
        dst.x = (int)t[i].x;
        dst.y = (int)t[i].y;
        SDL_QueryTexture(text[i].texture, NULL, NULL, &dst.w, &dst.h);  // Get the width and height from the texture
        // printf("%d, %d\n", dst.w, dst.h);
        SDL_RenderCopy(renderer, text[i].texture, NULL, &dst);
    }
}



bool hasNamedAncestor(ase_layer_t* layer, const char* name) {
    // Base case: if there's no parent, return false
    if (layer->parent == NULL) {
        return false;
    }
    // Base case: if the parent's name matches, return true
    if (strcmp(layer->parent->name, name) == 0) {
        return true;
    }
    // Recursive case: check the next parent
    return hasNamedAncestor(layer->parent, name);
}

bool hasNamedParent(ase_layer_t* layer, const char* name) {
    if (layer->parent == NULL) {
        return false;
    }
    return strcmp(layer->parent->name, name) == 0;
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

void makeAgent(const char* name, ecs_world_t* world, ecs_entity_t e)
{
        // Set common Agent attributes
    // Define the Agent prefab
    // ECS_PREFAB(world, prefab, Transform, Movable);
    // ecs_set(world, prefab, Movable, {false});
    // TODO: Figure out how to make an instance of the prefab
    // ecs_entity_t ai = ecs_new_w_pair(world, EcsIsA, prefab);
    ecs_entity_t ai = ecs_set_name(world, 0, strcat(name, "_agent"));
    // TODO: If the layer has a parent
    ecs_add(world, ai, Transform);
    ecs_set(world, ai, Movable, {false});
    // Set specific Agent attributes
    // ecs_set(world, ai, Agent, {"agent"});
    ecs_set(world, ai, Stats, {0, 0});
    printf("Add child of\n");
    ecs_add_pair(world, e, EcsChildOf, ai);
}

void parseAsepriteFile(ase_t* ase, ecs_world_t* world, SDL_Renderer* renderer) {
    for (int f = 0; f < ase->frame_count; ++f) {
        ase_frame_t* frame = ase->frames + f;
        // First, we'll add all the group layers
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            if (layer->type == ASE_LAYER_TYPE_GROUP)
            {
                ecs_entity_t e = ecs_set_name(world, 0, layer->name);
                ecs_set(world, e, Transform, {0, 0, 0, 0});
                if (hasNamedParent(layer, "agents")) {
                    makeAgent(layer->name, world, e);
                }
            }
        }

        // Hierarchy the group layers
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            if (layer && layer->type == ASE_LAYER_TYPE_GROUP)
            {
                ecs_entity_t e = ecs_lookup(world, layer->name);
                if (ecs_is_valid(world, e) && ecs_is_alive(world, e) && layer->parent) {
                    ecs_entity_t parent_entity = ecs_lookup(world, layer->parent->name);
                    if (ecs_is_valid(world, parent_entity) && ecs_is_alive(world, parent_entity)) 
                    {
                        // TODO: Hierarchy groups causes sprites in subfolders to not render if not done last??
                        printf("%s child of %s\n", ecs_get_name(world, e),ecs_get_name(world, parent_entity));
                        // ecs_add_pair(world, e, EcsChildOf, parent_entity);
                    }
                }
            }
        }

        // Add and hierarchy sprites :)
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_cel_t* cel = frame->cels + i;
            if (cel && cel->layer)
            {
                ecs_entity_t e = ecs_set_name(world, 0, cel->layer->name);
                // printf("Cel has layer %s\n", cel->layer->name);
                if (cel->layer->parent)
                {
                    ecs_entity_t parent_entity = ecs_lookup(world, cel->layer->parent->name);
                    if (parent_entity && ecs_is_alive(world, parent_entity)) 
                    {
                        ecs_add_pair(world, e, EcsChildOf, parent_entity);
                    }
                }
                if (cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) // TODO: Check if group parent is visible
                {
                    ecs_set(world, e, Transform, {0, 0, cel->x, cel->y});
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

                    ecs_set(world, e, Sprite, {texture, cel->w, cel->h, true});

                    if (hasNamedParent(cel->layer, "agents")) {
                        makeAgent(cel->layer->name, world, e);
                    } else if (hasNamedAncestor(cel->layer, "agents"))
                    {
                        ecs_entity_t agent = ecs_lookup(world, strcat(cel->layer->parent, "_agent"));
                        if (ecs_is_valid(world, agent))
                        {
                            ecs_add_pair(world, e, EcsChildOf, agent);
                        }
                        // TODO: Figure out how to call systems with non-parent ancestors
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {

    ecs_world_t *world = ecs_init();
    ECS_IMPORT(world, FlecsMeta);
    input = ecs_set_name(world, 0, "input");

    ECS_META_COMPONENT(world, Transform);
    ECS_META_COMPONENT(world, Movable);
    ECS_META_COMPONENT(world, EventMouseClick);
    ECS_META_COMPONENT(world, Stats);
    ECS_META_COMPONENT(world, ConsumeEvent);
    ECS_META_COMPONENT(world, EventMouseMotion);
    ECS_META_COMPONENT(world, Textbox);
    ECS_META_COMPONENT(world, EventTextInput);
    ECS_META_COMPONENT(world, Position);
    ECS_META_COMPONENT(world, Size);
    ECS_META_COMPONENT(world, Cursor);
    ECS_META_COMPONENT(world, Text);
    ECS_META_COMPONENT(world, Test);

    ECS_COMPONENT_DEFINE(world, TestNormal);
    ECS_COMPONENT_DEFINE(world, Font);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, EventKeyInput);

    ecs_entity_t ent = ecs_new_entity(world, "ent");
    ecs_add(world, ent, Textbox);
    ecs_add(world, ent, Text);
    ecs_add(world, ent, TestNormal);

    // ecs_query_t *q = ecs_query(world, {
    //     .filter.terms = {
    //         { .id = ecs_id(TestNormal), .inout = EcsIn }
    //     }
    // });

    // Do the transform
    // ecs_iter_t it = ecs_query_iter(world, q);
    // while (ecs_query_next(&it)) {
    //     for (int i = 0; i < it.count; i++) {
    //         printf("%s\n", ecs_get_name(world, it.entities[i]));
    //     }
    // }

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

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }

    ECS_OBSERVER(world, FontSet, EcsOnAdd, Font);
    ECS_OBSERVER(world, FontRemove, EcsOnRemove, Font);

    ecs_entity_t resource = ecs_set_name(world, 0, "resource");
    ecs_add(world, resource, Font);

    ecs_entity_t test = ecs_new(world, 0);
    ecs_set(world, test, Transform, {0, 0, 64, 64});
    ecs_set(world, test, Text, {"Bulwark", NULL, NULL, 1}); // TODO: OBSERVER construction
    ecs_set(world, test, Textbox, {0, true});

    parseAsepriteFile(ase, world, renderer);

    ecs_entity_t tb = ecs_new(world, 0);
    ecs_add(world, tb, Textbox);


    ecs_query_t *q_meta = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(Transform), .inout = EcsIn },
            {
                .id = ecs_id(Transform), 
                .inout = EcsIn,
                // Get from the parent, in breadth-first order (cascade)
                .src.flags = EcsParent | EcsCascade,
                // Make parent term optional so we also match the root (sun)
                .oper = EcsOptional
            }
        }
    });

    // Do the transform
    ecs_iter_t it = ecs_query_iter(world, q_meta);
    while (ecs_query_next(&it)) {
        for (int i = 0; i < it.count; i++) {
            // printf("%s\n", ecs_get_name(world, it.entities[i]));
            char* str = ecs_entity_to_json(world, it.entities[i], &(ecs_entity_to_json_desc_t) {
                .serialize_path = true,
                .serialize_values = true
            });
            printf("ent = %s\n", str);
            ecs_os_free(str);
        }
    }

    ECS_SYSTEM(world, MouseMovableSelection, EcsPostUpdate, Movable(parent), Transform(parent), Transform, Sprite, EventMouseClick(input));
    ECS_SYSTEM(world, MouseMoveGrabbed, EcsPostUpdate, Movable, Transform, EventMouseMotion(input));
    ECS_SYSTEM(world, Input, EcsPreUpdate, [inout] *());
    ECS_SYSTEM(world, Render, EcsPostFrame, Transform, Sprite);
    ECS_SYSTEM(world, ConsumeEvents, EcsPostFrame, (ConsumeEvent, *));
    ECS_SYSTEM(world, TransformCascadeHierarchy, EcsPreFrame, ?Transform(parent|cascade), Transform);
    ECS_SYSTEM(world, TextboxEntry, EcsOnUpdate, Textbox, Text, EventTextInput(input));
    ECS_SYSTEM(world, HandleBackspace, EcsOnUpdate, Textbox, Text, EventKeyInput(input));
    ECS_SYSTEM(world, TextboxClick, EcsOnUpdate, Textbox, Position, Size, EventMouseClick(input));
    ECS_SYSTEM(world, TextboxCursorBlink, EcsOnUpdate, Textbox, Cursor);
    ECS_SYSTEM(world, RenderText, EcsPostFrame, Transform, Text, Font(resource));

    while (ecs_progress(world, 0)) {
        SDL_RenderPresent(renderer);
    }

    return ecs_fini(world);
}