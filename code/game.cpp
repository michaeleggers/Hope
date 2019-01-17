#include "game.h"
#include "common_os.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global_var int gNumRooms;
global_var Entity gSpriteEntityList[MAX_SPRITES];
global_var Entity gMeshEntityList[MAX_MESHES];
global_var int gNumSpriteEntities;
global_var int gNumMeshEntities;
global_var Refdef gRefdef;
global_var PlatformAPI* gPlatformAPI;

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

struct v3
{
    float x, y, z;
};

struct v2
{
    float x, y;
};

struct Vertex
{
    v3 position;
    v3 normal;
    v2 UVs;
};

Mesh loadMeshFromOBJ(char * objfile)
{
    Mesh mesh = {};
    char * objData = gPlatformAPI->readTextFile(objfile);
    char* c = objData;
    int vCount = 0;
    int nCount = 0;
    int stCount = 0;
    v3 positions[256];
    v3 normals[256];
    v2 UVs[256];
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
                        int posIndex = extractIndex(valueBuffer, valueLength, &lengthOfNumber);
                        valueLength -= lengthOfNumber;
                        int normalIndex  = extractIndex(&valueBuffer[lengthOfNumber], valueLength, &lengthOfNumber); 
                        // TODO(Michael): check multiple versions of this shit
                        // for now just vertexPos // vertexNormal version
                        printf("(%d, ", posIndex);
                        printf("%d) ", normalIndex);
                    }
                    printf("\n");
                }
                else
                    pos += length;
            }
        }
        c += length+lineFeedLength;
    }
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

void game_init(PlatformAPI* platform_api, refexport_t* re)
{
    gPlatformAPI = platform_api;
    int res = re->addTwoNumbers(1, 11);
    printf("addTwoNumbers: %d\n", res);
    Entity azores;
    memcpy(azores.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores.entityType = SPRITE_E;
    int x, y, n;
    unsigned char * azoresImageData = 0;
    if (fileExists("..\\assets\\azores.png"))
        azoresImageData = stbi_load("..\\assets\\azores.png", &x, &y, &n, 4);
    Sprite azoresSprite = re->registerSprite("..\\assets\\azores.png",
                                             azoresImageData,
                                             560, 144,
                                             0, 0,
                                             560, 144);
    azores.transform.xPos = 5;
    azores.transform.yPos = 5;
    azores.transform.xScale = 3.0f;
    azores.transform.yScale = 3.0f;
    azores.sprite = azoresSprite;
    
    
    Entity azores2;
    memcpy(azores2.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores2.entityType = SPRITE_E;
    Sprite azoresSprite2 = re->registerSprite("..\\assets\\azores.png",
                                              azoresImageData,
                                              560, 144,
                                              0, 0,
                                              560, 144);
    azores2.transform.xPos = -10;
    azores2.transform.yPos = 0;
    azores2.transform.xScale = 7.0f;
    azores2.transform.yScale = 7.0f;
    azores2.sprite = azoresSprite2;
    re->addSpriteFrame(&azores2.sprite, 50, 50, 50, 50);
    re->addSpriteFrame(&azores2.sprite, 300, 0, 10, 10);
    azores2.sprite.currentFrame = 2;
    
    addEntity(&azores);
    addEntity(&azores2);
    
    float vertices[] = {
        -1, -1, 0,
        0, 1, 0,
        1, -1, 0
    };
    Mesh mesh = loadMeshFromOBJ("..\\assets\\cube.obj");
    //asteroidMesh.meshHandle = re->registerMesh(asteroidMesh.VVVNNNST, asteroidMesh.vertexCount);
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
        break;
    }
}

ControllerKeycode toControllerKeycode(Keycode keycode)
{
    switch (keycode)
    {
        case ARROW_LEFT  : return DPAD_LEFT;
        case ARROW_RIGHT : return DPAD_RIGHT;
        case ARROW_UP    : return DPAD_UP;
        case ARROW_DOWN  : return DPAD_DOWN;
        default          : return NONE;
    }
}

bool keyPressed(InputDevice* device, Keycode keycode)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
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
            ControllerKeycode controllerKeycode = toControllerKeycode(keycode);
            if (controllerKeycode == NONE) return false;
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
        
        case CONTROLLER:
        {
            ControllerKeycode controllerKeycode = toControllerKeycode(keycode);
            if (controllerKeycode == NONE) return false;
            Controller* controller = device->controller;
            if (controller->keycodes[controllerKeycode])
                return true;
            return false;
        }
        break;
        
        default: return false;
    }
}

bool keyUp(InputDevice * device, Keycode keycode)
{
    switch (device->deviceType)
    {
        case KEYBOARD:
        {
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
            ControllerKeycode controllerKeycode = toControllerKeycode(keycode);
            if (controllerKeycode == NONE) return false;
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

float p = 0.0f;
float velocity = 0.003f;
void game_update_and_render(float dt, InputDevice* inputDevice, refexport_t* re)
{
    static float posX = 0.0f;
    static float scaleY = 1.0f;
    
    Entity * spriteEntity = gSpriteEntityList;
    for (int i = 0;
         i < gNumSpriteEntities;
         ++i)
    {
        //spriteEntity->transform.xPos = 10.0f*sin(posX); //dt/1000.0f * velocity + spriteEntity->transform.xPos;
        
        if (keyPressed(inputDevice, ARROW_UP))
        {
            printf("DPAD UP pressed\n");
            spriteEntity->transform.yPos += 0.07f * dt/1000;
        }
        
        if (keyPressed(inputDevice, ARROW_DOWN))
        {
            printf("DPAD DOWN pressed\n");
            spriteEntity->transform.yPos -= 0.07f * dt/1000;
        }
        
        if (keyDown(inputDevice, ARROW_LEFT))
        {
            printf("DPAD LEFT pressed\n");
            spriteEntity->transform.xPos -= 0.07f * dt/1000;
        }
        
        if (keyUp(inputDevice, ARROW_RIGHT))
        {
            printf("DPAD RIGHT pressed\n");
            spriteEntity->transform.xPos += 0.07f * dt/1000;
        }
        
        if (keyDown(inputDevice, LETTER_A))
        {
            printf("A pressed\n");
            spriteEntity->transform.xScale += 0.02f * dt/1000;
        }
        
        spriteEntity++;
    }
    posX += dt * 0.00001f;
    scaleY += dt * 0.000001f;
    
    gRefdef.numSpriteEntities = gNumSpriteEntities;
    gRefdef.spriteEntities = gSpriteEntityList;
    // gRefdef.numMeshEntities = gNumMeshEntities;
    gRefdef.meshEntities = gMeshEntityList;
    re->renderFrame(&gRefdef);
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