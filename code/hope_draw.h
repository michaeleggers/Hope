#ifndef HOPE_DRAW_H
#define HOPE_DRAW_H

Window createSpriteWindow(Texture *texture,
                          int xOffset, int yOffset,
                          int width, int height);
SpriteSheet createSpriteSheet(refexport_t* re,
                              char * file,
                              int xOffset, int yOffset,
                              int windowWidth, int windowHeight);
void addSpriteFrame(SpriteSheet *spriteSheet,
                    int xOffset, int yOffset,
                    int width, int height);
void pushText(char *text, 
              float xPos, float yPos, 
              float xScale, float yScale,
              v3 tint,
              SpriteSheet *spriteSheet);
void pushTexturedRect(float xPos, float yPos, 
                      float xScale, float yScale,
                      v3 tint,
                      SpriteSheet *spriteSheet, int frame);
void pushTexturedRect(float xPos, float yPos, 
                      float xScale, float yScale,
                      v3 tint,
                      Quad quad,
                      Texture * texture);
void pushTTFText(char * text, float xPos, float yPos, v3 tint, FontInfo * fontInfo);
void pushLine2D(float x1, float y1, float x2, float y2, v3 tint, float thickness);
void pushRect2D(float left, float top, float right, float bottom, v3 tint, float thickness);
void pushFilledRect(float left, float top, float width, float height, v3 tint);


#endif

