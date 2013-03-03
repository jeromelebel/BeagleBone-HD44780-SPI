#include "graphic.h"

#ifndef FONT_H
#define FONT_H

class Font
{
private:
    unsigned char _width;
    unsigned char _height;
    const unsigned char *_buffer;
    unsigned int _charCount;

public:
    Font(PixelCoordonate width, PixelCoordonate height, const unsigned char *buffer, int bufferSize);
    
    unsigned char getWidth(void) { return _width; };
    unsigned char getHeight(void) { return _height; };
    unsigned char pixelsForChar(unsigned char character, unsigned char row) { if (character >= _charCount) { character = 0; } return _buffer[(character * _width) + row]; };
};

extern Font font1;
extern Font font2;

#endif /* FONT_H */
