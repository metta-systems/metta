class DefaultConsole
{
    public:
	DefaultConsole();

	void putchar(char ch);

    private:
	unsigned char *videoram;
};
