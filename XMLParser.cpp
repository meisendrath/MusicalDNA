#include "XMLParser.h"


#include "HAL.h"

#include <stdlib.h>


bool MYSTRCMP(const char *a, const char *b) {
	return (strlen(a) == strlen(b)) && (strcmp(a, b) == 0);
}

