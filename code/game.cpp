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
#define MAX_ENTITIES 1000
global_var Entity gFatguys[MAX_ENTITIES];
global_var int entity_count_fatguys;


global_var int fbHandle;

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

struct HopeVector
{
    void * ptrToArray;
};

#define hope_vector(type, object) type * object; \
object = (type *)malloc(sizeof(type)*3 + 2*sizeof(int)); \
*(int *)object = 3; \
((int *)object)[1] = 0; \
object = (type *)&((int *)object)[2];

#define hope_vector_size(object) \
((int *)object)[-2];

struct Foo
{
    int a, b;
};

#define hope_vector_push_back(object, item) \
hope_vector_create( *((void **)&object)
/*
if (object == 0) { \
    object = (Foo *)malloc(sizeof(item)*3 + 2*sizeof(int)); \
    *(int *)object = 3; \
    ((int *)object)[1] = 0; \
} \
object = (Foo *)&((int *)object)[2]; \
object[((int *)object)[-1]] = item; \
((int *)object)[-1] += 1;
*/

//if (! ((int *)object)[-2] - ((int *)object)[-1] ) \
//object = (int *)realloc( (void *)object, ((int *)object)[-2] * 2 * sizeof(object[0]) ); \
//object[((int *)object)[-1]] = item; \
//((int *)object)[-1] += 1;


struct Bar
{
    char c;
};

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

void push (void ** foothing, int sizeOfItem, void * item)
{
#if 1
    void * cpyLocation = 0;
    if (0 == *foothing)
    {
        *foothing = malloc(10*sizeOfItem * 2*sizeof(int));
        *((int *)(*foothing)) = 10;
        ((int *)*foothing)[1] = 0;
        *foothing = &((int *)*foothing)[2];
        cpyLocation = *foothing;
    }
    else
    {
        int currentPos = ((int *)*foothing)[-1];
        cpyLocation = (char *)*foothing + currentPos*sizeOfItem;
    }
    ((int *)*foothing)[-1] += 1;
    hope_memcpy(cpyLocation, item, sizeOfItem);
#endif
}

#define push_macro(array, item) push((void **)&array, sizeof(*array), &item);

Foo aFunction()
{
    return {666,666};
}

typedef int (*someFunctionPtr)(void);

int lol(void)
{
    printf("L OOOOOO L\n");
    return 0;
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

void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re)
{
    gPlatformAPI = platform_api;
    init_input(input_device);
    
    // create new framebuffer
    fbHandle = newFramebuffer(re, 320, 200);
    
#if 0
    Foo fooItem = {1,2};
    Foo fooItem2 = {999, 888};
    Foo fooItem3 = {321, 123};
    Foo * myFoo = 0;
    push_macro(myFoo, fooItem);
    push_macro(myFoo, fooItem2);
    push_macro(myFoo, fooItem3);
    push_macro(myFoo, aFunction());
    Foo getFooItem = myFoo[0];
    
    Foo * anItem = (Foo *)malloc(sizeof(Foo));
    anItem->a = 777;
    anItem->b = 89;
    Foo ** array = 0;
    push_macro(array, anItem);
    push_macro(array, anItem);
    //push((void**)&myFoo, sizeof(fooItem), (void *)&fooItem);
    //push((void**)&myFoo, sizeof(fooItem2), (void *)&fooItem2);
    
    // This won't work in my implementation
    someFunctionPtr * funcArray1;
    push_macro(funcArray1, lol);
    
    someFunctionPtr * funcArray = 0;
    sb_push(funcArray, lol);
    someFunctionPtr lolFunc = funcArray[0];
    lolFunc();
#endif
    
#if 0    
    std::vector<Foo> fooVec;
    fooVec.push_back(someObj);
    fooVec[2];
    
    void * thing = malloc(sizeof(int)*10);
    ((int *)thing)[0] = 999;
    ((int *)thing)[1] = 777;
    *(Bar *)&((int *)thing)[2] = {'a'};
    
    Foo * foo = 0;
    //int fooSize = hope_vector_size(foo);
    Foo item = {42, 33};
    Foo item2 = {66, 99};
    //foo[0] = item;
    //hope_vector_push_back(foo, item);
    //hope_vector_push_back(foo, item);
    //hope_vector_push_back(foo, item);
    //hope_vector_push_back(foo, item2);
#endif
    
#if 0
    hope_vector(Bar, bar);
    int barSize = hope_vector_size(bar);
    Bar barItem = {'Q'};
    hope_vector_push_back(bar, barItem);
    hope_vector_push_back(bar, {'X'});
    bar[2] = {'Y'};
    
    hope_vector_push_back(foo, item);
    hope_vector_push_back(foo, item);
    
    
    char a = 'a';
    char b = 'b';
    char c = 'c';
    hope_vector(char, cfoo);
    hope_vector_push_back(cfoo, a);
    hope_vector_push_back(cfoo, b);
    hope_vector_push_back(cfoo, c);
#endif
    
    // INIT HOPE UI
    gUiBinding.getWindowWidth = get_window_width;gUiBinding.getWindowHeight = get_window_height;
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
    
    // init drawlist
    gDrawList.vtxBuffer = (Vertex *)malloc(sizeof(float)*1000*1024);
    if (!gDrawList.vtxBuffer)
        OutputDebugStringA("failed to create vtxBuffer\n");
    gDrawList.idxBuffer = (uint16_t *)malloc(sizeof(uint16_t)*1000*1024);
    if (!gDrawList.idxBuffer)
        OutputDebugStringA("failed to create idxBuffer\n");
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
        entity->direction = v2normalize({entity->xPos-(float)get_mouse_x(), entity->yPos-(float)get_mouse_y()});
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
    
    float ortho_matrix[16];
    Rect windowDimensions = gPlatformAPI->getWindowDimensions();
    hope_create_ortho_matrix(
        0.0f, (float)windowDimensions.width,
        (float)windowDimensions.height, 0.f,
        -1.f, 1.0f,
        ortho_matrix
        );
    set_orthographic_projection(re, ortho_matrix);
    //useFramebuffer(fbHandle);
    render_entities(gEntities, 2);
    render_entities_ex(gFatguys, MAX_ENTITIES);
    pushFilledRect(0, 0, 20, 20, {1,0,0});
    pushFilledRect(300, 180, 20, 20, {0,1,0});
    //defaultFramebuffer(fbHandle);
    
#if 1    
    hope_ui_start();
    hope_ui_begin(GUID, HOPE_UI_LAYOUT_COLUMNS);
    if (hope_ui_button(GUID, "Toggle Animation Info")) {}
    if (hope_ui_button(GUID, "Toggle Secondary Window")) {}
    hope_ui_end();
    
    hope_ui_start();
    hope_ui_progress_bar(GUID, 0, 0, 500, 70, gEntities[0].hitpoints, 100);
    hope_ui_progress_bar(GUID, 1000, 0, 500, 70, gEntities[1].hitpoints, 100);
    hope_ui_end();
#endif
    
    hope_ui_render();
    HopeUIDrawList * uiDrawList = hope_ui_get_drawlist();
    hopeUIImpLAddToDrawList(uiDrawList);
    
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