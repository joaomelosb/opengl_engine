#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "load_file.h"

#define READ_CHUNK 512

char *load_file(char const *filename, size_t *size) {
	FILE *fp;
	
	if (!(fp = fopen(filename, "rb"))) {
		LOG("can't load file \"%s\": %s", filename, strerror(errno));
		return NULL;
	}
	
	char *buf = NULL, tmp[READ_CHUNK];
	size_t _size = 0;
	
	for (;;) {
		size_t count = fread(tmp, 1, READ_CHUNK, fp);
		
		if (ferror(fp)) {
			LOG_ERR("couldn't read file \"%s\": %s", filename, strerror(errno));
			free(buf);
			return NULL;
		}
		
		void *ptr = realloc(buf, _size + count + 1);
		
		if (!ptr) {
			LOG("couldn't allocate enough space for \"%s\"", filename);
			free(buf);
			return NULL;
		}
		
		buf = ptr;
		
		if (count) {
			memcpy(buf + _size, tmp, count);
			_size += count;
		}
		
		if (feof(fp))
			break;
	}
	
	buf[_size] = 0;
	
	if (size)
		*size = _size;
	
	return buf;
}