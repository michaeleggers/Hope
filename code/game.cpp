#include "game.h"
#include "common_os.h"
#define PI 3.141592f

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global_var int gNumRooms;
global_var Entity gSpriteEntityList[MAX_SPRITES];
global_var Entity gMeshEntityList[MAX_MESHES];
global_var Entity gPlayerEntity;
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
    
    //addEntity(&azores);
    //addEntity(&azores2);
    
    /*
    float vertices[] = {
        -1, -1, 0,
        0, 1, 0,
        1, -1, 0
    };
    asteroidMesh.meshHandle = re->registerMesh(asteroidMesh.VVVNNNST, asteroidMesh.vertexCount);
    */
    
    Mesh cubeMesh = loadMeshFromOBJ("..\\assets\\cube.obj");
    cubeMesh.meshHandle = re->registerMesh(cubeMesh.VVVNNNST, cubeMesh.vertexCount);
    
    // init asteroids
    for (int i = 0; i < 50; ++i)
    {
        Entity cubeEntity;
        memcpy(cubeEntity.transform.modelMat, gModelMatrix, 16*sizeof(float));
        cubeEntity.mesh = cubeMesh;
        cubeEntity.entityType = MESH_E;
        cubeEntity.transform.xPos = randBetween(-10.f, 10.f);
        cubeEntity.transform.yPos = randBetween(-10.f, 10.f);
        float randScale = randBetween(.2f, 1.f);
        cubeEntity.transform.xScale = randScale;
        cubeEntity.transform.yScale = randScale;
        cubeEntity.velocity = { randBetween(.003f, .007f), randBetween(.003f, .007f), 0.f };
        //addEntity(&cubeEntity);
    }
    
    // init player entity
    Mesh cubeMeshMaya = loadMeshFromOBJ("..\\assets\\cube.obj");
    cubeMeshMaya.meshHandle = re->registerMesh(cubeMeshMaya.VVVNNNST, cubeMeshMaya.vertexCount);
    
    Entity playerEntity;
    memcpy(playerEntity.transform.modelMat, gModelMatrix, 16*sizeof(float));
    playerEntity.mesh = cubeMeshMaya;
    playerEntity.entityType = PLAYER_E;
    playerEntity.transform.xPos = 0;
    playerEntity.transform.yPos = 0;
    playerEntity.transform.xScale = 1;
    playerEntity.transform.yScale = 1;
    playerEntity.transform.angle = 0;
    playerEntity.speed = { 0.001f, 0.001f };
    playerEntity.velocity = {0.f, -1.f, 0.f};
    addEntity(&playerEntity);
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
    
    // control player
    if (keyDown(inputDevice, ARROW_UP))
    {
        printf("DPAD UP pressed\n");
        v3 direction = {0, 1, 0}; 
        gPlayerEntity.speed.x += 0.00001f; 
        gPlayerEntity.speed.y += 0.00001f;
        float angleInRad = (PI*gPlayerEntity.transform.angle)/180.0f;
        gPlayerEntity.velocity.x = direction.x*cos(angleInRad) - direction.y*sin(angleInRad);
        gPlayerEntity.velocity.y = direction.x*sin(angleInRad) + direction.y*cos(angleInRad);
    }
    
    if (keyDown(inputDevice, ARROW_DOWN))
    {
        printf("DPAD DOWN pressed\n");
        v3 direction = {0, 1, 0};
        gPlayerEntity.speed.x -= 0.00001f;
        gPlayerEntity.speed.y -= 0.00001f;
        float angleInRad = (PI*gPlayerEntity.transform.angle)/180.0f;
        gPlayerEntity.velocity.x = direction.x*cos(angleInRad) - direction.y*sin(angleInRad);
        gPlayerEntity.velocity.y = direction.x*sin(angleInRad) + direction.y*cos(angleInRad);
    }
    
    if (keyDown(inputDevice, ARROW_LEFT))
    {
        printf("DPAD LEFT pressed\n");
        gPlayerEntity.transform.angle += .2f;
    }
    
    if (keyDown(inputDevice, ARROW_RIGHT))
    {
        printf("DPAD RIGHT pressed\n");
        gPlayerEntity.transform.angle -= .2f;
    }
    
    if (keyDown(inputDevice, LETTER_A))
    {
        printf("A pressed\n");
        //gPlayerEntity.transform.xScale += 0.02f * dt/1000;
        gPlayerEntity.speed.x += 0.00001f;
        gPlayerEntity.speed.y += 0.00001f;
    }
    gPlayerEntity.velocity = normalize(gPlayerEntity.velocity);
    gPlayerEntity.transform.xPos += gPlayerEntity.velocity.x * gPlayerEntity.speed.x * dt/1000;
    gPlayerEntity.transform.yPos += gPlayerEntity.velocity.y * gPlayerEntity.speed.x * dt/1000;
    
    // simulate asteroids
    Entity* meshEntity = gMeshEntityList;
    for (int i = 0; i < gNumMeshEntities; ++i)
    {
        if (meshEntity->transform.xPos > 10.f)  {
            meshEntity->transform.xPos = 10;
            meshEntity->velocity.x *= -1;
        }
        if (meshEntity->transform.xPos < -10.f)  {
            meshEntity->transform.xPos = -10;
            meshEntity->velocity.x *= -1;
        }
        if (meshEntity->transform.yPos > 10.f)  {
            meshEntity->transform.yPos = 10;
            meshEntity->velocity.y *= -1;
        }
        if (meshEntity->transform.yPos < -10.f)  {
            meshEntity->transform.yPos = -10;
            meshEntity->velocity.y *= -1;
        }
        
        meshEntity->transform.xPos += meshEntity->velocity.x * (dt/(float)1000);
        meshEntity->transform.yPos += meshEntity->velocity.y * (dt/(float)1000);
        meshEntity++;
    }
    
    gRefdef.numSpriteEntities = gNumSpriteEntities;
    gRefdef.spriteEntities = gSpriteEntityList;
    gRefdef.numMeshEntities = gNumMeshEntities;
    gRefdef.meshEntities = gMeshEntityList;
    gRefdef.playerEntity = &gPlayerEntity;
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