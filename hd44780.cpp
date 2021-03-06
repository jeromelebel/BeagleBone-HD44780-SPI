#include "hd44780.h"
#include "font.h"
#include <assert.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SPI_MODE              SPI_MODE_3    /// SPI Mode
#define SPI_BITS_PER_WORD     10        /// we're sending bytewise
#define SPI_MAX_SPEED         10000        /// maximum speed is 1 Mhz
#define MAX_COLUMN            20

#define ENGLISH_JAPANESE_FONT   0b00
#define WESTERN_EUROPEAN_1_FONT 0b01
#define ENGLISH_RUSSIAN_FONT    0b10
#define WESTERN_EUROPEAN_2_FONT 0b11

#define CLEAR_COMMAND           0b1
#define HOME_COMMAND            0b10
#define CURSOR_MODE_COMMAND(inc_dec, shift) (0b100 | ((0b1 & inc_dec) << 1) | (0b1 & shift))
#define SCREEN_CURSOR_COMMAND(screen_on_off, cursor_on_off, blinking_on_off) (0b1000 | ((0b1 & screen_on_off) << 2) | ((0b1 & cursor_on_off) << 1) | (0b1 & blinking_on_off))
#define FONT_COMMAND(font)      (0b111000 | (0b11 & font))
#define MOVE_TO_DDRAM_COMMAND(address) (0b10000000 | (0b1111111 & address))
#define MOVE_TO_CGRAM_COMMAND(address) (0b1000000 | (0b111111 & address))

HD44780::HD44780(void)
{
    _font = &font1;
}

HD44780::HD44780(Font *font)
{
    _font = font;
    assert(_font->getWidth() == CHARACTERE_WIDTH);
}

HD44780::~HD44780()
{
}

int HD44780::openDevice(const char *bus)
{
    int mode  = SPI_MODE;
    int bpw   = SPI_BITS_PER_WORD;
    int speed = SPI_MAX_SPEED;

    if((_spi_fd = open(bus, O_RDWR)) < 0){
        printf("error opening %s\n", bus);
        return -1;
    }

    if (ioctl(_spi_fd, SPI_IOC_WR_MODE, &mode) < 0) 
        return -1;
    if (ioctl(_spi_fd, SPI_IOC_RD_MODE, &mode) < 0) 
        return -1;

    if (ioctl(_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bpw) < 0) 
        return -1;
    if (ioctl(_spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bpw) < 0) 
        return -1;

    if (ioctl(_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)   < 0) 
        return -1;
    if (ioctl(_spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)   < 0) 
        return -1;

    return 0;
}

void HD44780::closeDevice(void)
{
    ::close(_spi_fd);
}

int HD44780::initDevice(int line, int row)
{
    unsigned short initialization[] = {  FONT_COMMAND(ENGLISH_JAPANESE_FONT), SCREEN_CURSOR_COMMAND(0, 0, 0), CLEAR_COMMAND, CURSOR_MODE_COMMAND(1, 0), HOME_COMMAND, SCREEN_CURSOR_COMMAND(1, 0, 0) };
    this->sendCommand(initialization, sizeof(initialization));
    _line = line;
    _row = row;
    return 0;
}

int HD44780::sendCommand(unsigned short *commands, unsigned int length, unsigned int delay)
{
    int ret = 0;
    struct spi_ioc_transfer spi;
 
    spi.tx_buf        = (unsigned long)commands;
    spi.rx_buf        = (unsigned long)commands;
    spi.len           = length;
    spi.delay_usecs   = delay;
    spi.speed_hz      = SPI_MAX_SPEED;
    spi.bits_per_word = SPI_BITS_PER_WORD;
 
/*    printf("command size %d %d\n", length, delay);
    for (unsigned int ii = 0; ii < length; ii += 2) {
        printf("  command %d %d '%c'\n", ((unsigned char *)commands)[ii], ((unsigned char *)commands)[ii + 1], (((unsigned char *)commands)[ii] >= 32) ? ((unsigned char *)commands)[ii] : '-');
    }*/
    if(ret = ioctl (_spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0){
        printf("ERROR while sending\n");
        return -1;
    }
    return 0;
}

void HD44780::setGlyphPosition(int row)
{
    unsigned char position;
    unsigned short command;
    
    // I don't know how to change the line
    assert(row >= _row);
    _x = row;
    position = _x * 5;
    command = 0x80 | position;
    this->sendCommand(&command, 2);
}

void HD44780::moveToLine(unsigned int line)
{
    unsigned short command;
    
    if (line == 0) {
        command = HOME_COMMAND;
    } else {
        command = MOVE_TO_DDRAM_COMMAND(0xC0);
    }
    this->sendCommand(&command, sizeof(command));
}

void HD44780::showCursor(unsigned int show, unsigned int blinking)
{
    unsigned short command;
    
    command = SCREEN_CURSOR_COMMAND(1, show, blinking);
    this->sendCommand(&command, sizeof(command));
}

void HD44780::print(const char *string)
{
    this->print((const unsigned char *)string);
}

void HD44780::print(const unsigned char *string)
{
    while (string[0]) {
        //this->printGlyphCharacter(string[0]);
        this->printCharacter(string[0]);
        string++;
    }
}

void HD44780::printCharacter(unsigned char character)
{
    unsigned short command;
    
    command = 0x200 | character;
    this->sendCommand(&command, sizeof(command));
}

void HD44780::printGlyphCharacter(unsigned char character)
{
    unsigned char ii;
    unsigned char glyph[CHARACTERE_WIDTH];
    
    ii = 0;
    while (ii < CHARACTERE_WIDTH) {
        unsigned short command;
        
        command = 0x200 | _font->pixelsForChar(character, ii);
        //this->sendCommand(&command, sizeof(command));
        glyph[ii] = _font->pixelsForChar(character, ii);
        ii++;
    }
    this->printGlyph(glyph);
}

void HD44780::printGlyph(unsigned char glyph[CHARACTERE_WIDTH])
{
    unsigned short command[CHARACTERE_WIDTH];
    unsigned char ii = 0;
    
    while (ii < CHARACTERE_WIDTH) {
        command[ii] = 0x200 | glyph[ii];
        this->sendCommand(&command[ii], 2);
        ii++;
    }
    _x++;
    if (_x >= _row) {
        while (_x < MAX_COLUMN) {
            ii = 0;
            while (ii < CHARACTERE_WIDTH) {
                command[ii] = 0x200 | 0;
                this->sendCommand(&command[ii], 2);
                ii++;
            }
            _x++;
        }
        _x = 0;
        _y++;
        if (_y >= _line) {
            _y = 0;
        }
    }
}
