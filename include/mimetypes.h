#pragma once

#include <string.h>

enum {
	TEXT_HTML, TEXT_PLAIN
};

const char *mimetypes[] = {
	"text/html", "text/plain"
};

char *get_filexten(const char *filepath) {
	char *ext = NULL;

	for(int i = strlen(filepath) - 1; i > 0; i--) {
		if(filepath[i] != '.')
			continue;

		ext = (char *) &filepath[i] + 1;
		break;
	}

	return ext;
}

const char *get_mimetype(const char *filepath) {
	char *ext = get_filexten(filepath);
	
	if(ext != NULL) {
		if(strcmp(ext, "html"))
			return mimetypes[TEXT_HTML];

		else if(strcmp(ext, "txt"))
			return mimetypes[TEXT_PLAIN];
	}

	return NULL;
}
