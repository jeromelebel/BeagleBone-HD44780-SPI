class Font;

#define CHARACTERE_WIDTH 5

class HD44780
{
    int _spi_fd;
    Font *_font;
    
    int _line, _row;
    int _x, _y;
    
    int sendCommand(unsigned short *commands, unsigned int length, unsigned int delay = 0);
    
    public:
        HD44780(void);
        HD44780(Font *font);
        ~HD44780();
        
        int openDevice(const char *bus);
        int initDevice(int line, int row);
        void closeDevice(void);
        
        void showCursor(unsigned int show, unsigned int blinking);
        void setGlyphPosition(int row);
        void print(const unsigned char *string);
        void print(const char *string);
        void printCharacter(unsigned char character);
        void printGlyphCharacter(unsigned char character);
        void printGlyph(unsigned char glyph[CHARACTERE_WIDTH]);
        void moveToLine(unsigned int line);
};
