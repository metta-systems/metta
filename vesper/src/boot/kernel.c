#define UNUSED(x) ((void)x)

void kmain( void* mbd, unsigned int magic )
{
    UNUSED(mbd);
    UNUSED(magic);

   /* Write your kernel here. Example: */
   unsigned char *videoram = (unsigned char *) 0xb8000;
   videoram[0] = 65; /* character 'A' */
   videoram[1] = 0x07; /* foreground, background colors. */
}
