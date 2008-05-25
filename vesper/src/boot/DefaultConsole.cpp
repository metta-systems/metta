#include "DefaultConsole.h"

DefaultConsole::DefaultConsole()
{
   videoram = (unsigned char *) 0xb8000;
}

void DefaultConsole::putchar(char ch)
{
   videoram[0] = ch; /* character */
   videoram[1] = 0x07; /* foreground, background colors. */
}
