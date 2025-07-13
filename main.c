#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fig_ds.h"
#include "fig_parser.h"
#include "fig_lex.h"
#include "fig_ast.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

#define OK 0
#define ERROR 1


void usage(FILE* stream){
	fprintf(stream, "Usage: ./example [OPTIONS] [--] <OUTPUT FILES...>\n");
    fprintf(stream, "OPTIONS:\n");
	flag_print_options(stream);
}


int main(int argc, char** argv)
{
	bool* help = flag_bool("help", false, "Prints out help");
	// TODO: Change default to NULL
	char** file_name = flag_str("filename", "main.fig", "Filename or path to file to parse");


	if(!flag_parse(argc, argv)){
		usage(stderr);
		return ERROR;
	}

	if(*help){
		usage(stdout);
		return OK;
	}

	if(!(*file_name)){
		fprintf(stderr, "Filename is empty!\n");
		usage(stderr);
		return ERROR;
	}


	list* content = file_read(*file_name);
	if(!content){
		fprintf(stderr, "Error while reading file\n");
		return ERROR;
	}
	list* tokens = lex(content);
	if(!tokens){
		fprintf(stderr, "Error while lexing file\n");
		return ERROR;
	}

	for(size_t i = 0; i < tokens->size; i++){
		token* tok = get_token(tokens, i);
		printf("TokenType: (%i) TokenInfo: %i, Value: %s\n", tok->type, tok->token_info, tok->value);
	}
	ExprNode* ast = generate_ast(tokens);
	if(!ast){
		printf("Error while generating AST\n");
		return ERROR;
	}

	print_ast(ast, 0);
	printf("Result of ast: %i", compute_ast(ast));

	free_list(tokens);
	free_list(content);

	return OK;
}