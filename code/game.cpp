#include "game.h"
#include "hope_draw.h"
#include "hope_draw.cpp"
#include "hope_ui.h"
#include "hope_ui.cpp"
#include "hope_ui_impl_render.cpp"

#include "common_os.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

global_var InputDevice* gInputDevice;

global_var int gIsoMap[10000];
global_var HopeUIBinding gUiBinding;

Background loadBackground(char * file)
{
    Background bg;
    // check if file exists!
    if (!fileExists(file))
    {
        bg.imageFile = 0;
        printf("fuck! Failed to load %s!\n", file);
    }
    else
    {
        bg.imageFile = file;
    }
    return bg;
}

Object loadObject(char * file)
{
    Object obj;
    // check if file exists!
    if (!fileExists(file))
    {
        obj.imageFile = 0;
        printf("fuck! Failed to load %s!\n", file);
    }
    else
    {
        obj.imageFile = file;
    }
    return obj;
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
        case TURN_LEFT  : return DPAD_LEFT;
        case TURN_RIGHT : return DPAD_RIGHT;
        case ACCELERATE    : return DPAD_A;
        default          : return DPAD_NONE;
    }
}

Keycode toKeyboardKeycode(GameInput gameInput)
{
    switch (gameInput)
    {
        case TURN_LEFT:  return ARROW_LEFT; break;
        case TURN_RIGHT: return ARROW_RIGHT; break;
        case ACCELERATE: return ARROW_UP; break;
        default: return KEYBOARD_NONE;
    }
}

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

#define hope_vector(type, object) type * object;  \
object = (type *)malloc(sizeof(type)*123 + 2*sizeof(int)); \
*(int *)object = 123; \
*(int *)&object[1] = 0; \
object = &((type *)object)[2];

#define hope_vector_size(object) \
*(int *)&(object[-2]);

#define hope_vector_push_back(object, item) \
object[*(int *)&object[-1]] = item; \
*(int *)&object[-1] += 1;

struct Foo
{
    int a, b;
};

struct Bar
{
    char c;
};

void game_init(PlatformAPI* platform_api, InputDevice* input_device, refexport_t* re)
{
    gPlatformAPI = platform_api;
    gInputDevice = input_device;
    
#if 0    
    std::vector<Foo> fooVec;
    fooVec.push_back(someObj);
    fooVec[2];
#endif
    
    hope_vector(Foo, foo);
    Foo item = {42, 33};
    int fooSize = hope_vector_size(foo);
    foo[0] = item;
    Foo item2 = {66, 99};
    hope_vector_push_back(foo, item2);
    hope_vector_push_back(foo, item);
    hope_vector_push_back(foo, item);
    
    hope_vector(Bar, bar);
    int barSize = hope_vector_size(bar);
    
    char a = 'a';
    char b = 'b';
    char c = 'c';
    hope_vector(char, cfoo);
    hope_vector_push_back(cfoo, a);
    hope_vector_push_back(cfoo, b);
    hope_vector_push_back(cfoo, c);
    
    // INIT HOPE UI
    gUiBinding.getWindowWidth = get_window_width;gUiBinding.getWindowHeight = get_window_height;
    gUiBinding.getMouseX = get_mouse_x;
    gUiBinding.getMouseY = get_mouse_y;
    gUiBinding.leftMouseButtonDown = leftMouseButtonDown;
    gUiBinding.leftMouseButtonPressed = leftMouseButtonPressed;
    hope_ui_init(&gUiBinding);
    
    // TTF font loading
    char* ttf_font = gPlatformAPI->readTextFile("..\\assets\\ttf\\efmi.ttf");
#if 0
    // load TTF Font
    stbtt_fontinfo font;
    unsigned char *ttfBitmap = 0;
    stbtt_InitFont(&font, (uint8_t*)ttf_font, stbtt_GetFontOffsetForIndex((uint8_t*)ttf_font, 0));
    int w, h;
    unsigned char * ttfTexture = (unsigned char *)malloc(sizeof(unsigned char)*1024*1024);
    
    for (int codePoint = 0; codePoint < 26; ++codePoint)
    {
        ttfBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 80.0f), codePoint + 65, &w, &h, 0, 0);
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
    stbtt_PackFontRange(&spc, (unsigned char*)ttf_font, 0, 24,
                        ' ', '~'-' ', chardata);
    stbtt_PackEnd(&spc);
    gTTFTexture = re->createTextureFromBitmap(pixels, 1024, 1024);
    free(pixels);
    
    gFontInfo.texture = gTTFTexture;
    memcpy(gFontInfo.chardata, chardata, ('~'-' ')*sizeof(stbtt_packedchar));
    gFontInfo.fontSize = 24.f;
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
    
    // init drawlist
    gDrawList.vtxBuffer = (Vertex *)malloc(sizeof(float)*1000*1024);
    if (!gDrawList.vtxBuffer)
        OutputDebugStringA("failed to create vtxBuffer\n");
    gDrawList.idxBuffer = (uint16_t *)malloc(sizeof(uint16_t)*1000*1024);
    if (!gDrawList.idxBuffer)
        OutputDebugStringA("failed to create idxBuffer\n");
}

void addEntity(Entity * entity)
{
    switch (entity->entityType)
    {
        case SPRITE_E:
        {
            if (gNumSpriteEntities >= MAX_SPRITES) return;
            gSpriteEntityList[gNumSpriteEntities] = *entity;
            gNumSpriteEntities++;
        }
        break;
        
        case MESH_E:
        {
            if (gNumMeshEntities >= MAX_MESHES) return;
            gMeshEntityList[gNumMeshEntities] = *entity;
            gNumMeshEntities++;
        }
        
        case PLAYER_E:
        {
            gPlayerEntity = *entity;
        }
        break;
    }
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

void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re)
{
    // new rendering API proposal:
    // beginRender(renderDevice, renderTarget);
    
    // render text
    static float xTextScale = 0.0f;
    static float yTextScale = 0.0f;
    float multiplicator = 1;
    if (xTextScale > 10) xTextScale *= -1;
    if (yTextScale > 10) yTextScale *= -1;
    xTextScale += dt/1000*0.001f;
    yTextScale += dt/1000*0.001f;
    //pushText("Educating the mind without educating the heart is no education at all.", -19, 0, .5f, 1.f, {1,0,0}, &gFontSpriteSheet);
    //pushText("H E L L O", 0, 0, 1, 7, {1, 0.4f, 0}, &gFontSpriteSheet);
    //pushText("and even more bitmap text", xTextScale, yTextScale, 1, 1, {0.1f, 0.7f, 0.2f}, &gFontSpriteSheet);
    //pushText("moar text!", -10, -5, abs(sinf(xTextScale)), 1, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    char uiAngleBuffer[256];
    strcpy(uiAngleBuffer, ftoa(-15.1f));
    //pushText("ship angle: ", -15, 8, 1.f, 1.f, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    //pushText(uiAngleBuffer, -15, 7, 1.f, 1.f, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    
    // HACK(Michael): 'camera'-controlls to move around the iso-map
    static float xOffset = 0.0f;
    static float yOffset = 0.0f;
    if (keyDown(inputDevice, ARROW_LEFT))
        xOffset += 0.1f;
    if (keyDown(inputDevice, ARROW_RIGHT))
        xOffset -= 0.1f;
    if (keyDown(inputDevice, ARROW_UP))
        yOffset -= 0.1f;
    if (keyDown(inputDevice, ARROW_DOWN))
        yOffset += 0.1f;
    
    /*
    // render iso-map
    for (int y = 0; y < 100; y++)
    {
        for (int x = 0; x < 100; x++)
        {
            pushTexturedRect(x -y+xOffset, x*(-0.5f) - y*0.5f + yOffset,
                     1, 1,
                     {1, 1, 1},
                     &gTilesSpriteSheet, gIsoMap[100*y + x]);
        }
    }
    
    pushTexturedRect(-18, 5,1, 1,{1, 1, 1},&gTilesSpriteSheet, 0);
    pushText("BC", -10, -5, abs(sinf(xTextScale)), 1, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    pushTexturedRect(-16, 5,1, 1,{1, 1, 1},&gTilesSpriteSheet, 19);
    pushTexturedRect(-14, 5,1, 1,{1, 1, 1},&gTilesSpriteSheet, 20);
    pushText("DC", -5, -5, abs(sinf(xTextScale)), 1, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    pushText("EF", 0, -5, abs(sinf(xTextScale)), 1, {0.1f, 0.4f, 0.5f}, &gFontSpriteSheet);
    */
#if 0
    pushText("rendering 10.000 tiles!", -5, 5, 1, 1, {0.8f, 0.1f, 0.1f}, &gFontSpriteSheet);
    pushLine2D(0.f, 1.f, 10.f, 1.f, {1,0,0},3);
    pushLine2D(0.f, 5.f, 10.f, 5.f, {0,1,0},5);
    pushLine2D(-10.f, -10.f, 0.f, 0.f, {0,0,1},3);
    pushLine2D(-10.f, -10.f, 10.f, 0.f, {0,0,1},3);
    pushText("rendering 10.000 tiles!", -5, 5, 1, 1, {0.8f, 0.1f, 0.1f}, &gFontSpriteSheet);
    pushRect2D(0.0f, 0.0f, 7.0f, -7.0f, {1,0,0}, 2.5f);
    //pushTexturedRect(x, y, scale, &bitmapData);
    pushFilledRect(-5.0f, 0.0f, 3.0f, 5.0f, {1,1,0});
    pushFilledRect(-10.0f, 0.0f, 1.0f, 4.0f, {1,1,0});
    pushTexturedRect(-18, 0, 20, 20, {1, 1, 1}, gTTFTexture);
#endif
    
    pushLine2D(0.f, 900.f, 10.f, 900.f, {1,1,0},7);
    static float advance = 0.f;
    if (advance > 1080.+120.f)
        advance = 0.f;
    advance += 1.3f;
    pushTexturedRect(-advance, 0, 10, 10, {1, 1, 1}, &gTilesSpriteSheet, 0);
    pushTTFText("Test1\nLinebreak1\0", 960, advance, {1.f,1.f, 1.f}, &gFontInfo);
    pushTTFText("Test2\nLinebreak2\0", 960, advance-80.f, {0.f,1.f, 0.f}, &gFontInfo);
    pushTTFText("Test3\nLinebreak3\0", 960, advance-160.f, {0.f,0.f, 1.f}, &gFontInfo);
    pushFilledRect(0.0f, 0.0f, 1920.0f, 100.0f, {1,0,1});
    pushFilledRect(0.0f, 80.f, 1920.0f, 20.f, {0.0f, 0, 1.0f});
    
    // Some button with logic
    hope_ui_begin(GUID);
    static bool buttonClicked = false;
    if (hope_ui_button(GUID, "Das ist Button A\0", {0, 0, 300, 100}))
        buttonClicked = !buttonClicked;
    if (hope_ui_button(GUID, "Button B\0", {0, 200, 300, 300}))
        buttonClicked = !buttonClicked;
    if (hope_ui_button(GUID, "Dieser Text ist etwas zu lang fuer den Button!\0", {600, 200, 900, 300}))
        buttonClicked = !buttonClicked;
    if (buttonClicked)
        pushTTFText("Das ist Button AQ\0", 960, 540, {1,1,1}, &gFontInfo);
    hope_ui_end();
    hope_ui_render();
    HopeUIDrawList * uiDrawList = hope_ui_get_drawlist();
    hopeUIImpLAddToDrawList(uiDrawList);
    
    gRefdef.playerEntity = &gPlayerEntity;
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