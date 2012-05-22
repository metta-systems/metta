/**
This file implements very basic ASCII subset for the following functions:

int isalnum(int c);
int isalpha(int c);
int isblank(int c);  // C99
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);
*/

inline int isblank(int c)
{
	return 0;
}

inline int iscntrl(int c)
{
	return 0;
}

inline int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

inline int isgraph(int c)
{
	return 0;
}

inline int islower(int c)
{
	return c >= 'a' && c <= 'z';
}

inline int ispunct(int c)
{
	return 0;
}

inline int isspace(int c)
{
	return c == ' ';
}

inline int isupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

inline int isxdigit(int c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline int tolower(int c)
{
	return isupper(c) ? (c + 'a' - 'A') : c;
}

inline int toupper(int c)
{
	return islower(c) ? (c - 'a' - 'A') : c;
}

inline int isalpha(int c)
{
	return isupper(c) || islower(c);
}

inline int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

inline int isprint(int c)
{
	return isalnum(c);
}
