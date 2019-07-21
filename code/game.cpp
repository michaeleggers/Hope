#include "game.h"
#include "common_os.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

global_var int gNumRooms;
global_var Entity gSpriteEntityList[MAX_SPRITES];
global_var Entity gMeshEntityList[MAX_MESHES];
global_var Entity gPlayerEntity;
global_var int gNumSpriteEntities;
global_var int gNumMeshEntities;
global_var Sprite gBitmapFontSprite;
global_var Refdef gRefdef;
global_var PlatformAPI* gPlatformAPI;
global_var DrawList gDrawList;
global_var SpriteSheet gFontSpriteSheet;
global_var SpriteSheet gTilesSpriteSheet;
global_var SpriteSheet gTTFSpriteSheet;
global_var Texture *gTTFTexture;
global_var int gIsoMap[10000];

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

Window createSpriteWindow(Texture *texture,
                          int xOffset, int yOffset,
                          int width, int height)
{
    uint32_t textureWidth  = texture->width;
    uint32_t textureHeight = texture->height;
    Window window;
    window.width  = (1.0f / (float)textureWidth) * (float)width;
    window.height = (1.0f / (float)textureHeight) * (float)height;
    if (xOffset > 0)
        window.x = (1.0f / (float)textureWidth) * (float) (xOffset % textureWidth);
    else
        window.x = 0;
    if (yOffset > 0)
        window.y = (1.0f / (float)textureHeight) * (float) ((width / textureWidth) * height + yOffset);
    else
        window.y = 0;
    window.intWidth = width;
    window.intHeight = height;
    
    return window;
}

SpriteSheet createSpriteSheet(refexport_t* re,
                              char * file,
                              int xOffset, int yOffset,
                              int windowWidth, int windowHeight)
{
    int x, y, n;
    unsigned char * bitmapData = 0;
    if (fileExists(file))
        bitmapData = stbi_load(file, &x, &y, &n, 4);
    // else
    // TODO(Michael): what to do on failure???
    
    SpriteSheet spriteSheet;
    Texture *texture = re->createTexture(file, bitmapData, x, y);
    spriteSheet.texture = texture;
    spriteSheet.currentFrame = 0;
    spriteSheet.frameCount = 0;
    spriteSheet.freeWindowIndex = 0;
    spriteSheet.width = x;
    spriteSheet.height = y;
    strcpy(spriteSheet.name, file);
    
    Window window = createSpriteWindow(texture, 0, 0, x, y);
    spriteSheet.windows[0] = window;
    // no incrementing of freeWindowIndex, because addSpriteFrame will do that.
    return spriteSheet;
}

void addSpriteFrame(SpriteSheet *spriteSheet,
                    int xOffset, int yOffset,
                    int width, int height)
{
    uint32_t textureWidth = spriteSheet->texture->width;
    uint32_t textureHeight = spriteSheet->texture->height;
    int windowIndex = spriteSheet->freeWindowIndex;
    spriteSheet->windows[windowIndex] = createSpriteWindow(spriteSheet->texture,
                                                           xOffset, yOffset,
                                                           width, height);
    spriteSheet->freeWindowIndex++;
    spriteSheet->frameCount++;
}

void pushText(char *text, 
              float xPos, float yPos, 
              float xScale, float yScale,
              v3 tint,
              SpriteSheet *spriteSheet)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd 
        && (prevRenderCmd->type == RENDER_CMD_TEXT)
        && (tint.x == prevRenderCmd->tint.x)
        && (tint.y == prevRenderCmd->tint.y)
        && (tint.z == prevRenderCmd->tint.z))
    {
        renderCmdPtr = prevRenderCmd;
    }
    else
    {
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->type = RENDER_CMD_TEXT;
        renderCmdPtr->tint = tint;
        renderCmdPtr->textureID = spriteSheet->texture->texture_id;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->quadCount = 0;
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    char * c = text;
    for (int i = 0;
         *c != '\0';
         i++)
    {
        int asciiValue = *c;
		if (asciiValue >= 97 && asciiValue <= 122) // lower case letters
        {
            asciiValue -= 32;
        }
        int frame = asciiValue - ' '; // glyph texture starts with <SPACE> (dec=32)
        
        Window window = spriteSheet->windows[frame]; // TODO(Michael): frame value legal?
        float xUVoffset = (1.0f/(float)944) * (float)((frame*16)%944);
        
        // for each character we need to:
        // - create four vertices
        // - create four UVs (windows into texture)
        vertex[0].position.x = xPos + i*xScale;
        vertex[0].position.y = yPos;
        vertex[0].position.z = 0.f;
        vertex[0].UVs.x = window.x;
        vertex[0].UVs.y = window.height;
        vertex[1].position.x = xPos + i*xScale + xScale;
        vertex[1].position.y = yPos;
        vertex[1].position.z = 0.f;
        vertex[1].UVs.x = window.x + window.width;
        vertex[1].UVs.y = window.height;
        vertex[2].position.x = xPos + i*xScale + xScale;
        vertex[2].position.y = yPos + yScale;
        vertex[2].position.z = 0.f;
        vertex[2].UVs.x = window.x + window.width;
        vertex[2].UVs.y = window.y;
        vertex[3].position.x = xPos + i*xScale;
        vertex[3].position.y = yPos + yScale;
        vertex[3].position.z = 0.f;
        vertex[3].UVs.x = window.x;
        vertex[3].UVs.y = window.y;
        index[0] = 0+gDrawList.quadCount*4; index[1] = 1+gDrawList.quadCount*4; index[2] = 2+gDrawList.quadCount*4; // first triangle
        index[3] = 2+gDrawList.quadCount*4; index[4] = 3+gDrawList.quadCount*4; index[5] = 0+gDrawList.quadCount*4; // second triangle
        vertex += 4;
        index  += 6;
        gDrawList.vtxCount += 4;
        gDrawList.idxCount += 6;
        gDrawList.quadCount++;
        renderCmdPtr->quadCount++;
        
        c++;
    }
}

void pushTexturedRect(float xPos, float yPos, 
                      float xScale, float yScale,
                      v3 tint,
                      SpriteSheet *spriteSheet, int frame)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_TEXTURED_RECT))
    {
        renderCmdPtr = prevRenderCmd;
    }
    else
    {
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->type = RENDER_CMD_TEXTURED_RECT;
        renderCmdPtr->tint = tint;
        renderCmdPtr->textureID = spriteSheet->texture->texture_id;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->quadCount = 0;
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    Window window = spriteSheet->windows[frame]; // TODO(Michael): frame value legal?
    float aspectRatio = (float)window.intWidth / (float)window.intHeight;
    vertex[0].position.x = xPos;
    vertex[0].position.y = yPos;
    vertex[0].position.z = 0.f;
    vertex[0].UVs.x = window.x;
    vertex[0].UVs.y = window.height;
    vertex[1].position.x = xPos + xScale*aspectRatio;
    vertex[1].position.y = yPos;
    vertex[1].position.z = 0.f;
    vertex[1].UVs.x = window.x + window.width;
    vertex[1].UVs.y = window.height;
    vertex[2].position.x = xPos + xScale*aspectRatio;
    vertex[2].position.y = yPos + yScale;
    vertex[2].position.z = 0.f;
    vertex[2].UVs.x = window.x + window.width;
    vertex[2].UVs.y = window.y;
    vertex[3].position.x = xPos;
    vertex[3].position.y = yPos + yScale;
    vertex[3].position.z = 0.f;
    vertex[3].UVs.x = window.x;
    vertex[3].UVs.y = window.y;
    index[0] = 0+gDrawList.quadCount*4; index[1] = 1+gDrawList.quadCount*4; index[2] = 2+gDrawList.quadCount*4; // first triangle
    index[3] = 2+gDrawList.quadCount*4; index[4] = 3+gDrawList.quadCount*4; index[5] = 0+gDrawList.quadCount*4; // second triangle
    vertex += 4;
    index  += 6;
    gDrawList.vtxCount += 4;
    gDrawList.idxCount += 6;
    gDrawList.quadCount++;
    renderCmdPtr->quadCount++;
}

void pushTexturedRect(float xPos, float yPos, 
                      float xScale, float yScale,
                      v3 tint,
                      Texture * texture)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_TTF))
    {
        renderCmdPtr = prevRenderCmd;
    }
    else
    {
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->type = RENDER_CMD_TTF;
        renderCmdPtr->tint = tint;
        renderCmdPtr->textureID = texture->texture_id;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->quadCount = 0;
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    vertex[0].position.x = xPos;
    vertex[0].position.y = yPos;
    vertex[0].position.z = 0.f;
    vertex[0].UVs.x = 0.0f;
    vertex[0].UVs.y = 0.0f;
    vertex[1].position.x = xPos + xScale;
    vertex[1].position.y = yPos;
    vertex[1].position.z = 0.f;
    vertex[1].UVs.x = 1.0f;
    vertex[1].UVs.y = 0.0f;
    vertex[2].position.x = xPos + xScale;
    vertex[2].position.y = yPos + yScale;
    vertex[2].position.z = 0.f;
    vertex[2].UVs.x = 1.0f;
    vertex[2].UVs.y = 1.0f;
    vertex[3].position.x = xPos;
    vertex[3].position.y = yPos + yScale;
    vertex[3].position.z = 0.f;
    vertex[3].UVs.x = 0.0f;
    vertex[3].UVs.y = 1.0f;
    index[0] = 0+gDrawList.quadCount*4; index[1] = 1+gDrawList.quadCount*4; index[2] = 2+gDrawList.quadCount*4; // first triangle
    index[3] = 2+gDrawList.quadCount*4; index[4] = 3+gDrawList.quadCount*4; index[5] = 0+gDrawList.quadCount*4; // second triangle
    vertex += 4;
    index  += 6;
    gDrawList.vtxCount += 4;
    gDrawList.idxCount += 6;
    gDrawList.quadCount++;
    renderCmdPtr->quadCount++;
}

void pushLine2D(float x1, float y1, float x2, float y2, v3 tint, float thickness)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_LINE) &&
        (prevRenderCmd->tint.x == tint.x) &&
        (prevRenderCmd->tint.y == tint.y) &&
        (prevRenderCmd->tint.z == tint.z) &&
        (prevRenderCmd->thickness == thickness) )
    {
        renderCmdPtr = prevRenderCmd;
    }
    else
    {
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->type = RENDER_CMD_LINE;
        renderCmdPtr->tint = tint;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->lineCount = 0;
        renderCmdPtr->thickness = thickness;
        gDrawList.lineCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    vertex[0].position.x = x1;
    vertex[0].position.y = y1;
    vertex[0].position.z = 0.0f;
    vertex[0].UVs.x = 0.f;
    vertex[0].UVs.y = 0.f;
    vertex[1].position.x = x2;
    vertex[1].position.y = y2;
    vertex[1].position.z = 0.0f;
    vertex[1].UVs.x = 0.f;
    vertex[1].UVs.y = 0.f;
    index[0] = 0+gDrawList.lineCount*2; index[1] = 1+gDrawList.lineCount*2;
    vertex += 2;
    index += 2;
    gDrawList.vtxCount += 2;
    gDrawList.idxCount += 2;
    gDrawList.lineCount++;
    renderCmdPtr->lineCount++;
}

void pushRect2D(float left, float top, float right, float bottom, v3 tint, float thickness)
{
    // top horizontal line
    pushLine2D(left, top, right, top, tint, thickness);
    // bottom horizontal line
    pushLine2D(left, bottom, right, bottom, tint, thickness);
    // left vertical line
    pushLine2D(left, top, left, bottom, tint, thickness);
    // right vertical line
    pushLine2D(right, top, right, bottom, tint, thickness);
}

void pushFilledRect(float left, float top, float width, float height, v3 tint)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_FILLED_RECT) &&
        (prevRenderCmd->tint.x == tint.x) && 
        (prevRenderCmd->tint.y == tint.y) &&
        (prevRenderCmd->tint.z == tint.z) )
    {
        renderCmdPtr = prevRenderCmd;
    }
    else
    {
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->type = RENDER_CMD_FILLED_RECT;
        renderCmdPtr->tint = tint;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->quadCount = 0;
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    vertex[0].position.x = left;
    vertex[0].position.y = top - height;
    vertex[0].position.z = 0.f;
    vertex[1].position.x = left + width;
    vertex[1].position.y = top - height;
    vertex[1].position.z = 0.f;
    vertex[2].position.x = left + width;
    vertex[2].position.y = top;
    vertex[2].position.z = 0.f;
    vertex[3].position.x = left;
    vertex[3].position.y = top;
    vertex[3].position.z = 0.f;
    index[0] = 0+gDrawList.quadCount*4; index[1] = 1+gDrawList.quadCount*4; index[2] = 2+gDrawList.quadCount*4; // first triangle
    index[3] = 2+gDrawList.quadCount*4; index[4] = 3+gDrawList.quadCount*4; index[5] = 0+gDrawList.quadCount*4; // second triangle
    vertex += 4;
    index  += 6;
    gDrawList.vtxCount += 4;
    gDrawList.idxCount += 6;
    gDrawList.quadCount++;
    renderCmdPtr->quadCount++;
}

void game_init(PlatformAPI* platform_api, refexport_t* re)
{
    gPlatformAPI = platform_api;
    
    // load TTF Font
    char* ttf_font = gPlatformAPI->readTextFile("c:/windows/fonts/arialbd.ttf");
    stbtt_fontinfo font;
    unsigned char *ttfBitmap = 0;
    stbtt_InitFont(&font, (uint8_t*)ttf_font, stbtt_GetFontOffsetForIndex((uint8_t*)ttf_font, 0));
    int w, h;
    ttfBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 100.0f), 65, &w, &h, 0, 0);
    unsigned char ttfTexture[512*512] = {};
    for (int i = 0; i<h; ++i)
    {
        for (int k = 0; k<w; ++k)
            ttfTexture[i*512+k] = ttfBitmap[(h-i)*w+k];
    }
    gTTFTexture = re->createTextureFromBitmap(ttfTexture, 512, 512);
    
#if 0
    for (int i = 0; i<h; ++i)
    {
        for (int k = 0; k<w; ++k)
            ttfBitmap[i*w+k] > 0 ? printf(" # ") : printf(" _ ");
        printf("\n");
    }
#endif
    
    
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
    
    pushText("rendering 10.000 tiles!", -5, 5, 1, 1, {0.8f, 0.1f, 0.1f}, &gFontSpriteSheet);
    pushLine2D(0.f, 1.f, 10.f, 1.f, {1,0,0},3);
    pushLine2D(0.f, 5.f, 10.f, 5.f, {0,1,0},5);
    pushLine2D(-10.f, -10.f, 0.f, 0.f, {0,0,1},3);
    pushLine2D(-10.f, -10.f, 10.f, 0.f, {0,0,1},3);
    pushText("rendering 10.000 tiles!", -5, 5, 1, 1, {0.8f, 0.1f, 0.1f}, &gFontSpriteSheet);
    pushLine2D(0.f, 0.f, 10.f, -10.f, {1,1,0},7);
    pushRect2D(0.0f, 0.0f, 7.0f, -7.0f, {1,0,0}, 2.5f);
    //pushTexturedRect(x, y, scale, &bitmapData);
    pushFilledRect(-5.0f, 0.0f, 3.0f, 5.0f, {1,1,0});
    pushFilledRect(-10.0f, 0.0f, 1.0f, 4.0f, {1,1,0});
    pushFilledRect(0.0f, 0.0f, 10.0f, 2.0f, {1,0,1});
    pushTexturedRect(-18, 0,1, 1,{1, 1, 1},&gTilesSpriteSheet, 0);
    pushTexturedRect(-18, 0, 5, 5,{0, 1, 1}, gTTFTexture);
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