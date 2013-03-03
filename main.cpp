#include "hd44780.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

HD44780 *display;

int main(int argc, char* argv[])
{
    int ii;
    
    display = new HD44780();
    
    if (display->openDevice("/dev/spidev2.0") != 0) {
        printf("can't open device\n");
        return 1;
    }
    if (display->initDevice(2, 16) != 0) {
        printf("can't init device\n");
        return 1;
    }
    ii = 1;
    while (argc > ii) {
        display->print(argv[ii]);
        ii++;
    }
    return 0;
}
