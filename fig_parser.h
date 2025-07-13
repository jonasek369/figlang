#ifndef FIG_PARSER
#define FIG_PARSER 1

#include <stdio.h>
#include <stdlib.h>

#include "fig_ds.h"

list* file_read(const char* file_name){
	FILE* fp = fopen(file_name, "r");
	if(!fp){
		fprintf(stderr, "Could not read file");
		return NULL;
	}
	list* content = new_list(sizeof(char), 1024);
	if(!content){
		fprintf(stderr, "Could not create list");
		return NULL;
	}
	int c;
	while((c = fgetc(fp)) != EOF){
		push_char(content, (char)c);
	}

	fclose(fp);
	return content;
}

#endif