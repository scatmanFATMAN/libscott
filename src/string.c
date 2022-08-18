#include "string.h"

size_t
strlcpy(char *dst, const char *src, size_t size) {
	const char *osrc;
	size_t left;

	osrc = src;
	left = size;


	if (left != 0) {
		while (--left != 0) {
			if ((*dst++ = *src++) == '\0') {
				break;
			}
		}
	}

	if (left == 0) {
		if (size != 0) {
			*dst = '\0';
		}
		while (*src++);
	}

	return src - osrc - 1;
}
