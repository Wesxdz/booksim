#include "flecs.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"
#include <string.h>
#include "log.h"

ecs_entity_t input;

ECS_STRUCT(Test, {
    float x;
});

ECS_STRUCT(SceneGraph,
{
    ecs_entity_t symbol;
    ecs_entity_t prev; // TODO: These should probably be refactored to relationship pairs
    ecs_entity_t next;
    bool user_mark_visible;
    bool user_mark_expanded;
    bool is_visible;
    bool is_expanded;
    int32_t children_count;
    int32_t index;
    int32_t depth;
});

ECS_TAG_DECLARE(Selected);

// Relationship
ECS_TAG_DECLARE(Symbol);
ECS_TAG_DECLARE(Background);

ECS_ENUM(ScopeIndicator, {
    EXPANDED,
    REDUCED
});

ECS_STRUCT(ArrowStatus, {
    ScopeIndicator scope;
});

// TODO: Refactor, replace component with Position pairs
ECS_STRUCT(Transform, {
    float x;
    float y;
    float r_x;
    float r_y;
});

ECS_STRUCT(Line, {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
});

ECS_STRUCT(Color, {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
});

ECS_ENUM(BoxMode, {
    FILL, OUTLINE
});

ECS_STRUCT(Box, {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    BoxMode mode;
    Color color;
});

// TODO: Replace transform with (Position, World)/(Position, Local) pairs!

ECS_TAG_DECLARE(World);
ECS_TAG_DECLARE(Local);

ECS_STRUCT(Position, {
    int32_t x;
    int32_t y;
});

ECS_STRUCT(Size, {
    int32_t width;
    int32_t height;
});

// TODO: Evaluate for SceneGraph, whether to remove/recreate components or toggle render
ECS_STRUCT(Renderable, {
    int32_t z_index;
    bool visible;
});

typedef struct TestNormal {
    float value;
} TestNormal;
ECS_COMPONENT_DECLARE(TestNormal);

typedef struct SceneGraphLayout {
    int index;
} SceneGraphLayout;
ECS_COMPONENT_DECLARE(SceneGraphLayout);

typedef struct Sprite {
    SDL_Texture* texture;
    int32_t width;
    int32_t height;
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

ECS_STRUCT(EventMouseWheel, {
    int32_t x;
    int32_t y;
    uint32_t direction;
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
    char str[256]; // https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
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

typedef struct SDL_Interface
{
    SDL_Window* window;
    SDL_Renderer* renderer;
} SDL_Interface;
ECS_COMPONENT_DECLARE(SDL_Interface);

Sprite loadSprite(SDL_Renderer* renderer, char* file_path) {
    Sprite sprite;

    // Load image into a surface
    SDL_Surface* temp_surface = IMG_Load(file_path);
    if (!temp_surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return sprite;
    }

    // Convert surface to texture
    sprite.texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    if (!sprite.texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp_surface);
        return sprite;
    }

    // Fill in sprite struct
    sprite.width = temp_surface->w;
    sprite.height = temp_surface->h;

    // Free the temporary surface
    SDL_FreeSurface(temp_surface);

    return sprite;
}

// Rather than 'Selected' being a tag, it can be a prefab?
void RenderSelectedBox(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 1);
    Text* t = ecs_field(it, Text, 2);
    SceneGraph* sc = ecs_field(it, SceneGraph, 3);
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 5);

for (int i = 0; i < it->count; i++) 
{
    // create SDL_Rect for the box around the text
    SDL_Rect rect;
    rect.x = p[i].x - 2; // subtract padding from position
    rect.y = p[i].y + 2; // subtract padding from position
    if (t[i].surface)
    {
        rect.w = t[i].surface->w + 4; // add twice the padding to the width
        rect.h = t[i].surface->h - 1; // add twice the padding to the height
    } else
    {
        rect.w = 0;
        rect.h = 0;
    }

    // draw the box
    SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255); // set color to white for the outline
    SDL_RenderDrawRect(sdl->renderer, &rect);

    if (sc[i].children_count)
    {
        // create SDL_Rect for the playful bright purple square
        SDL_Rect purple_rect;
        purple_rect.x = rect.x - (rect.h+2); // position it right after the previous box
        purple_rect.y = rect.y; // same y position as previous box
        purple_rect.w = rect.h+1; // width equals to height of previous box to make a square
        purple_rect.h = rect.h; // same height as previous box

        // draw the purple box
        SDL_SetRenderDrawColor(sdl->renderer, 151, 41, 255, 255); // set color to bright purple
        SDL_RenderDrawRect(sdl->renderer, &purple_rect);
    }
}


}

// void IndicateSelectedSprite(ecs_iter_t *it) {
//     Transform *p = ecs_field(it, Transform, 1);
//     Sprite* t = ecs_field(it, Text, 2);
//     SDL_Interface* sdl = ecs_field(it, SDL_Interface, 5);

//     for (int i = 0; i < it->count; i++) 
//     {
//         // create SDL_Rect for the box around the text
//         SDL_Rect rect;
//         rect.x = p[i].x - 2; // subtract padding from position
//         rect.y = p[i].y + 2; // subtract padding from position
//         if (t[i].surface)
//         {
//             rect.w = t[i].surface->w + 4; // add twice the padding to the width
//             rect.h = t[i].surface->h - 1; // add twice the padding to the height
//         } else
//         {
//             rect.w = 0;
//             rect.h = 0;
//         }

//         // draw the box
//         SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255); // set color to white for the outline
//         SDL_RenderDrawRect(sdl->renderer, &rect);
//     }
// }

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
        } else if (e.type == SDL_MOUSEWHEEL)
        {
            ecs_set(it->world, input, EventMouseWheel, {e.wheel.x, e.wheel.y, e.wheel.direction});
            ecs_set_pair(it->world, input, ConsumeEvent, ecs_id(EventMouseWheel), {}); // TODO: Refactor ConsumeEvent to tag...
        }
    }
}

void TransformCascadeHierarchy(ecs_iter_t *it) {
    Position* world_pos = ecs_field(it, Position, 1);
    const Position* local_pos = ecs_field(it, Position, 2);
    const Position* parent_world_pos = ecs_field(it, Position, 3);
    if (parent_world_pos)
    {
        for (int i = 0; i < it->count; i++) 
        {
            if (local_pos)
            {
                world_pos[i].x = parent_world_pos->x + local_pos[i].x;
                world_pos[i].y = parent_world_pos->y + local_pos[i].y;
            }
            // If there is no local position, the world_pos of the parent is not propagated
        }
    } else 
    {
        for (int i = 0; i < it->count; i++) 
        {
            if (local_pos)
            {
                world_pos[i].x = local_pos[i].x;
                world_pos[i].y = local_pos[i].y;
            }
        }
    }
}

void SceneGraphSettingsCascadeHierarchy(ecs_iter_t *it) {
    // printf("SceneGraphSettingsCascadeHierarchy\n");
    SceneGraph* sc_parent = ecs_field(it, SceneGraph, 1);
    SceneGraph *sc = ecs_field(it, SceneGraph, 2);
    // Renderable* settings = ecs_field(it, Renderable, 3);
    for (int i = 0; i < it->count; i++) 
    {
        if (sc_parent)
        {
            log_trace("%s's parent is %d expanded\n", ecs_get_name(it->world, it->entities[i]), sc_parent->is_expanded);
            if (!sc[i].user_mark_expanded)
            {
                if (sc_parent->user_mark_expanded == false)
                {
                    sc[i].is_expanded = false;
                } else
                {
                    sc[i].is_expanded = sc_parent->is_expanded;
                }
            } else
            {
                if (sc_parent->user_mark_expanded == false)
                {
                    sc[i].is_expanded = false;
                } else
                {
                    sc[i].is_expanded = sc_parent->is_expanded;
                }
            }
        } else 
        { 
            sc[i].is_expanded = true;
        }
        // printf("%s is %d expanded\n", ecs_get_name(it->world, it->entities[i]), sc[i].is_expanded);
        // settings[i].visible = sc[i].is_expanded;
    }
}

void MouseMovableSelection(ecs_iter_t *it) {
    printf("Mouse movable selection\n");
    Movable *m = ecs_field(it, Movable, 1); //parent
    Position *t_p = ecs_field(it, Position, 2);
    Position *p = ecs_field(it, Position, 3);
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
    Position *p = ecs_field(it, Position, 2);
    EventMouseMotion *move_event = ecs_field(it, EventMouseMotion, 3);

    for (int i = 0; i < it->count; i++) {
        if (m[i].is_grabbed)
        {
            // p[i].r_x += 1;
            p[i].x = move_event->x - (int)m[i].offset_x; // Subtract the offset when moving the sprite
            p[i].y = move_event->y - (int)m[i].offset_y;
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
            // printf("%s\n", txt[i].str);
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
            // printf("%s\n", txt[i].str);
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

void UpdateArrowDirection(ecs_iter_t* it)
{
    SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    Position* pos = ecs_field(it, Position, 2);
    Sprite* sprite = ecs_field(it, Sprite, 3);
    ArrowStatus* status = ecs_field(it, ArrowStatus, 4);
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 5);

    for (int i = 0; i < it->count; i++)
    {
        if (sc->children_count)
        {
            if (sc->user_mark_expanded)
            {
                if (status[i].scope == REDUCED)
                {
                    status[i].scope = EXPANDED;
                    // Switch pos/sprite to down arrow
                    Sprite update = loadSprite(sdl->renderer, "../res/arrow_down.png");
                    ecs_set(it->world, it->entities[i], Sprite, {update.texture, update.width, update.height});
                    ecs_set_pair(it->world, it->entities[i], Position, Local, {-11 , 6});
                }
            } else
            {
                if (status[i].scope == EXPANDED)
                {
                    status[i].scope = REDUCED;
                    // Switch pos/sprite to right arrow
                    Sprite update = loadSprite(sdl->renderer, "../res/arrow_right.png");
                    ecs_set(it->world, it->entities[i], Sprite, {update.texture, update.width, update.height});
                    ecs_set_pair(it->world, it->entities[i], Position, Local, {-9 , 4});

                }
            }
        }
    }
}

void UpdateSceneGraphLines(ecs_iter_t* it)
{
    SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    // Line* l = ecs_field(it, Line, 2);

    for (int i = 0; i < it->count; i++)
    {
        if (sc[i].children_count)
        {
            ecs_entity_t next = sc[i].next;
            int expanded_children_count = 0;
            int x = 0;
            while (ecs_is_valid(it->world, next) && ecs_is_alive(it->world, next) && x < sc[i].children_count)
            {
                SceneGraph* scNext = ecs_get_mut(it->world, next, SceneGraph);
                next = scNext->next;
                if (scNext->is_expanded)
                {
                    expanded_children_count++;
                }
                x++;
            }
            if (ecs_is_valid(it->world, it->entities[i]))
            {
                if (expanded_children_count == 0)
                {
                    ecs_remove(it->world, it->entities[i], Line);
                } else
                {
                    ecs_set(it->world, it->entities[i], Line, {-11, 16, -11, 16 + expanded_children_count*12-8});
                }
            }
        }
    }
}

void ToggleSceneGraphHierarchy(ecs_iter_t* it)
{
    SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    EventKeyInput* event = ecs_field(it, EventKeyInput, 3);
    for (int32_t i = 0; i < it->count; i++)
    {
        if (event->keycode == SDLK_a || event->keycode == SDLK_LEFT)
        {
            // Retract
            sc->user_mark_expanded = false;
            
        }
        else if (event->keycode == SDLK_d || event->keycode == SDLK_RIGHT)
        {
            // Expand
            sc->user_mark_expanded = true;
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

void GenTextTexture(ecs_iter_t* it)
{
    Text* text = ecs_field(it, Text, 1);
    Font* font = ecs_field(it, Font, 3);
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 4);
    for (int i = 0; i < it->count; it++)
    {
        if (text[i].surface) {
            SDL_FreeSurface(text[i].surface);
        }
        if (text[i].texture) {
            SDL_DestroyTexture(text[i].texture);
        }

        // Create new surface and texture
        text[i].surface = TTF_RenderText_Solid(font->font, text[i].str, (SDL_Color){255, 255, 255, 255});
        text[i].texture = SDL_CreateTextureFromSurface(sdl->renderer, text[i].surface);
        text[i].changed = 0;  // Mark text as unchanged
    }
}

void UpdateTextSurface(ecs_iter_t *it) {
    Position* p = ecs_field(it, Position, 1);
    Text *text = ecs_field(it, Text, 2);
    Font *font = ecs_field(it, Font, 3);
    Renderable *settings = ecs_field(it, Renderable, 4);
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 5);


    // printf("RENDER TEXT\n");
    for (int i = 0; i < it->count; i++) {
        if (!settings || settings[i].visible)
        {
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
                text[i].texture = SDL_CreateTextureFromSurface(sdl->renderer, text[i].surface);
                text[i].changed = 0;  // Mark text as unchanged
            }
        }
    }
}

void SetupSelectedNodeIndicator(ecs_iter_t* it)
{
    Box* b = ecs_field(it, Box, 2);
    SceneGraph* sc = ecs_field(it, SceneGraph, 3);
    Text* text = ecs_field(it, Text, 4);

    for (int i = 0; i < it->count; i++)
    {
        int txtWidth = 0;
        int txtHeight = 0;
        SDL_QueryTexture(text->texture, NULL, NULL, &txtWidth, &txtHeight);
        b[i].w = txtWidth+4;
        b[i].h = txtHeight-1;
    }
}

void KeyNavSceneGraph(ecs_iter_t* it)
{
    SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    EventKeyInput* event = ecs_field(it, EventKeyInput, 3);

    if (event->keycode == SDLK_DOWN || event->keycode == SDLK_s)
    {
        // SceneGraph* nextNode = ecs_get(it.world, sc->next, SceneGraph);
        for (int i = 0; i < it->count; i ++) {
            ecs_entity_t next = sc[i].next;
            while (ecs_is_valid(it->world, next))
            {
                SceneGraph* scNext = ecs_get(it->world, next, SceneGraph);
                if (scNext->is_expanded)
                {
                    ecs_add_pair(it->world, it->entities[i], EcsChildOf, next);
                    break;
                }
                next = scNext->next;
            }
        }
    } else if (event->keycode == SDLK_UP || event->keycode == SDLK_w)
    {
        for (int i = 0; i < it->count; i ++) {
            ecs_entity_t prev = sc[i].prev;
            while (ecs_is_valid(it->world, prev))
            {
                SceneGraph* scPrev = ecs_get(it->world, prev, SceneGraph);
                if (scPrev->is_expanded)
                {
                    ecs_add_pair(it->world, it->entities[i], EcsChildOf, prev);
                    break;
                }
                prev = scPrev->prev;
            }
        }
    }
}

void MouseWheelNavSceneGraph(ecs_iter_t* it)
{
    SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    EventMouseWheel* event = ecs_field(it, EventMouseClick, 3);

    if (event->y < 0)
    {
        for (int i = 0; i < it->count; i++)
        {
            ecs_entity_t next = sc[i].next;
            while (ecs_is_valid(it->world, next))
            {
                SceneGraph* scNext = ecs_get(it->world, next, SceneGraph);
                if (scNext->is_expanded)
                {
                    ecs_add_pair(it->world, it->entities[i], EcsChildOf, next);
                    break;
                }
                next = scNext->next;
            }
        }
    }
    else if (event->y > 0) 
    {
        for (int i = 0; i < it->count; i++)
        {
            ecs_entity_t prev = sc[i].prev;
            while (ecs_is_valid(it->world, prev))
            {
                SceneGraph* scPrev = ecs_get(it->world, prev, SceneGraph);
                if (scPrev->is_expanded)
                {
                    ecs_add_pair(it->world, it->entities[i], EcsChildOf, prev);
                    break;
                }
                prev = scPrev->prev;
            }
        }
    }
}

void UpdateSymbolBackground(ecs_iter_t* it)
{
    printf("UpdateSymbolBackground\n");
    Box* box = ecs_field(it, Box, 1);
    Position* p = ecs_field(it, Position, 2);
    SceneGraph* sc = ecs_field(it, SceneGraph, 4);
    Text* text = ecs_field(it, Text, 5);

    for (int i = 0; i < it->count; i++) {
        int start_x = sc->depth*8.0f;
        int txtWidth = 0;
        int txtHeight = 0;
        SDL_QueryTexture(text->texture, NULL, NULL, &txtWidth, &txtHeight);
        int ebox_start_x = start_x;
        if (sc->children_count)
        {
            ebox_start_x += 11;
        }
        box[i].w = ebox_start_x+txtWidth + 2;
        p[i].x = -ebox_start_x;
    }
}

void RenderCommander(ecs_iter_t* it)
{
    // printf("Render COMMANDER!\n");
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 7);
    Position* p = ecs_field(it, Position, 1);
    Renderable* renderable = ecs_field(it, Renderable, 2);

    // Optional
    Sprite* s = ecs_field(it, Sprite, 3);
    Text* text = ecs_field(it, Text, 4);
    Line* l = ecs_field(it, Line, 5);
    Box* box = ecs_field(it, Box, 6);

    if (s)
    {
        for (int i = 0; i < it->count; i++) {
            SDL_Rect dst;
            dst.x = (int)p[i].x;
            dst.y = (int)p[i].y;
            dst.w = s[i].width;
            dst.h = s[i].height;
            SDL_RenderCopy(sdl->renderer, s[i].texture, NULL, &dst);
        }
    }
    if (text)
    {        
        for (int i = 0; i < it->count; i++) {
            SDL_Rect dst;
            dst.x = (int)p[i].x;
            dst.y = (int)p[i].y;
            SDL_QueryTexture(text[i].texture, NULL, NULL, &dst.w, &dst.h);  // Get the width and height from the texture
            // printf("%d, %d\n", dst.w, dst.h);
            SDL_RenderCopy(sdl->renderer, text[i].texture, NULL, &dst);
        }
    }
    if (l)
    {
        for (int i = 0; i < it->count; i++)
        {
            SDL_SetRenderDrawColor(sdl->renderer, 128, 128, 128, 255);
            // SDL_SetRenderDrawColor(sdl->renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(sdl->renderer, p[i].x + l[i].x1, p[i].y + l[i].y1, p[i].x +  l[i].x2, p[i].y + l[i].y2);
        }
        SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
    }
    if (box)
    {
        for (int i = 0; i < it->count; i++) {
            SDL_Rect rect;
            rect.x = (int)p[i].x + box[i].x;
            rect.y = (int)p[i].y + box[i].y;
            rect.w = box[i].w;
            rect.h = box[i].h;
            // enable blending
            SDL_SetRenderDrawBlendMode(sdl->renderer, SDL_BLENDMODE_BLEND);
            // set color to white with alpha for the filled box
            SDL_SetRenderDrawColor(sdl->renderer, box[i].color.r, box[i].color.g, box[i].color.b, box[i].color.a);
            if (box[i].mode == FILL)
            {
                SDL_RenderFillRect(sdl->renderer, &rect);
            } else
            {
                SDL_RenderDrawRect(sdl->renderer, &rect);
            }
        }
    }
}


void RenderPresent(ecs_iter_t *it)
{
    SDL_Interface* sdl = ecs_field(it, SDL_Interface, 1);

    for (int i = 0; i < it->count; i++) {
        SDL_RenderPresent(sdl[i].renderer);
        SDL_SetRenderDrawColor(sdl[i].renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl[i].renderer);
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


void parseAsepriteFile(ase_t* ase, ecs_world_t* world, ecs_entity_t sceneGraph, SDL_Renderer* renderer) {

    ecs_entity_t node[CUTE_ASEPRITE_MAX_LAYERS];
    memset(node, 0, sizeof(node));
    ecs_entity_t parents[CUTE_ASEPRITE_MAX_LAYERS];
    memset(parents, 0, sizeof(parents));
    bool has_parent[CUTE_ASEPRITE_MAX_LAYERS];
    memset(has_parent, false, sizeof(has_parent));

    for (int f = 0; f < ase->frame_count; ++f) {
        ase_frame_t* frame = ase->frames + f;
        // First, we'll add all the group layers
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            if (layer->type == ASE_LAYER_TYPE_GROUP)
            {
                if (layer->parent)
                {
                    log_trace("LAYER %s PARENT %s\n", layer->name, layer->parent->name);
                } else
                {
                    log_trace("LAYER %s\n", layer->name);
                }
            }
        }

        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            if (layer->type == ASE_LAYER_TYPE_GROUP)
            {
                ecs_entity_t e = ecs_set_name(world, 0, layer->name);
                ecs_set_pair(world, e, Position, World, {0, 0});
                
                if (layer->parent)
                {
                    log_trace("CREATE GROUP: %s (parent layer):%s\n", layer->name, layer->parent->name);
                } else {
                    log_trace("CREATE GROUP: %s NO PARENT LAYER\n", layer->name);
                }
                // if (hasNamedParent(layer, "agents")) {
                //     // TODO: This messes up layers parenting too unfortunately....
                //     makeAgent(layer->name, world, e);
                // }
            }
        }

        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            if (layer && layer->type == ASE_LAYER_TYPE_GROUP)
            {
                if (layer->parent)
                {
                    log_trace("Layer %s has parent %s\n", layer->name, layer->parent->name);
                }
                ecs_entity_t e = ecs_lookup(world, layer->name);
                node[i] = e;
                if (ecs_is_valid(world, e) && ecs_is_alive(world, e) && layer->parent) {
                    ecs_entity_t parent_entity = ecs_lookup(world, layer->parent->name);
                    if (ecs_is_valid(world, parent_entity) && ecs_is_alive(world, parent_entity)) 
                    {
                        log_trace("GROUP HIERARCHY: %s child of %s\n", ecs_get_name(world, e),ecs_get_name(world, parent_entity));
                        parents[i] = parent_entity;
                        has_parent[i] = true;
                    }
                }
            }
        }

        // Add and hierarchy sprites :)
        int c = 0;
        for (int i = 0; i < ase->layer_count; ++i) 
        {
            ase_layer_t* layer = ase->layers + i;
            ase_cel_t* cel = frame->cels + c;
            bool new_cel = false;
            if (layer->type != ASE_LAYER_TYPE_GROUP)
            {
                c++;
                new_cel = true;
            }
            log_trace("CEL %s vs LAYER %s\n", cel->layer->name, layer->name);
            char entityName[256];
            // snprintf(entityName, sizeof(entityName), "%s_%d", cel->layer->name, i);
            snprintf(entityName, sizeof(entityName), "%s", cel->layer->name);
            if (new_cel)
            {
                ecs_entity_t e = ecs_set_name(world, 0, entityName);
                if (cel->layer->parent)
                {
                    ecs_entity_t parent_entity = ecs_lookup(world, cel->layer->parent->name);
                    node[i] = e;
                    if (parent_entity) 
                    {
                        log_trace("CEL LAYER: %s child of %s\n", ecs_get_name(world, e), ecs_get_name(world, parent_entity));
                        parents[i] = parent_entity;
                        has_parent[i] = true;
                    }
                }
                if (cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) // TODO: Check if group parent is visible
                {
                    ecs_set_pair(world, e, Position, World, {0, 0});
                    ecs_set_pair(world, e, Position, Local, {cel->x, cel->y});
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

                    ecs_set(world, e, Sprite, {texture, cel->w, cel->h});
                    ecs_set(world, e, Renderable, {i, true});

                    // if (hasNamedParent(cel->layer, "agents")) {
                    //     makeAgent(cel->layer->name, world, e);
                    // } else if (hasNamedAncestor(cel->layer, "agents"))
                    // {
                    //     ecs_entity_t agent = ecs_lookup(world, strcat(cel->layer->parent, "_agent"));
                    //     if (ecs_is_valid(world, agent))
                    //     {
                    //         ecs_add_pair(world, e, EcsChildOf, agent);
                    //     }
                    //     // TODO: Figure out how to call systems with non-parent ancestors
                    // }
                }
            }   
        }
    }
    for (int i = CUTE_ASEPRITE_MAX_LAYERS-1; i >=0 ; i--)
    {
        if (has_parent[i])
        {
            log_trace("%s %s\n", ecs_get_name(world, node[i]), ecs_get_name(world, parents[i]));
            ecs_add_pair(world, node[i], EcsChildOf, parents[i]);

        } else if (ecs_is_valid(world, node[i]))
        {
            ecs_add_pair(world, node[i], EcsChildOf, sceneGraph); // root node
            log_trace("Root node is %s\n", ecs_get_name(world, node[i]));
        }
    }
}

typedef struct {
    ecs_world_t* world;
    ecs_entity_t root;
    int depth;
    int count;
    ecs_entity_t prev;
} lambda_parameters;


typedef struct {
    int count;
    ecs_entity_t firstCreatedNode;
    ecs_entity_t lastCreatedNode;
} lambda_output;

typedef struct {
    ecs_entity_t* nodes;
    ecs_entity_t* parents;
} lambda_data;

ecs_entity_t test_node(lambda_parameters* params)
{
    ecs_world_t* world = params->world;
    ecs_entity_t root = params->root;
    int depth = params->depth;
    int count = params->count;
    ecs_entity_t prev = params->prev;

    char* root_name = ecs_get_name(world, root);
    printf("Root name gather %s\n", root_name);
}

ecs_entity_t create_node(lambda_parameters* params)
{
    ecs_world_t* world = params->world;
    ecs_entity_t root = params->root;
    int depth = params->depth;
    int count = params->count;
    ecs_entity_t prev = params->prev;

    char* root_name = ecs_get_name(world, root);
    char entityName[256];
    snprintf(entityName, sizeof(entityName), "%s_%s", root_name, "node");

    char* s = ecs_get_name(world, root);
    ecs_entity_t ebox = ecs_set_name(world, 0, entityName);
    ecs_add_pair(world, ebox, Symbol, root);
    ecs_set(world, ebox, Text, {*s, NULL, NULL, 1});
    char* path = ecs_get_fullpath(world, root);
    int start_x = depth*8.0f;
    int start_y = (count-1)*12.0f-2;
    int children_count = get_children_count(world, root);
    bool has_children = children_count > 0;
    // TODO: Better padding config
    ecs_set(world, ebox, SceneGraph, {root, prev, NULL, true, true, true, true, children_count, count, depth});

    log_trace("%s\n", path);
    Text* text = ecs_get_mut(world, ebox, Text);
    memset(text->str, 0, sizeof(text->str));
    strcat(text->str, s);
    ecs_os_free(path);

    ecs_set(world, ebox, Renderable, {10000, true});

    text = ecs_get_mut(world, ebox, Text);
    int txtWidth = 0;
    int txtHeight = 0;
    SDL_QueryTexture(text->texture, NULL, NULL, &txtWidth, &txtHeight);

    int ebox_start_x = start_x;
    if (has_children)
    {
        ebox_start_x += 11;
        ecs_set_pair(world, ebox, Position, World, {ebox_start_x, start_y});

        ecs_entity_t arrow = ecs_new(world, 0);
        // ecs_set_pair(world, arrow, Position, World, {start_x + txtWidth+5.0f, start_y+6});
        ecs_set_pair(world, arrow, Position, World, {0, 0});
        ecs_set_pair(world, arrow, Position, Local, {-11 , 6});
        ecs_add_pair(world, arrow, EcsChildOf, ebox);
        ecs_entity_t sdl = ecs_lookup(world, "sdl");
        SDL_Interface* sdlinterface = ecs_get_mut(world, sdl, SDL_Interface);
        Sprite sprite = loadSprite(sdlinterface->renderer, "../res/arrow_down.png");
        ecs_set(world, arrow, Sprite, {sprite.texture, sprite.width, sprite.height});
        ecs_set(world, arrow, Renderable, {1500, true});
        ecs_set(world, arrow, ArrowStatus, {EXPANDED});
        ecs_set(world, ebox, Line, {-11, 16, -11, 16 + children_count*12-8});
    } else
    {
        ecs_set_pair(world, ebox, Position, World, {start_x, start_y});
        ecs_set(world, ebox, Line, {-8, 8, -4, 8});
    }

    ecs_entity_t background = ecs_new(world, 0);
    ecs_set(world, background, Renderable, {1000, true});
    ecs_set_pair(world, background, Position, World, {0, 0});
    ecs_set(world, background, Box, {0, 2, ebox_start_x+txtWidth + 2, txtHeight - 1, FILL, {0, 0, 0, 200}});
    ecs_set_pair(world, background, Position, Local, {-ebox_start_x, 0});   
    ecs_add_pair(world, background, EcsChildOf, ebox);
    ecs_add(world, background, Background);

    if (prev)
    {
        // printf("%s has next %s\n", ecs_get_name(world, ebox), ecs_get_name(world, prev));
        if (prev && ecs_is_alive(world, prev))
        {
            SceneGraph* prevNode = ecs_get_mut(world, prev, SceneGraph);
            prevNode->next = ebox;
            log_trace("%s has next %s\n", ecs_get_name(world, prev), ecs_get_name(world, ebox));
            // printf("%s", ecs_get_name(world, root));
        }
    }

    return ebox;
}

ecs_entity_t lambda_function(lambda_parameters* params) 
{
    ecs_world_t* world = params->world;
    ecs_entity_t root = params->root;
    int depth = params->depth;
    int count = params->count;
    ecs_entity_t prev = params->prev;

    if (depth > 0)
    {
        ecs_entity_t SelectedNode = ecs_new_prefab(world, "selected_node_prefab");
        ecs_set(world, SelectedNode, Renderable, {2000, true});
        ecs_set_pair(world, SelectedNode, Position, World, {0, 0});
        ecs_set_pair(world, SelectedNode, Position, Local, {-2, 2});
        ecs_set(world, SelectedNode, Box, {0, 0, 0, 0, OUTLINE, {255, 255, 255, 255}});
        ecs_add(world, SelectedNode, Selected);
        ecs_entity_t node = create_node(params);
        if (count == 1)
        {
            ecs_entity_t selectedInst = ecs_new_entity(world, "selected_node");
            ecs_add_pair(world, selectedInst, EcsIsA, SelectedNode);
            ecs_add_pair(world, selectedInst, EcsChildOf, node);
            printf("Selected added to %s\n", ecs_get_name(world, root));
        }
        return node;
    }
    return NULL;
}

int get_children_count(ecs_world_t* world, ecs_entity_t root)
{
    int count = 0;
    ecs_iter_t it = ecs_children(world, root);
    while (ecs_children_next(&it)) {
        for (int i = 0; i < it.count; i++)
        {
            ecs_entity_t child = it.entities[i];
            count += 1 + get_children_count(world, child);
        }
    }
    return count;
}

typedef ecs_entity_t (*lambda_function_ptr)(lambda_parameters*);

lambda_output iter_depth_recursive(ecs_world_t* world, ecs_entity_t root, int depth, int count, ecs_entity_t prev, ecs_entity_t parentCreatedNode, lambda_data* data, lambda_function_ptr lambda)
{
    lambda_output lo_ret;
    lambda_parameters params = {world, root, depth, count, prev};
    ecs_entity_t createdNode = lambda(&params);
    data->nodes[count] = createdNode;
    data->parents[count] = parentCreatedNode;

    lo_ret.count = count;
    lo_ret.firstCreatedNode = createdNode;
    lo_ret.lastCreatedNode = createdNode;

    ecs_entity_t lastPrev = createdNode;
    ecs_iter_t it = ecs_children(world, root);

    depth++;
    while (ecs_children_next(&it)) {
        for (int i = 0; i < it.count; i++)
        {
            ecs_entity_t child = it.entities[i];
            lambda_output lo = iter_depth_recursive(world, child, depth, count + 1, lastPrev, createdNode, data, lambda);
            count = lo.count;
            lastPrev = lo.lastCreatedNode;
        }
    }

    lo_ret.count = count;
    lo_ret.lastCreatedNode = lastPrev;
    return lo_ret;
}

ecs_entity_t create_ui_element(ecs_world_t *world, const char *name) {
    ecs_entity_t e = ecs_new_id(world);
    ecs_set_name(world, e, name);
    return e;
}

int compare_z_index(
    ecs_entity_t e1,
    const Renderable *r1,
    ecs_entity_t e2,
    const Renderable *r2)
{
    (void)e1;
    (void)e2;
    return (r1->z_index > r2->z_index) - (r1->z_index < r2->z_index);
}

int compare_sc_index(
    ecs_entity_t e1,
    const SceneGraph *sc1,
    ecs_entity_t e2,
    const SceneGraph *sc2)
{
    (void)e1;
    (void)e2;
    return (sc1->index > sc2->index) - (sc1->index < sc2->index);
}

int main(int argc, char *argv[]) {
    log_set_quiet(true);
    ecs_world_t *world = ecs_init();
    ECS_IMPORT(world, FlecsMeta);
    input = ecs_set_name(world, 0, "input");

    ECS_META_COMPONENT(world, Transform);
    ECS_META_COMPONENT(world, Movable);
    ECS_META_COMPONENT(world, Color);
    ECS_META_COMPONENT(world, ScopeIndicator);
    ECS_META_COMPONENT(world, EventMouseClick);
    ECS_META_COMPONENT(world, Stats);
    ECS_META_COMPONENT(world, ConsumeEvent);
    ECS_META_COMPONENT(world, EventMouseMotion);
    ECS_META_COMPONENT(world, Textbox);
    ECS_META_COMPONENT(world, EventTextInput);
    ECS_META_COMPONENT(world, EventMouseWheel);
    ECS_META_COMPONENT(world, Position);
    ECS_META_COMPONENT(world, Size);
    ECS_META_COMPONENT(world, Cursor);
    ECS_META_COMPONENT(world, Text);
    ECS_META_COMPONENT(world, Test);
    ECS_META_COMPONENT(world, SceneGraph);
    ECS_META_COMPONENT(world, Renderable);
    ECS_META_COMPONENT(world, Line);
    ECS_META_COMPONENT(world, BoxMode);
    ECS_META_COMPONENT(world, Box);
    ECS_META_COMPONENT(world, ArrowStatus);

    ECS_COMPONENT_DEFINE(world, TestNormal);
    ECS_COMPONENT_DEFINE(world, Font);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, EventKeyInput);
    ECS_COMPONENT_DEFINE(world, SDL_Interface);
    ECS_COMPONENT_DEFINE(world, SceneGraphLayout);

    ECS_TAG_DEFINE(world, Selected);
    ECS_TAG_DEFINE(world, Symbol);
    ECS_TAG_DEFINE(world, World);
    ECS_TAG_DEFINE(world, Local);
    ECS_TAG_DEFINE(world, Background);
    // ecs_add_id(world, Symbol, EcsSymmetric);

    // TODO: Vertical/horizontal scene graph hierarchy lines
    // TODO: Toggle scene graph expanded

    ecs_entity_t ent = ecs_new_entity(world, "ent");
    ecs_add(world, ent, Textbox);
    ecs_add(world, ent, Text);
    ecs_add(world, ent, TestNormal);

    ecs_entity_t sceneGraph = ecs_new_entity(world, "scene_graph_interface");
    ecs_add(world, sceneGraph, SceneGraph);
    ecs_add(world, sceneGraph, SceneGraphLayout);
    

    ase_t* ase = cute_aseprite_load_from_file("table.ase", NULL);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return 1;
    }
    int width = dm.w;
    int height = dm.h;
    SDL_Window* window = SDL_CreateWindow("Book Simulator Online", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    // SDL_Window* window = SDL_CreateWindow("Book Simulator Online", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ase->w, ase->h, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

    ecs_entity_t sdl = ecs_set_name(world, 0, "sdl");
    ecs_set(world, sdl, SDL_Interface, {window, renderer});
    
    ECS_OBSERVER(world, GenTextTexture, EcsOnSet, Text, Renderable, Font(resource), SDL_Interface(sdl));

    // ecs_entity_t test = ecs_new(world, 0);
    // ecs_set(world, test, Transform, {0, 0, 64, 64});
    // ecs_set(world, test, Text, {"Bulwark", NULL, NULL, 1}); // TODO: OBSERVER construction
    // ecs_set(world, test, Textbox, {0, true});

    parseAsepriteFile(ase, world, sceneGraph, renderer);

    ecs_entity_t tb = ecs_new(world, 0);
    ecs_add(world, tb, Textbox);

    FILE* file = fopen("log.txt", "w");
    if (file == NULL) {
        printf("Failed to open log file.\n");
        return 1;
    }
    log_add_fp(file, 0);


    lambda_data sc_data;
    int graphCount = get_children_count(world, sceneGraph)+1;
    sc_data.nodes = calloc(graphCount, sizeof(ecs_entity_t));
    sc_data.parents = calloc(graphCount, sizeof(ecs_entity_t));
    lambda_output lo = iter_depth_recursive(world, sceneGraph, 0, 0, NULL, NULL, &sc_data, &lambda_function);
    for (int i = 0; i < graphCount; i++)
    {
        if (ecs_is_valid(world, sc_data.nodes[i]) && ecs_is_valid(world, sc_data.parents[i]))
        {
            // char* s2 = ecs_get_name(world, sc_data.parents[i]);
            ecs_add_pair(world, sc_data.nodes[i], EcsChildOf, sc_data.parents[i]);
        }
    }
    for (int i = 0; i < graphCount; i++)
    {
        if (ecs_is_valid(world, sc_data.nodes[i]) && ecs_is_valid(world, sc_data.parents[i]))
        {
            char* s1 = ecs_get_fullpath(world, sc_data.nodes[i]);
            // printf("Node %s\n", s1);
        }
    }

    // TODO: Create agents, auto update SceneGraph nodes
    // How to create agents?
    // Two approaches 
    // 1. Start with SceneGraph nodes and gather entities they represent
    // 2. Start with entities and then update the SceneGraph automatically when entities are modified...
    // Multiple SceneGraph user interface widgets, to represent queries, etc. Need to represent components in user interface
    // Iterate through SceneGraph, get agents nodes
    // Let's start with approach 1
    // Start by iterating through agents_node entity

    ecs_entity_t agentsNode = ecs_lookup(world, "agents_node");
    SceneGraph* agentsSceneGraph = ecs_get_mut(world, agentsNode, SceneGraph);
    ecs_iter_t agents_it = ecs_children(world, agentsNode);
    int agent_i = 0;
    int h_index = 0;
    ecs_entity_t nodes[100];
    ecs_entity_t parents[100];
    while (ecs_children_next(&agents_it)) {
        for (int i = 0; i < agents_it.count; i++)
        {
            ecs_entity_t child = agents_it.entities[i];
            if (ecs_is_valid(world, child))
            {
                // printf("Agent child %s\n", ecs_get_name(agents_it.world, child));
                SceneGraph* node = ecs_get(world, child, SceneGraph);
                if (node && ecs_is_valid(world, node->symbol))
                {
                    char* agent_name = ecs_get_name(world, node->symbol);
                    char* name = malloc((strlen(agent_name) + strlen("_agent") + 1) * sizeof(char)); // +1 for the null-terminator
                    strcpy(name, agent_name);
                    strcat(name, "_agent");
                    printf("Agent created %s\n", name);
                    ecs_entity_t ai = ecs_set_name(world, 0, name);
                    free(name);
                    ecs_set_pair(world, ai, Position, World, {0, 0});
                    ecs_set(world, ai, Movable, {false});
                    ecs_set(world, ai, Stats, {0, 0});

                    ecs_add_pair(world, ai, EcsChildOf, agentsSceneGraph->symbol);
                    ecs_add_pair(world, node->symbol, EcsChildOf, ai);
                    // We need to update the SceneGraph with the new node hierarchy in a way that can
                    // be reusable for generic updates
                    // How?
                    // Let's start by refactoring the code that generates a SceneGraph node to a utiliy function
                    char* aname = ecs_get_name(world, ai);
                    printf("Agent name gather %s\n", aname);
                    lambda_parameters lp = {world, ai, node->depth, node->index, node->prev};

                    // TODO: Check if node hierarchy is correct!
                    // I don't think it's actually reparenting.....zzzz
                    ecs_entity_t createdNode = create_node(&lp);

                    if (agent_i == 0)
                    {
                        agentsSceneGraph->next = createdNode;
                    }
                    SceneGraph* createdSceneGraph = ecs_get_mut(world, createdNode, SceneGraph);
                    createdSceneGraph->next = child;
                    node->prev = createdNode; // Update the pushed node to point here
                    // Now that the node is created...
                    // Let's update the rest of the graph to account for the changes...

                    // Iterate through all subsequent nodes and increment their index by one
                    ecs_entity_t e = createdSceneGraph->next;
                    while (ecs_is_valid(world, e))
                    {
                        SceneGraph* sc_next = ecs_get_mut(world, e, SceneGraph);
                        sc_next->index++;
                        e = sc_next->next;
                    }
                    
                    // printf("Entity %s has depth %d\n", ecs_get_name(world, createdNode), createdSceneGraph->depth);

                    // Increase depth of all children by one
                    e = createdSceneGraph->next;
                    int same_depth_count = 0;
                    while (ecs_is_valid(world, e))
                    {
                        SceneGraph* sc_next = ecs_get_mut(world, e, SceneGraph);
                        // printf("Entity %s has depth %d\n", ecs_get_name(world, e), sc_next->depth);
                        if (sc_next->depth <= createdSceneGraph->depth)
                        {
                            same_depth_count++;
                            if (same_depth_count > 1)
                            {
                                break;
                            }
                            nodes[h_index] = e;
                            parents[h_index] = createdNode;
                            h_index++;
                        }
                        sc_next->depth++;
                        // printf("UPDATE Entity %s now has depth %d\n", ecs_get_name(world, e), sc_next->depth);
                        e = sc_next->next;
                    }

                    // Increase children_count of all parents by one
                    e = createdSceneGraph->prev;
                    int lowest_depth = createdSceneGraph->depth;
                    ecs_entity_t first_parent = NULL;
                    while (ecs_is_valid(world, e))
                    {
                        SceneGraph* sc_prev = ecs_get_mut(world, e, SceneGraph);
                        if (sc_prev->depth < lowest_depth)
                        {
                            sc_prev->children_count++;
                            if (!ecs_is_valid(world, first_parent))
                            {
                                first_parent = e;
                                nodes[h_index] = createdNode;
                                parents[h_index] = first_parent;
                                h_index++;
                            }
                            lowest_depth = sc_prev->depth;
                            // printf("UPDATE Entity %s now has children count %d\n", ecs_get_name(world, e), sc_prev->children_count);
                        }
                        e = sc_prev->prev;
                    }
                }
            }
            agent_i++;
        }
    }
    for (int i = 0; i < h_index; i++)
    {
        ecs_add_pair(world, nodes[i], EcsChildOf, parents[i]);
    }


    ECS_SYSTEM(world, SetupSelectedNodeIndicator, EcsPostUpdate, Selected, Box, SceneGraph(parent), Text(parent));
    ECS_SYSTEM(world, Input, EcsPreUpdate, [inout] *());
    ECS_SYSTEM(world, MouseMovableSelection, EcsPostUpdate, Movable(parent), Position(parent, World), (Position, World), Sprite, EventMouseClick(input));
    ECS_SYSTEM(world, MouseMoveGrabbed, EcsPostUpdate, Movable, (Position, World), EventMouseMotion(input));
    ECS_SYSTEM(world, ConsumeEvents, EcsPostFrame, (ConsumeEvent, *));
    // ECS_SYSTEM(world, TransformCascadeHierarchy, EcsPreFrame, ?Position(parent|cascade), Transform);
    ecs_system(world, {
        .entity = ecs_entity(world, {
            .name = "TransformCascadeHierarchy",
            .add = { ecs_dependson(EcsOnUpdate) }
        }),
        .query.filter.terms = {
            {.id = ecs_pair(ecs_id(Position), World), .inout = EcsInOut },
            {.id = ecs_pair(ecs_id(Position), Local), .inout = EcsIn, .oper = EcsOptional },
            {.id = ecs_pair(ecs_id(Position), World), 
            .inout = EcsIn, 
            .src.flags = EcsParent | EcsCascade, 
            .oper = EcsOptional
            },
        },
        .callback = TransformCascadeHierarchy
    });

    ECS_SYSTEM(world, SceneGraphSettingsCascadeHierarchy, EcsPreFrame, ?SceneGraph(parent|cascade), SceneGraph);

    ECS_SYSTEM(world, TextboxEntry, EcsOnUpdate, Textbox, Text, EventTextInput(input));
    ECS_SYSTEM(world, HandleBackspace, EcsOnUpdate, Textbox, Text, EventKeyInput(input));

    ECS_SYSTEM(world, ToggleSceneGraphHierarchy, EcsOnUpdate, [inout] SceneGraph(parent), Selected, EventKeyInput(input));
    ECS_SYSTEM(world, KeyNavSceneGraph, EcsOnUpdate, SceneGraph(parent), Selected, EventKeyInput(input));
    ECS_SYSTEM(world, MouseWheelNavSceneGraph, EcsOnUpdate, SceneGraph(parent), Selected, EventMouseWheel(input));
    ECS_SYSTEM(world, UpdateArrowDirection, EcsPostUpdate, SceneGraph(parent), (Position, Local), Sprite, ArrowStatus, SDL_Interface(sdl));
    ECS_SYSTEM(world, UpdateSceneGraphLines, EcsPostUpdate, SceneGraph);
    ECS_SYSTEM(world, UpdateSymbolBackground, EcsOnUpdate, Box, (Position, Local), Background, SceneGraph(parent), Text(parent));

    ECS_SYSTEM(world, TextboxClick, EcsOnUpdate, Textbox, Position, Size, EventMouseClick(input));
    ECS_SYSTEM(world, TextboxCursorBlink, EcsOnUpdate, Textbox, Cursor);

    // ECS_SYSTEM(world, Render, EcsPostFrame, (Position, World), Sprite, SDL_Interface(sdl));
    ECS_SYSTEM(world, UpdateTextSurface, EcsPreFrame, (Position, World), Text, Font(resource), ?Renderable, SDL_Interface(sdl));
    ecs_system(world, {
        .entity = ecs_entity(world, {
            .name = "RenderCommander",
            .add = { ecs_dependson(EcsPostFrame) }
        }),
        .query = {
            .filter.expr = "(Position, World), Renderable, ?Sprite, ?Text, ?Line, ?Box, SDL_Interface(sdl)",
            // .filter.terms = {
            //     {.id = ecs_id(SDL_Interface), .entity = ecs_id(sdl) },
            //     {.id = ecs_id(Renderable) },
            //     {.id = ecs_id(Sprite), .oper = EcsOptional },
            //     {.id = ecs_id(Text), .oper = EcsOptional },
            //     {.id = ecs_id(Line), .oper = EcsOptional },
            //     {.id = ecs_id(Box), .oper = EcsOptional },
            // },
            .order_by = (ecs_order_by_action_t)compare_z_index,
            .order_by_component = ecs_id(Renderable)
        },
        .callback = RenderCommander
    });
    // ECS_SYSTEM(world, RenderBox, EcsPostFrame, (Position, World), Text, SceneGraph, SDL_Interface(sdl));
    // ECS_SYSTEM(world, RenderSelectedBox, EcsPostFrame, (Position, World), Text, SceneGraph, Selected, SDL_Interface(sdl));
    ECS_SYSTEM(world, RenderPresent, EcsPostFrame, SDL_Interface);
    
    ecs_query_t* qsc = ecs_query(world, {
            .filter.expr = "[inout] (Position, World), SceneGraph, ?SceneGraph(parent)",
            .order_by = (ecs_order_by_action_t)compare_sc_index,
            .order_by_component = ecs_id(SceneGraph)
        });

    while (ecs_progress(world, 0)) {

        ecs_iter_t sc_it = ecs_query_iter(world, qsc);
        int visible_index = 0;
        while (ecs_query_next(&sc_it)) {
            Position* p = ecs_field(&sc_it, Position, 1);
            SceneGraph* sc = ecs_field(&sc_it, SceneGraph, 2);
            SceneGraph* sc_parent = ecs_field(&sc_it, SceneGraph, 2);

            
            for (int i = 0; i < sc_it.count; i++)
            {
                char* n = ecs_get_name(sc_it.world, sc_it.entities[i]);
                // printf("%d %s %d\n", sc[i].is_expanded, n, visible_index);
                p[i].x = sc[i].depth*8.0f;
                if (sc[i].children_count)
                {
                    p[i].x += 11;
                }
                if (sc[i].is_expanded)
                {
                    visible_index++;
                    p[i].y = (visible_index-1)*12.0f-2; // TODO: Better padding config
                } else
                {
                    // Hide/disable
                    p[i].y = -10000;
                }
            }
        }
    }

    return ecs_fini(world);
}