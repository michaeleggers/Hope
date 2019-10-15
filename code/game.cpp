#include <stdlib.h>

#include "game.h"
#include "hope_draw.h"
#include "hope_draw.cpp"
#include "hope_ui.h"
#include "hope_ui.cpp"
#include "hope_ui_impl_render.cpp"
#include "input.h"
#include "input.cpp"
#include "stretchy_buffer.h"

#define JSON_PARSER_IMPLEMENTATION
#include "json_parser.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

global_var int gIsoMap[10000];
global_var HopeUIBinding gUiBinding;

global_var Entity gEntities[2];
#define MAX_ENTITIES 100
global_var Entity gFatguys[MAX_ENTITIES];
global_var int entity_count_fatguys;

#define MAX_BLOCKS 10
global_var Window g_blocks[MAX_BLOCKS];

global_var int g_default_framebuffer;
global_var int g_ui_framebuffer;

void initSpriteSheetFromJson(SpriteSheet * spriteSheet, char  * jsonFile)
{
    JsonDocument indyJson = json_parse(jsonFile);
    JsonNode * framesArray = json_get_value_by_name(indyJson.tree, "frames");
    JsonNode * arrayItem = json_get_child(framesArray);
    while (arrayItem) {
        JsonNode * filenameField = json_get_value_by_name(arrayItem, "filename");
        JsonNode * frameField = json_get_value_by_name(arrayItem, "frame");
        JsonNode * xOffset = json_get_value_by_name(frameField, "x");
        JsonNode * yOffset = json_get_value_by_name(frameField, "y");
        JsonNode * width = json_get_value_by_name(frameField, "w");
        JsonNode * height = json_get_value_by_name(frameField, "h");
        float xOffset_ = json_value_float(xOffset);
        float yOffset_ = json_value_float(yOffset);
        float width_ = json_value_float(width);
        float height_ = json_value_float(height);
        addSpriteFrame(spriteSheet, (int)xOffset_, (int)yOffset_, (int)width_, (int)height_);
        arrayItem = json_get_next_value(arrayItem);
    }
    JsonNode * metaInfoNode = json_get_value_by_name(indyJson.tree, "meta");
    JsonNode * frameTagsArray = json_get_value_by_name(metaInfoNode, "frameTags");
    JsonNode * nextFrameTag = json_get_child(frameTagsArray);
    while (nextFrameTag) {
        char * name = json_value_name(json_get_value_by_name(nextFrameTag, "name"));
        int from = (int)json_value_float(json_get_value_by_name(nextFrameTag, "from"));
        int to = (int)json_value_float(json_get_value_by_name(nextFrameTag, "to"));
        SpriteSequence sequence = {};
        sequence.start = from;
        sequence.end = to;
        strcpy(sequence.name, name);
        if (!strcmp(name, "walk_back")) {
            spriteSheet->sequences[WALK_BACK] = sequence;
        }
        else if (!strcmp(name, "walk_front")) {
            spriteSheet->sequences[WALK_FRONT] = sequence;
        }
        else if (!strcmp(name, "walk_right")) {
            spriteSheet->sequences[WALK_SIDE_RIGHT] = sequence;
            spriteSheet->sequences[WALK_SIDE_LEFT] = sequence;
            spriteSheet->sequences[WALK_SIDE_LEFT].flipHorizontal = true;
        }
        else if (!strcmp(name, "fight_walk")) {
            spriteSheet->sequences[FIGHT_WALK_RIGHT] = sequence;
            spriteSheet->sequences[FIGHT_WALK_LEFT] = sequence;
        }
        else if (!strcmp(name, "fight_ready")) {
            spriteSheet->sequences[FIGHT_READY] = sequence;
        }
        else if (!strcmp(name, "punch_high")) {
            spriteSheet->sequences[PUNCH_HIGH] = sequence;
        }
        else if (!strcmp(name, "punch_mid")) {
            spriteSheet->sequences[PUNCH_MID] = sequence;
        }
        else if (!strcmp(name, "punch_low")) {
            spriteSheet->sequences[PUNCH_LOW] = sequence;
        }
        else if (!strcmp(name, "hit_high")) {
            spriteSheet->sequences[HIT_HIGH] = sequence;
        }
        else if (!strcmp(name, "hit_mid")) {
            spriteSheet->sequences[HIT_MID] = sequence;
        }
        else if (!strcmp(name, "ko")) {
            spriteSheet->sequences[KO] = sequence;
        }
        else if (!strcmp(name, "dead")) {
            spriteSheet->sequences[DEAD] = sequence;
        }
        nextFrameTag = json_get_next_value(nextFrameTag);
    }
}

inline float randBetween(float lowerBound, float upperBound)
{
    float offset = lowerBound - 0.0f;
    float range = upperBound - lowerBound;
    return range/1.0f * (rand()/(float)RAND_MAX) + lowerBound;
}

// HOPE UI CALLBACKS
int get_window_width()
{
    return gPlatformAPI->getWindowDimensions().width;
}

int get_window_height()
{
    return gPlatformAPI->getWindowDimensions().height;
}

void * hope_memcpy(void * dest, void * src, int numBytes)
{
    int i = numBytes;
    char * destByte = (char *)dest;
    char * srcByte  = (char *)src;
    for (int i=0; i<numBytes; ++i)
    {
        destByte[i] = srcByte[i];
    }
    return dest;
}

Entity create_entity(int  spritesheet_id, float x_pos, float y_pos)
{
    Entity entity = {};
    entity.xPos = x_pos;
    entity.yPos = y_pos;
    entity.spritesheet = spritesheet_id;
    entity.cooldown = 0.f;
    entity.cooldownInit = 300.f;
    entity.frameTime = 0.f;
    entity.hitpoints = 100;
    entity.facingDirection = FACING_RIGHT;
    entity.currentSequence = WALK_SIDE_RIGHT;
    SpriteSheet * spritesheet = get_spritesheet_from_id(spritesheet_id);
    entity.currentFrame = spritesheet->sequences[entity.currentSequence].start;
    entity.direction = { };
    entity.velocity = 1.0f;
    return entity;
}

Window create_random_block(float y_offset);

void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re)
{
    gPlatformAPI = platform_api;
    init_input(input_device);
    hope_draw_init();
    
    srand(13434345);
    
    Rect window_dim = gPlatformAPI->getWindowDimensions();
    // create new framebuffer
    g_ui_framebuffer = new_framebuffer(re, window_dim.width, window_dim.height);
    g_default_framebuffer = new_framebuffer(re, 2560, 1440);
    
    float y_offset_last = 0.f;
    float last_width = 0.f;
    for (int i=0; i<MAX_BLOCKS; ++i) {
        float y_offset = randBetween(10.f, 100.f);
        g_blocks[i] = create_random_block(y_offset + y_offset_last + last_width);
        y_offset_last = y_offset;
        last_width = g_blocks[i].width;
    }
    
    // INIT HOPE UI
    gUiBinding.getWindowWidth = get_window_width;
    gUiBinding.getWindowHeight = get_window_height;
    gUiBinding.getMouseX = get_mouse_x;
    gUiBinding.getMouseY = get_mouse_y;
    gUiBinding.leftMouseButtonDown = leftMouseButtonDown;
    gUiBinding.leftMouseButtonPressed = leftMouseButtonPressed;
    hope_ui_init(&gUiBinding);
    
    // TTF font loading
    char* ttf_font = gPlatformAPI->readTextFile("..\\assets\\ttf\\ProggyClean.ttf");
    // stb_truetype texture baking API
    stbtt_fontinfo font;
    stbtt_InitFont(&font, (uint8_t*)ttf_font, 0);
    //float scale = stbtt_ScaleForMappingEmToPixels(&font, 15);
    
    stbtt_pack_context spc;
    unsigned char * pixels = (unsigned char *)malloc(sizeof(unsigned char)*1024*1024);
    if (!stbtt_PackBegin(&spc, pixels, 1024, 1024, 0, 1, 0))
        printf("failed to create packing context\n");
    
    //stbtt_PackSetOversampling(&spc, 2, 2);
    stbtt_packedchar chardata['~'-' '];
    stbtt_PackFontRange(&spc, (unsigned char*)ttf_font, 0, 13,
                        ' ', '~'-' ', chardata);
    stbtt_PackEnd(&spc);
    gTTFTexture = re->createTextureFromBitmap(pixels, 1024, 1024);
    free(pixels);
    
    gFontInfo.texture = gTTFTexture;
    memcpy(gFontInfo.chardata, chardata, ('~'-' ')*sizeof(stbtt_packedchar));
    gFontInfo.fontSize = 13.f;
    gFontInfo.numCharsInRange = '~' - ' ';
    gFontInfo.firstChar = ' ';
    // ! TTF FONT LOADING 
    
    
    // entity creation
    int indy_spritesheet = createSpriteSheet(re,
                                             "..\\assets\\indy\\indy_animation_project.png",
                                             0, 0,
                                             0, 0);
    char * jsonFile = gPlatformAPI->readTextFile("..\\assets\\indy\\indy_animation_project.json");
    initSpriteSheetFromJson(get_spritesheet_from_id(indy_spritesheet), jsonFile);
    Entity playerEntity = create_entity(indy_spritesheet, 0, 0);
    gEntities[0] = playerEntity;
    
    char * jsonFileFatGuy = gPlatformAPI->readTextFile("..\\assets\\fatguy\\fatguy.json");
    int fatguy_spritesheet = createSpriteSheet(re,
                                               "..\\assets\\fatguy\\fatguy.png",
                                               0, 0,
                                               0, 0);
    initSpriteSheetFromJson(get_spritesheet_from_id(fatguy_spritesheet), jsonFileFatGuy);
    Entity fatguyEntity = create_entity(fatguy_spritesheet, 10, 0);
    gEntities[1] = fatguyEntity;
    
    Rect window_dimensions = gPlatformAPI->getWindowDimensions();
    for (int i=0; i<MAX_ENTITIES; ++i) {
        float x_pos = randBetween(100, window_dimensions.width-100);
        float y_pos = randBetween(100, window_dimensions.height-100);
        Entity fatguy = create_entity(indy_spritesheet, x_pos, y_pos);
        fatguy.state = ENTITY_STATE_FIGHT_WALK_RIGHT;
        fatguy.direction = { randBetween(-1.f, 1.f), randBetween(-1.f, 1.f) };
        fatguy.direction = v2normalize(fatguy.direction);
        gFatguys[i] = fatguy;
    }
    // ! ENTITY CREATION
}

void update_entity_animation(Entity * entity, float dt)
{
    entity->frameTime += dt/1000.f; // dt in milliseconds
    SpriteSheet * spritesheet = get_spritesheet_from_id(entity->spritesheet);
    if (entity->frameTime >= 100.0f) {
        int * currentFramePtr = &entity->currentFrame;
        int endAnimPtr = spritesheet->sequences[entity->currentSequence].end;
        if (*currentFramePtr < endAnimPtr) {
            entity->currentFrame++;
        }
        else {
            entity->currentFrame = spritesheet->sequences[entity->currentSequence].start;
            
        }
        entity->frameTime = 0.f;
    }
}

void update_input(Entity * entity, float dt, Controller * controller)
{
    SpriteSheet * spritesheet = get_spritesheet_from_id(entity->spritesheet);
    update_entity_animation(entity, dt);
}

int compare_entities_y_pos(void const * a, void const * b) 
{
    return ((Entity*)a)->yPos < ((Entity*)b)->yPos;
}

void render_entities(Entity * entities, int entityCount)
{
    Entity sorted_entities[2];
    memcpy(sorted_entities, entities, 2*sizeof(Entity));
    qsort(sorted_entities, 2, sizeof(Entity), compare_entities_y_pos);
    Entity * entity = sorted_entities;
    for (int i=0; i<entityCount; i++) {
        bool flipHorizontally = false;
        if (entity->currentSequence == WALK_SIDE_LEFT) {
            flipHorizontally = true;
        }
#if 1
        Rect window_dimensions = gPlatformAPI->getWindowDimensions();
        float scale = 1.f;
        scale = entity->yPos/(float)window_dimensions.height;
        scale = (scale-1)*(-1.f);
#endif
        SpriteSheet * spritesheet = get_spritesheet_from_id(entity->spritesheet);
        pushTexturedRect(entity->xPos, entity->yPos,
                         3, 3,
                         {1, 1, 1},
                         spritesheet, entity->currentFrame,
                         flipHorizontally);
        entity++;
    }
}

void render_entities_ex(Entity * entities, int entityCount)
{
    
    Entity * entity = entities;
    for (int i=0; i<entityCount; i++) {
        bool flipHorizontally = false;
        if (entity->currentSequence == WALK_SIDE_LEFT) {
            flipHorizontally = true;
        }
#if 1
        Rect window_dimensions = gPlatformAPI->getWindowDimensions();
        float scale = 1.f;
        scale = entity->yPos/(float)window_dimensions.height;
        scale = (scale-1)*(-1.f);
#endif
        SpriteSheet * spritesheet = get_spritesheet_from_id(entity->spritesheet);
        pushTexturedRect(entity->xPos, entity->yPos,
                         1, 1,
                         {1, 1, 1},
                         spritesheet, entity->currentFrame,
                         flipHorizontally);
        entity++;
    }
}

Window create_random_block(float x_offset)
{
    Rect window_dim = gPlatformAPI->getWindowDimensions();
    float top       = randBetween((float)window_dim.height-300, (float)window_dim.height);
    float left      = window_dim.width + x_offset;
    float width     = randBetween(200, 500);
    float height    = window_dim.height - top;
    return {left, top, (float)width, (float)height, 0, 0};
}

void update_blocks()
{
    Window * last_block = &g_blocks[MAX_BLOCKS-1];
    if ( (last_block->x + last_block->width) < 0) {
        float last_x_offset = 0.f;
        for (int i=0; i<MAX_BLOCKS; ++i) {
            float x_offset = randBetween(0.f, 100.f);
            g_blocks[i] = create_random_block(x_offset+last_x_offset);
            last_x_offset += x_offset+g_blocks[i].width;
        }
    }
    
    Window * block = g_blocks;
    for (int i=0; i<MAX_BLOCKS; ++i) {
        block->x -= 10;
        block++;
    }
}

void render_blocks()
{
    Window * block = g_blocks;
    for (int i=0; i<MAX_BLOCKS; ++i) {
        pushFilledRect(block->x, block->y, block->width, block->height, {1,0,0});
        pushRect2D(block->x, block->y, block->x+block->width, block->y+block->height, {1,1,1}, 2.f);
        block++;
    }
}

void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re)
{
    // new rendering API proposal:
    
    update_input(&gEntities[0], dt, inputDevice->controller1);
    update_input(&gEntities[1], dt, inputDevice->controller2);
    Rect win_dimensions = gPlatformAPI->getWindowDimensions();
    int win_width = win_dimensions.width;
    int win_height = win_dimensions.height;
    for (int i=0; i<MAX_ENTITIES; ++i) {
        Entity * entity = &gFatguys[i];
        if ( (abs((int)entity->xPos-get_mouse_x()) < 300) &&
            (abs((int)entity->yPos-get_mouse_y()) < 300)) {
            entity->direction = v2normalize({entity->xPos-(float)get_mouse_x(), entity->yPos-(float)get_mouse_y()});
        }
        if (entity->xPos > win_width) {
            entity->direction.x *= -1;
            //entity->direction.y = randBetween(-1.f, 1.f);
        }
        else if (entity->xPos < 0) {
            entity->direction.x *= -1;
            //entity->direction.y = randBetween(-1.f, 1.f);
        }
        if (entity->yPos > win_height) {
            entity->direction.y *= -1;
            //entity->direction.x = randBetween(-1.f, 1.f);
        }
        else if (entity->yPos < 0) {
            entity->direction.y *= -1;
            //entity->direction.x = randBetween(-1.f, 1.f);
        }
        entity->xPos += entity->direction.x*entity->velocity;
        entity->yPos += entity->direction.y*entity->velocity;
        v2 direction = entity->direction;
        float theta = acos( (v2dot(direction, {1.f, 0.f})) / (v2length(direction)) );
        if (direction.y < 0) {
            if (direction.x < 0) {
                theta += PI/2.f;
            }
            else {
                theta += (6.f/4.f)*PI;
            }
        }
        float theta_in_deg = 180.f/PI*theta;
        if (direction.y > 0) {
            if (theta_in_deg < 45) {
                entity->currentSequence = WALK_SIDE_RIGHT;
            }
            else if (theta_in_deg >= 45 && theta_in_deg < 135) {
                entity->currentSequence = WALK_FRONT;
            }
            else {
                entity->currentSequence = WALK_SIDE_LEFT;
            }
        }
        else {
            if (theta_in_deg < 225) {
                entity->currentSequence = WALK_SIDE_LEFT;
            }
            else if (theta_in_deg >= 225 && theta_in_deg < 315) {
                entity->currentSequence = WALK_BACK;
            }
            else {
                entity->currentSequence = WALK_SIDE_RIGHT;
            }
        }
        update_entity_animation(entity, dt);
    }
    if (gEntities[0].xPos > gEntities[1].xPos) {
        if (gEntities[0].state != ENTITY_STATE_KO) {
            gEntities[0].facingDirection = FACING_LEFT;
        }
        if (gEntities[1].state != ENTITY_STATE_KO) {
            gEntities[1].facingDirection = FACING_RIGHT;
        }
    }
    else {
        if (gEntities[0].state != ENTITY_STATE_KO) {
            gEntities[0].facingDirection = FACING_RIGHT;
        }
        if (gEntities[1].state != ENTITY_STATE_KO) {
            gEntities[1].facingDirection = FACING_LEFT;
        }
    }
    
    update_blocks();
    
    hope_draw_start_frame(re);
    hope_ui_start();
    hope_ui_begin(GUID, HOPE_UI_LAYOUT_COLUMNS);
    if (hope_ui_button(GUID, "Toggle Animation Info")) {}
    if (hope_ui_button(GUID, "Toggle Secondary Window")) {}
    hope_ui_end();
    
    hope_ui_start();
    hope_ui_progress_bar(GUID, 0, 0, 500, 70, gEntities[0].hitpoints, 100);
    hope_ui_progress_bar(GUID, 1000, 0, 500, 70, gEntities[1].hitpoints, 100);
    pushLine2D(0,0,win_dimensions.width, win_dimensions.height, {1,1,1}, 2);
    hope_ui_end();
    
    set_render_target(re, g_ui_framebuffer);
    hope_ui_render();
    HopeUIDrawList * uiDrawList = hope_ui_get_drawlist();
    hopeUIImpLAddToDrawList(uiDrawList);
    reset_render_target(re);
    
    float ortho_matrix[16];
    hope_create_ortho_matrix(
        0.0f, (float)get_framebuffer_width(re, g_default_framebuffer),
        (float)get_framebuffer_height(re, g_default_framebuffer), 0.f,
        -1.f, 1.0f,
        ortho_matrix
        );
    //set_orthographic_projection_framebuffer(re, fbHandle, ortho_matrix);
    //useFramebuffer(fbHandle);
    set_render_target(re, g_default_framebuffer);
    //render_entities(gEntities, 2);
    render_entities_ex(gFatguys, MAX_ENTITIES);
    //pushFilledRect(0, 0, 20, 20, {1,0,0});
    //pushFilledRect(300, 180, 20, 20, {0,1,0});
    reset_render_target(re);
    render_blocks();
    
    Rect windowDimensions = gPlatformAPI->getWindowDimensions();
    hope_create_ortho_matrix(
        0.0f, (float)windowDimensions.width,
        windowDimensions.height, 0.f,
        -1.f, 1.0f,
        ortho_matrix
        );
    set_orthographic_projection(re, ortho_matrix);
    
    draw_from_framebuffer(re, g_ui_framebuffer, 0, 0, 0, 0);
    draw_from_framebuffer(re, g_default_framebuffer, 0, 0, 0, 0);
    
    re->endFrame(&gDrawList);
}

// NOTE(Michael): Not in use yet. Maybe reverse the control so that
// the game loop is controlled from the game and not the platform layer.
void game_loop()
{
    bool running = false;
    
    while (running)
    {
        update_messages();
    }
}