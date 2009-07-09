#include <stdio.h>

#include "lr_string_utf8.h"

using namespace lr;

typedef lrStringUTF8 <> lrString;

int main(void) {
	lrString string(lrString::ASCII("Hello world!\n"));

	string.insert(0,L'W');

	printf("%s",string.get_storage().begin());
	return 0;
}
