#include "hope_draw.h"
#include "platform.h"
#include "stb_image.h"
#include "stb_truetype.h"

global_var int gNumSpriteEntities;
global_var int gNumMeshEntities;
global_var Sprite gBitmapFontSprite;
global_var DrawList gDrawList;
global_var SpriteSheet gFontSpriteSheet;
global_var SpriteSheet gTilesSpriteSheet;
global_var SpriteSheet gTTFSpriteSheet;
global_var SpriteSheet gIndySpriteSheet;
global_var SpriteSheet gFatguySpriteSheet;
global_var Texture *gTTFTexture;
global_var PlatformAPI* gPlatformAPI;
global_var FontInfo gFontInfo;

#define MAX_SPRITESHEETS 32
global_var SpriteSheet gSpriteSheets[MAX_SPRITESHEETS];
global_var int spritesheet_count;

Window createSpriteWindow(Texture *texture,
                          int xOffset, int yOffset,
                          int width, int height)
{
    uint32_t textureWidth  = texture->width;
    uint32_t textureHeight = texture->height;
    Window window;
    window.width  = (1.0f / (float)textureWidth) * (float)width;
    window.height = (1.0f / (float)textureHeight) * (float)height;
    window.x = (1.0f / (float)textureWidth) * (float)xOffset;
    window.y = (1.0f / (float)textureHeight) * (float)yOffset;
    window.intWidth = width;
    window.intHeight = height;
    
    return window;
}

SpriteSheet * get_spritesheet_from_id(int spritesheet_id)
{
    if (spritesheet_id < MAX_SPRITESHEETS) {
        return &gSpriteSheets[spritesheet_id];
    }
    return 0;
}

int createSpriteSheet(refexport_t* re,
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
    
    SpriteSheet spriteSheet = {};
    Texture *texture = re->createTexture(file, bitmapData, x, y);
    spriteSheet.texture = texture;
    spriteSheet.frameCount = 0;
    spriteSheet.freeWindowIndex = 0;
    spriteSheet.width = x;
    spriteSheet.height = y;
    strcpy(spriteSheet.name, file);
    
    Window window = createSpriteWindow(texture, xOffset, yOffset, x, y);
    spriteSheet.windows[0] = window;
    // no incrementing of freeWindowIndex, because addSpriteFrame will do that.
    int current_spritesheet_count = spritesheet_count;
    gSpriteSheets[current_spritesheet_count] = spriteSheet;
    spritesheet_count++;
    return current_spritesheet_count;
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

// TODO(Michael): text passed not guaranteed to end with \0 !
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
                      SpriteSheet *spriteSheet, int frame,
                      bool flipHorizontally)
{
    RenderCommand *renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
#if 0
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_TEXTURED_RECT))
    {
        renderCmdPtr = prevRenderCmd;
    }
#endif
    //else
    //{
    renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
    renderCmdPtr->type = RENDER_CMD_TEXTURED_RECT;
    renderCmdPtr->tint = tint;
    renderCmdPtr->textureID = spriteSheet->texture->texture_id;
    renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
    renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
    renderCmdPtr->quadCount = 0;
    renderCmdPtr->alphaColor = 0x0000AB;// TODO(Michael): make configurable
    gDrawList.quadCount = 0;
    gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
    gDrawList.freeIndex++;
    //}
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    Window window = spriteSheet->windows[frame]; // TODO(Michael): frame value legal?
    int uv0 = 0;
    int uv1 = 1;
    int uv2 = 2;
    int uv3 = 3;
    if ( flipHorizontally ) {
        int tmp = uv0;
        uv0 = uv1;
        uv1 = tmp;
        tmp = uv2;
        uv2 = uv3;
        uv3 = tmp;
    }
#if 0
    if (sequence.flipVertical) {
        int tmp = uv0;
        uv0 = uv3;
        uv3 = tmp;
        tmp = uv1;
        uv1 = uv2;
        uv2 = tmp;
    }
#endif
    float aspectRatio = (float)window.intWidth / (float)window.intHeight;
    int width = window.intWidth;
    int height = window.intHeight;
    vertex[0].position.x = xPos;
    vertex[0].position.y = yPos+yScale*height;
    vertex[0].position.z = 0.f;
    vertex[uv0].UVs.x = window.x;
    vertex[uv0].UVs.y = window.y + window.height;
    vertex[1].position.x = xPos + xScale*width;
    vertex[1].position.y = yPos + yScale*height;
    vertex[1].position.z = 0.f;
    vertex[uv1].UVs.x = window.x + window.width;
    vertex[uv1].UVs.y = window.y + window.height;
    vertex[2].position.x = xPos + xScale*width;
    vertex[2].position.y = yPos;
    vertex[2].position.z = 0.f;
    vertex[uv2].UVs.x = window.x + window.width;
    vertex[uv2].UVs.y = window.y;
    vertex[3].position.x = xPos;
    vertex[3].position.y = yPos;
    vertex[3].position.z = 0.f;
    vertex[uv3].UVs.x = window.x;
    vertex[uv3].UVs.y = window.y;
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
                      Quad quad,
                      Texture * texture)
{
    RenderCommand *renderCmdPtr = 0;
#if 0
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd && (prevRenderCmd->type == RENDER_CMD_TTF))
    {
        renderCmdPtr = prevRenderCmd;
    }
#endif
    //else
    //{
    renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
    renderCmdPtr->type = RENDER_CMD_TEXTURED_RECT;
    renderCmdPtr->tint = tint;
    renderCmdPtr->textureID = texture->texture_id;
    renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
    renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
    renderCmdPtr->quadCount = 0;
    gDrawList.quadCount = 0;
    gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
    gDrawList.freeIndex++;
    //}
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    vertex[0].position.x = quad.x0;
    vertex[0].position.y = quad.y0;
    vertex[0].position.z = 0.f;
    vertex[0].UVs.x = quad.s0;
    vertex[0].UVs.y = quad.t0;
    vertex[1].position.x = quad.x0 + xScale;
    vertex[1].position.y = quad.y1;
    vertex[1].position.z = 0.f;
    vertex[1].UVs.x = quad.s0;
    vertex[1].UVs.y = quad.t1;
    vertex[2].position.x = quad.x1 + xScale;
    vertex[2].position.y = quad.y1 + yScale;
    vertex[2].position.z = 0.f;
    vertex[2].UVs.x = quad.s1;
    vertex[2].UVs.y = quad.t1;
    vertex[3].position.x = quad.x1;
    vertex[3].position.y = quad.y0 + yScale;
    vertex[3].position.z = 0.f;
    vertex[3].UVs.x = quad.s1;
    vertex[3].UVs.y = quad.t0;
    index[0] = 0+gDrawList.quadCount*4; index[1] = 1+gDrawList.quadCount*4; index[2] = 2+gDrawList.quadCount*4; // first triangle
    index[3] = 2+gDrawList.quadCount*4; index[4] = 3+gDrawList.quadCount*4; index[5] = 0+gDrawList.quadCount*4; // second triangle
    vertex += 4;
    index  += 6;
    gDrawList.vtxCount += 4;
    gDrawList.idxCount += 6;
    gDrawList.quadCount++;
    renderCmdPtr->quadCount++;
}

void pushTTFText(char * text, float xPos, float yPos, v3 tint, FontInfo * fontInfo)
{
    float xOffset = 0, yOffset = 0;
    float lineBreakOffset = 0.f;
    char * c = text;
    stbtt_aligned_quad quad = {};
    while (*c != '\0')
    {
        if (*c == '\n') 
        { 
            lineBreakOffset += fontInfo->fontSize;
            xOffset = 0.f;
            yOffset = 0.f;
            c++; 
            continue; 
        }
        
        int characterCode = *c;
        stbtt_GetPackedQuad(fontInfo->chardata, 
                            fontInfo->texture->width, fontInfo->texture->height,  // same data as above
                            characterCode - fontInfo->firstChar,             // character to display
                            &xOffset, &yOffset,
                            // pointers to current position in screen pixel space
                            &quad,      // output: quad to draw
                            1);
        
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
            renderCmdPtr->textureID = fontInfo->texture->texture_id;
            renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
            renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
            renderCmdPtr->quadCount = 0;
            Rect windowDimensions = gPlatformAPI->getWindowDimensions();
            hope_create_ortho_matrix(
                0.0f, (float)windowDimensions.width,
                (float)windowDimensions.height, 0.0f,
                -1.0f, 1.0f,
                renderCmdPtr->projectionMatrix.c
                );
            gDrawList.quadCount = 0;
            gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
            gDrawList.freeIndex++;
        }
        
        // current free pos in global vertex/index buffers
        Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
        uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
        
        vertex[0].position.x =  quad.x0 + xPos;
        vertex[0].position.y =  quad.y0 + lineBreakOffset + yPos;
        vertex[0].position.z = 0.f;
        vertex[0].UVs.x = quad.s0;
        vertex[0].UVs.y = quad.t0;
        vertex[1].position.x =  quad.x0 + xPos;
        vertex[1].position.y =  quad.y1 + lineBreakOffset + yPos;
        vertex[1].position.z = 0.f;
        vertex[1].UVs.x = quad.s0;
        vertex[1].UVs.y = quad.t1;
        vertex[2].position.x =  quad.x1 + xPos;
        vertex[2].position.y =  quad.y1 + lineBreakOffset + yPos;
        vertex[2].position.z = 0.f;
        vertex[2].UVs.x = quad.s1;
        vertex[2].UVs.y = quad.t1;
        vertex[3].position.x =  quad.x1 + xPos;
        vertex[3].position.y =  quad.y0 + lineBreakOffset + yPos;
        vertex[3].position.z = 0.f;
        vertex[3].UVs.x = quad.s1;
        vertex[3].UVs.y = quad.t0;
        
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

void pushTTFTextInBoundaries(char * text, 
                             float xPos, float yPos,
                             HopeDrawRect boundary, v3 tint)
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
        renderCmdPtr->textureID = gFontInfo.texture->texture_id;
        renderCmdPtr->idxBufferOffset = gDrawList.idxCount;
        renderCmdPtr->vtxBufferOffset = gDrawList.vtxCount;
        renderCmdPtr->quadCount = 0;
        Rect windowDimensions = gPlatformAPI->getWindowDimensions();
        hope_create_ortho_matrix(
            0.0f, (float)windowDimensions.width,
            (float)windowDimensions.height, 0.0f,
            -1.0f, 1.0f,
            renderCmdPtr->projectionMatrix.c
            );
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    int textLength = 0;
    float yOffsetBoundary = (int)(boundary.height/2.0f + gFontInfo.fontSize/2.0f);
    float xOffset = 0, yOffset = 0;
    float lineBreakOffset = 0.f;
    char * c = text;
    stbtt_aligned_quad quad = {};
    while (*c != '\0')
    {
        textLength++;
        if (*c == '\n') 
        { 
            lineBreakOffset += gFontInfo.fontSize;
            xOffset = 0.f;
            yOffset = 0.f;
            c++; 
            continue; 
        }
        
        int characterCode = *c;
        stbtt_GetPackedQuad(gFontInfo.chardata, 
                            gFontInfo.texture->width, gFontInfo.texture->height,  // same data as above
                            characterCode - gFontInfo.firstChar,             // character to display
                            &xOffset, &yOffset,
                            // pointers to current position in screen pixel space
                            &quad,      // output: quad to draw
                            1);
        
        // current free pos in global vertex/index buffers
        Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
        uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
        
        // NOTE(Michael): xPos is not added yet because the text might not
        // fit into the boundary in which case the text has to be scaled
        // down along the x-axis. And that is easier when the text
        // starts at the x-origin (0.0f).
        vertex[0].position.x =  quad.x0;
        vertex[0].position.y =  quad.y0 + lineBreakOffset + yPos + yOffsetBoundary;
        vertex[0].position.z = 0.f;
        vertex[0].UVs.x = quad.s0;
        vertex[0].UVs.y = quad.t0;
        vertex[1].position.x =  quad.x0;
        vertex[1].position.y =  quad.y1 + lineBreakOffset + yPos + yOffsetBoundary;
        vertex[1].position.z = 0.f;
        vertex[1].UVs.x = quad.s0;
        vertex[1].UVs.y = quad.t1;
        vertex[2].position.x =  quad.x1;
        vertex[2].position.y =  quad.y1 + lineBreakOffset + yPos + yOffsetBoundary;
        vertex[2].position.z = 0.f;
        vertex[2].UVs.x = quad.s1;
        vertex[2].UVs.y = quad.t1;
        vertex[3].position.x =  quad.x1;
        vertex[3].position.y =  quad.y0 + lineBreakOffset + yPos + yOffsetBoundary;
        vertex[3].position.z = 0.f;
        vertex[3].UVs.x = quad.s1;
        vertex[3].UVs.y = quad.t0;
        
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
    
    float squeeze = 1.f;
    float padding = 10.f;
    // NOTE(Michael): clamping to full pixels avoids
    // resampling which will result in a fuzzy font.
    float xOffsetToCenter = (int) ((boundary.width-xOffset)/2.0f);
    if (xOffset > boundary.width)
    {
        squeeze = (boundary.width - 2*padding)/(xOffset);
        xOffsetToCenter = padding;
    }
    Vertex * vertex = gDrawList.vtxBuffer + renderCmdPtr->vtxBufferOffset;
    for (int i=0; i<textLength; ++i)
    {
        vertex[0].position.x = (vertex[0].position.x)*squeeze + xOffsetToCenter + xPos;
        vertex[1].position.x = (vertex[1].position.x)*squeeze + xOffsetToCenter + xPos;
        vertex[2].position.x = (vertex[2].position.x)*squeeze + xOffsetToCenter + xPos;
        vertex[3].position.x = (vertex[3].position.x)*squeeze + xOffsetToCenter + xPos;
        vertex += 4;
    }
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
#if 0
        Rect windowDimensions = gPlatformAPI->getWindowDimensions();
        hope_create_ortho_matrix(
            0.0f, (float)windowDimensions.width,
            (float)windowDimensions.height, 0.0f,
            -1.0f, 1.0f,
            renderCmdPtr->projectionMatrix.c
            );
#endif
        gDrawList.quadCount = 0;
        gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
        gDrawList.freeIndex++;
    }
    
    // current free pos in global vertex/index buffers
    Vertex *vertex   = gDrawList.vtxBuffer + gDrawList.vtxCount;
    uint16_t *index = gDrawList.idxBuffer  + gDrawList.idxCount;
    
    vertex[0].position.x = left;
    vertex[0].position.y = top;
    vertex[0].position.z = 0.f;
    vertex[1].position.x = left;
    vertex[1].position.y = top + height;
    vertex[1].position.z = 0.f;
    vertex[2].position.x = left + width;
    vertex[2].position.y = top + height;
    vertex[2].position.z = 0.f;
    vertex[3].position.x = left + width;
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

// returns handle index into framebuffer slot managed by rendering implementation (OpenGL)
int new_framebuffer(refexport_t * re, int width, int height)
{
    return re->createFramebuffer(width, height);
}

void set_render_target(int handle)
{
    RenderCommand *renderCmdPtr = 0;
    renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
    renderCmdPtr->type = RENDER_CMD_SET_FRAMEBUFFER;
    renderCmdPtr->framebufferHandle = handle;
    gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
    gDrawList.freeIndex++;
}

void reset_render_target()
{
    RenderCommand *renderCmdPtr = 0;
    renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
    renderCmdPtr->type = RENDER_CMD_SET_DEFAULT_FRAMEBUFFER;
    gDrawList.prevRenderCmd = &gDrawList.renderCmds[gDrawList.freeIndex];
    gDrawList.freeIndex++;
}

void set_orthographic_projection(refexport_t * re, float ortho_matrix[])
{
    re->set_ortho_matrix(ortho_matrix);
}

int get_framebuffer_width(refexport_t * re, int handle)
{
    return re->get_framebuffer_width(handle);
}

int get_framebuffer_height(refexport_t * re, int handle)
{
    return re->get_framebuffer_height(handle);
}

void draw_from_framebuffer(int fb_handle, float x, float y, float scale_x, float scale_y)
{
    RenderCommand * renderCmdPtr = 0;
    RenderCommand *prevRenderCmd = gDrawList.prevRenderCmd;
    if (prevRenderCmd->type == RENDER_CMD_DRAW_FROM_FRAMEBUFFER &&
        prevRenderCmd->framebufferHandle == fb_handle) {
        renderCmdPtr = prevRenderCmd;
    }
    else { // flush current render commands and create new render command
        
        renderCmdPtr = &gDrawList.renderCmds[gDrawList.freeIndex];
        renderCmdPtr->pos_x = x;
        renderCmdPtr->pos_y = y;
        renderCmdPtr->scale_x = scale_x;
        renderCmdPtr->scale_y = scale_y;
    }
}