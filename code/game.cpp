#include "game.h"
#include "hope_draw.h"
#include "hope_draw.cpp"
#include "hope_ui.h"
#include "hope_ui.cpp"
#include "hope_ui_impl_render.cpp"
#include "stretchy_buffer.h"

#define JSON_PARSER_IMPLEMENTATION
#include "json_parser.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

global_var InputDevice* gInputDevice;

global_var int gIsoMap[10000];
global_var HopeUIBinding gUiBinding;

global_var Entity gEntities[2];
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
        sequence.currentFrame = from;
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
    spriteSheet->currentSequence = FIGHT_READY;
}

int nextLine(char* input, char *buffer, int *length)
{
    int l = 0;
    int lineFeedLength = 0;
    while (*input != '\0')
    {
        if (*input == '\r')
        {
            lineFeedLength++;
            input++;
            if (*input == '\0')
            {
            }
            else if (*input == '\n')
            {
                lineFeedLength++;
            }
            break;
        }
        else if (*input == '\n')
        {
            lineFeedLength++;
            break;
        }
        *buffer = *input;
        input++;
        buffer++;
        l++;
    }
    *buffer = '\0';
    *length = l;
    
    return lineFeedLength;
}

void getValue(char *input, char *buffer, int *length)
{
    int l = 0;
    while (*input != '\0' && (*input != ' ' && *input != '\t'))
    {
        *buffer = *input;
        input++;
        buffer++;
        l++;
    }
    *buffer = '\0';
    *length = l;
}

int skipWhitespaces(char* buffer)
{
    int skipped = 0;
    while (*buffer != '\0' && (*buffer == ' ' || *buffer == '\t'))
    {
        buffer++;
        skipped++;
    }
    return skipped;
}

Mesh loadMeshFromOBJ(char * objfile)
{
    Mesh mesh = {};
    char * objData = gPlatformAPI->readTextFile(objfile);
    char* c = objData;
    int vCount = 0;
    int nCount = 0;
    int stCount = 0;
    int iCount = 0;
    v3 positions[256];
    v3 normals[256];
    v2 UVs[256];
    v2 indices[256];
    while (*c != '\0')
    {
        char buffer[256];
        int length;
        int lineFeedLength = nextLine(c, buffer, &length);
        int pos = 0;
        pos += skipWhitespaces(&buffer[pos]);
        // TODO(Michael): trim whitespaces
        if (pos < length)
        {
            if (buffer[pos] == '#') // comment
            {}
            else
            {
                if (buffer[pos] == 'v') // vertex or normal
                {
                    int v3Count = 0;
                    float floatValue[3];
                    bool isVertex = false;
                    pos++; // advance over 'v' char
                    if (pos < length && buffer[pos] == 'n') // normal
                    {
                        printf("normal data (%d): ", nCount); 
                        pos++; // advance over 'n' char
                    }
                    else // vertex (position)
                    {
                        isVertex = true;
                        printf("vertex data (%d): ", vCount);
                    }
                    while (pos < length)
                    {
                        int skipped = skipWhitespaces(&buffer[pos]);
                        char valueBuffer[256];
                        int valueLength;
                        pos += skipped;
                        getValue(&buffer[pos], valueBuffer, &valueLength);
                        printf ("%s ", valueBuffer);
                        pos += valueLength;
                        float value = atof(valueBuffer);
                        floatValue[v3Count] = value;
                        v3Count++;
                        //printf("( %f ), ", value);
                    }
                    if (isVertex)
                    {
                        positions[vCount].x = floatValue[0];
                        positions[vCount].y = floatValue[1];
                        positions[vCount].z = floatValue[2];
                        vCount++;
                    }
                    else
                    {
                        normals[nCount].x = floatValue[0];
                        normals[nCount].y = floatValue[1];
                        normals[nCount].z = floatValue[2];
                        nCount++;
                    }
                    printf("\n");
                }
                else if (buffer[pos] == 'f') // indices
                {
                    int posIndex = 0;
                    int normalIndex = 0;
                    while (pos < length)
                    {
                        pos++; // advance over 'f' char
                        int skipped = skipWhitespaces(&buffer[pos]);
                        char valueBuffer[256];
                        int valueLength;
                        pos += skipped;
                        getValue(&buffer[pos], valueBuffer, &valueLength);
                        pos += valueLength;
                        
                        printf("%s ", valueBuffer);
                        
                        // extract index from value buffer, twice.
                        int lengthOfNumber;
                        posIndex = extractIndex(valueBuffer, valueLength, &lengthOfNumber);
                        valueLength -= lengthOfNumber;
                        normalIndex  = extractIndex(&valueBuffer[lengthOfNumber], valueLength, &lengthOfNumber); 
                        // TODO(Michael): check multiple versions of this shit
                        // for now just vertexPos // vertexNormal version
                        
                        // obj starts counting at 1
                        printf("(%d, ", posIndex-1);
                        printf("%d) ", normalIndex-1);
                        indices[iCount].x = posIndex-1; 
                        indices[iCount].y = normalIndex-1;
                        iCount++;
                    }
                    printf("\n");
                    
                }
                else
                    pos += length;
            }
        }
        c += length+lineFeedLength;
    }
    // done parsing
    
    // create buffer, that can be drawn by opengl.
    for (int i = 0; i < iCount; ++i)
    {
        int vertexIndex = indices[i].x;
        int normalIndex = indices[i].y;
        mesh.VVVNNNST[i] = 
        { 
            { positions[vertexIndex] },
            { normals[normalIndex] },
            { 0.f, 0.f }
        };
    }
    mesh.vertexCount = iCount;
    
    return mesh;
}

int extractIndex(char * input, int length, int * outLength)
{
    int result = 0;
    char * c = input;
    char buffer[256];
    char * b = buffer;
    int ol = 0;
    // skip all the non numerical chars first
    while (*c != '\0' &&
           *c < '0' || *c > '9')
        c++;
    // then try to get the number
    while (*c != '\0')
    {
        if (*c >= '0' && *c <= '9')
        {
            *b = *c;
            b++;
            ol++;
        }
        else
            break;
        c++;
    }
    *b = '\0';
    result = atoi(buffer);
    *outLength = ol;
    return result;
}

inline float randBetween(float lowerBound, float upperBound)
{
    float offset = lowerBound - 0.0f;
    float range = upperBound - lowerBound;
    return range/1.0f * (rand()/(float)RAND_MAX) + lowerBound;
}

ControllerKeycode toControllerKeycode(GameInput gameInput)
{
    switch (gameInput)
    {
        case FACE_LEFT  : return DPAD_LEFT; break;
        case FACE_RIGHT : return DPAD_RIGHT; break;
        case PUNCH      : return DPAD_A; break;
        default         : return DPAD_NONE; break;
    }
}

Keycode toKeyboardKeycode(GameInput gameInput)
{
    switch (gameInput)
    {
        case FACE_LEFT  : return ARROW_LEFT; break;
        case FACE_RIGHT : return ARROW_RIGHT; break;
        case PUNCH      : return ARROW_UP; break;
        default         : return KEYBOARD_NONE;
    }
}

#if 0
bool keyPressed(InputDevice* device, GameInput gameInput)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = device->keyboard;
            if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 1;
                return true;
            }
            else if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 0;
                return false;
            }
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = device->controller;
            if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 1;
                return true;
            }
            else if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 0;
                return false;
            }
            return false;
        }
        break;
        
        default:
        return false;
    }
}
#endif

bool keyPressed(Controller * controller, ControllerKeycode controllerKeycode)
{
    if (controllerKeycode == DPAD_NONE) return false;
    if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
    {
        controller->prevKeycodes[controllerKeycode] = 1;
        return true;
    }
    else if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
    {
        controller->prevKeycodes[controllerKeycode] = 0;
        return false;
    }
    return false;
}

#if 0
bool keyDown(InputDevice * device, GameInput gameInput)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = device->keyboard;
            if (keyboard->keycodes[keycode])
                return true;
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = device->controller;
            if (controller->keycodes[controllerKeycode])
                return true;
            return false;
        }
        break;
        
        default: return false;
    }
}
#endif

bool keyDown(InputDevice * device, Keycode keycode)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
            Keyboard* keyboard = device->keyboard;
            if (keyboard->keycodes[keycode])
                return true;
            return false;
        }
        break;
        
        default: return false;
    }
}

bool keyDown(Controller * controller, ControllerKeycode keycode)
{
    if (controller->keycodes[keycode])
        return true;
    return false;
}

#if 0
bool keyUp(InputDevice * device, GameInput gameInput)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
            Keycode keycode = toKeyboardKeycode(gameInput);
            Keyboard* keyboard = device->keyboard;
            if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 0;
                return true;
            }
            else if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
            {
                keyboard->prevKeycodes[keycode] = 1;
                return false;
            }
            return false;
        }
        break;
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(gameInput);
            if (controllerKeycode == DPAD_NONE) return false;
            Controller* controller = device->controller;
            if (!controller->keycodes[controllerKeycode] && controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 0;
                return true;
            }
            else if (controller->keycodes[controllerKeycode] && !controller->prevKeycodes[controllerKeycode])
            {
                controller->prevKeycodes[controllerKeycode] = 1;
                return false;
            }
            return false;
        }
        break;
        
        default:
        return false;
    }
}
#endif

// HOPE UI CALLBACKS
int get_window_width()
{
    return gPlatformAPI->getWindowDimensions().width;
}

int get_window_height()
{
    return gPlatformAPI->getWindowDimensions().height;
}

int get_mouse_x()
{
    return gInputDevice->mouse->x;
}

int get_mouse_y()
{
    return gInputDevice->mouse->y;
}

bool leftMouseButtonDown()
{
    Mouse * mouse = gInputDevice->mouse;
    if (mouse->keycodes[LBUTTON_DOWN])
        return true;
    return false;
}

bool leftMouseButtonPressed()
{
    Mouse * mouse = gInputDevice->mouse;
#if 1    
    if (mouse->keycodes[LBUTTON_DOWN] &&
        !mouse->prevKeycodes[LBUTTON_DOWN])
    {
        mouse->prevKeycodes[LBUTTON_DOWN] = 1;
        return false;
    }
    else if (!mouse->keycodes[LBUTTON_DOWN] &&
             mouse->prevKeycodes[LBUTTON_DOWN])
    {
        mouse->prevKeycodes[LBUTTON_DOWN] = 0;
        return true;
    }
    return false;
#endif
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

void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re)
{
    gPlatformAPI = platform_api;
    gInputDevice = input_device;
    
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
#if 0
    // load TTF Font
    stbtt_fontinfo font;
    unsigned char *ttfBitmap = 0;
    stbtt_InitFont(&font, (uint8_t*)ttf_font, stbtt_GetFontOffsetForIndex((uint8_t*)ttf_font, 0));
    int w, h;
    unsigned char * ttfTexture = (unsigned char *)malloc(sizeof(unsigned char)*1024*1024);
    
    for (int codePoint = 0; codePoint < 26; ++codePoint)
    {
        ttfBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 13.0f), codePoint + 65, &w, &h, 0, 0);
        for (int i = 0; i<h; ++i)
        {
            for (int k = 0; k<w; ++k)
            {
                ttfTexture[ (codePoint*64/1024)*64*1024+i*1024 + (k+(codePoint%16)*64)] = ttfBitmap[(h-i)*w+k];
            }
        }
    }
    gTTFTexture = re->createTextureFromBitmap(ttfTexture, 1024, 1024);
    free(ttfTexture);
#endif
    
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
    
    // init resources for new rendering API
    gFontSpriteSheet = createSpriteSheet(re, 
                                         "..\\assets\\kromagrad_16x16.png",
                                         0, 0,
                                         0, 0);
    for (int i = 0;
         i < 944/16; // number of glyphs
         i++)
    {
        addSpriteFrame(&gFontSpriteSheet, 0 + i*16, 0, 16, 16);
    }
    
    gTilesSpriteSheet = createSpriteSheet(re,
                                          "..\\assets\\dungeon_iso_tiles.png",
                                          0, 0,
                                          0, 0);
    for (int i = 0;
         i < 1536/64; // number of tiles
         i++)
    {
        addSpriteFrame(&gTilesSpriteSheet, 0 + i*64, 0, 64, 32);
    }
    
    gIndySpriteSheet = createSpriteSheet(re,
                                         "..\\assets\\indy\\indy_animation_project.png",
                                         0, 0,
                                         0, 0);
    char * jsonFile = gPlatformAPI->readTextFile("..\\assets\\indy\\indy_animation_project.json");
    initSpriteSheetFromJson(&gIndySpriteSheet, jsonFile);
    Entity playerEntity = {};
    playerEntity.xPos = 0.f;
    playerEntity.yPos = 0.f;
    playerEntity.spriteSheet = &gIndySpriteSheet;
    playerEntity.cooldown = 0.f;
    playerEntity.cooldownInit = 300.f;
    playerEntity.frameTime = 0.f;
    playerEntity.hitpoints = 100;
    playerEntity.facingDirection = FACING_RIGHT;
    gEntities[0] = playerEntity;
    
    char * jsonFileFatGuy = gPlatformAPI->readTextFile("..\\assets\\fatguy\\fatguy.json");
    Entity fatguyEntity = {};
    gFatguySpriteSheet = createSpriteSheet(re,
                                           "..\\assets\\fatguy\\fatguy.png",
                                           0, 0,
                                           0, 0);
    initSpriteSheetFromJson(&gFatguySpriteSheet, jsonFileFatGuy);
    fatguyEntity.spriteSheet = &gFatguySpriteSheet;
    fatguyEntity.xPos = 10.f;
    fatguyEntity.yPos = 0.f;
    fatguyEntity.cooldown = 0.f;
    fatguyEntity.cooldownInit = 300.f;
    fatguyEntity.frameTime = 0.f;
    fatguyEntity.hitpoints = 100;
    fatguyEntity.facingDirection = FACING_LEFT;
    gEntities[1] = fatguyEntity;
    
    // init drawlist
    gDrawList.vtxBuffer = (Vertex *)malloc(sizeof(float)*1000*1024);
    if (!gDrawList.vtxBuffer)
        OutputDebugStringA("failed to create vtxBuffer\n");
    gDrawList.idxBuffer = (uint16_t *)malloc(sizeof(uint16_t)*1000*1024);
    if (!gDrawList.idxBuffer)
        OutputDebugStringA("failed to create idxBuffer\n");
}

// 1000 0000
// 8    0 
char* ftoa(float n)
{
    int decimalPart = (int)n;
    static char b[32] = {};
    int i = 0;
    if (decimalPart < 0)
    {
        b[i] = '-';
        decimalPart *= -1;
        i++;
    }
    for (; decimalPart != 0; i++)
    {
        b[i] = (char)((decimalPart % 10) + '0');
        decimalPart /= 10;
    }
    
    return b;
}

void update_input(Entity * entity, float dt, Controller * controller)
{
    entity->frameTime += dt/1000.f; // dt in milliseconds
    if (entity->frameTime >= 200.0f) {
        if (entity->state == ENTITY_STATE_KO) {
            int * currentFramePtr = &entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].currentFrame;
            int endAnimPtr = entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].end;
            if (*currentFramePtr < endAnimPtr) {
                entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].currentFrame++;
            }
        }
        else {
            entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].currentFrame++;
        }
        entity->frameTime = 0.f;
    }
    if (entity->cooldown <= 100.f) {
        entity->spriteSheet->currentSequence = FIGHT_READY;
        entity->state = ENTITY_STATE_FIGHT_READY;
    }
    
    if (keyDown(controller, DPAD_RIGHT)) {
        if (entity->cooldown <= 0.f) {
            entity->spriteSheet->currentSequence = FIGHT_WALK_RIGHT;
            entity->xPos += 10;
            entity->state = ENTITY_STATE_FIGHT_WALK_RIGHT;
        }
    }
    if (keyDown(controller, DPAD_LEFT)) {
        if (entity->cooldown <= 0.f) {
            entity->spriteSheet->currentSequence = FIGHT_WALK_LEFT;
            entity->xPos -= 10;
            entity->state = ENTITY_STATE_FIGHT_WALK_LEFT;
        }
    }
    if (keyDown(controller, DPAD_UP)) {
        if (entity->cooldown <= 0.f) {
            entity->spriteSheet->currentSequence = FIGHT_WALK_RIGHT;
            entity->yPos += 10;
            entity->state = ENTITY_STATE_FIGHT_WALK_RIGHT;
        }
    }
    if (keyDown(controller, DPAD_DOWN)) {
        if (entity->cooldown <= 0.f) {
            entity->spriteSheet->currentSequence = FIGHT_WALK_LEFT;
            entity->yPos -= 10;
            entity->state = ENTITY_STATE_FIGHT_WALK_LEFT;
        }
    }
    if (keyPressed(controller, DPAD_A)) {
        if (entity->cooldown <= 0.f) {
            if (keyDown(controller, DPAD_UP)) {
                entity->spriteSheet->currentSequence = PUNCH_HIGH;
                entity->state = ENTITY_STATE_PUNCH_HIGH;
                entity->cooldown = entity->cooldownInit;
            }
            else if (keyDown(controller, DPAD_DOWN)) {
                entity->spriteSheet->currentSequence = PUNCH_LOW;
                entity->state = ENTITY_STATE_PUNCH_LOW;
                entity->cooldown = entity->cooldownInit;
            }
            else {
                entity->spriteSheet->currentSequence = PUNCH_MID;
                entity->state = ENTITY_STATE_PUNCH_MID;
                entity->cooldown = entity->cooldownInit;
            }
        }
    }
    
    entity->cooldown -= dt/1000.f;
    if (entity->cooldown < 0.f) {
        entity->cooldown = 0.f;
    }
    
    int * currentFramePtr = &entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].currentFrame;
    int startAnimPtr = entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].start;
    int endAnimPtr = entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].end;
    if (*currentFramePtr > endAnimPtr) *currentFramePtr = startAnimPtr;
    if (*currentFramePtr < startAnimPtr)  *currentFramePtr = endAnimPtr;
}

void check_collision(Entity * entity1, Entity * entity2)
{
    if (entity1->xPos+3 > entity2->xPos &&
        entity1->xPos < entity2->xPos+6) {
        if (entity1->state == ENTITY_STATE_PUNCH_HIGH) {
            if (entity2->state == ENTITY_STATE_FIGHT_READY) {
                entity2->spriteSheet->currentSequence = HIT_HIGH;
                entity2->cooldown = entity2->cooldownInit;
                entity2->hitpoints -= 10;
            }
            entity1->state = ENTITY_STATE_FIGHT_READY;
        }
        else if (entity1->state == ENTITY_STATE_PUNCH_MID) {
            if (entity2->state == ENTITY_STATE_FIGHT_READY) {
                entity2->spriteSheet->currentSequence = HIT_MID;
                entity2->cooldown = entity2->cooldownInit;
                entity2->hitpoints -= 10;
            }
            entity1->state = ENTITY_STATE_FIGHT_READY;
        }
    }
    
    if (entity2->hitpoints <= 0) {
        entity2->state = ENTITY_STATE_KO;
        entity2->spriteSheet->currentSequence = KO;
        entity2->cooldown = entity2->cooldownInit;
    }
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
        bool flipHorizontally = entity->facingDirection == FACING_RIGHT ? false : true;
#if 0
        Rect window_dimensions = gPlatformAPI->getWindowDimensions();
        float scale = 1.f;
        scale = entity->yPos/(float)window_dimensions.height;
        scale = (scale-1)*(-1.f);
#endif
        pushTexturedRect(entity->xPos, entity->yPos,
                         500, 500,
                         {1, 1, 1},
                         entity->spriteSheet, entity->spriteSheet->sequences[entity->spriteSheet->currentSequence].currentFrame,
                         flipHorizontally);
        entity++;
    }
}

void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re)
{
    // new rendering API proposal:
    
    // render text
    static float xTextScale = 0.0f;
    static float yTextScale = 0.0f;
    float multiplicator = 1;
    if (xTextScale > 10) xTextScale *= -1;
    if (yTextScale > 10) yTextScale *= -1;
    xTextScale += dt/1000*0.001f;
    yTextScale += dt/1000*0.001f;
    char uiAngleBuffer[256];
    strcpy(uiAngleBuffer, ftoa(-15.1f));
    
    static float advance = 0.f;
    if (advance > 1080.+120.f)
        advance = 0.f;
    advance += 1.0f;
    
    update_input(&gEntities[0], dt, inputDevice->controller1);
    update_input(&gEntities[1], dt, inputDevice->controller2);
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
    check_collision(&gEntities[0], &gEntities[1]);
    check_collision(&gEntities[1], &gEntities[0]);
    
    float ortho_matrix[16];
    Rect windowDimensions = gPlatformAPI->getWindowDimensions();
    hope_create_ortho_matrix(
        0.0f, (float)windowDimensions.width,
        0.0f, (float)windowDimensions.height,
        -1.f, 1.0f,
        ortho_matrix
        );
    set_orthographic_projection(re, ortho_matrix);
    //useFramebuffer(fbHandle);
    render_entities(gEntities, 2);
    pushFilledRect(0, 0, 20, 20, {1,0,0});
    pushFilledRect(300, 180, 20, 20, {0,1,0});
    //defaultFramebuffer(fbHandle);
    
#if 0    
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