#ifndef FIG_LEXER
#define FIG_LEXER 1

#include "fig_ds.h"
#include <stdbool.h>
#include <string.h>

typedef enum {IDENTIFIER=0, KEYWORD, OPERATOR, NUMBER, SEPARATOR} TokenType;

typedef struct{
	TokenType type;
	uint8_t token_info;
	char* value;
} token;

void push_token(list* l, token* tok){
	if(!l || !tok){
		fprintf(stderr, "Null list or token passed!\n");
		return;
	}
	if(l->size+1 > l->capacity){
		realloc_list(l);
	}
	((token**)l->data)[l->size] = tok;
	l->size++;
}

token* pop_token(list* l){
	if(!l){
		fprintf(stderr, "Null list while popping!\n");
		return NULL;
	}
	if(l->size == 0){
		fprintf(stderr, "Cannot pop token from empty list\n");
		return NULL;
	}
	token* tok = ((token**)l->data)[l->size-1];
	((token**)l->data)[l->size-1] = NULL;
	l->size--;
	return tok;
}


token* get_token(list* l, size_t index){
	if(index > l->size){
		fprintf(stderr, "GET_TOKEN: Out of bounds error! Accesing %p at %u\n", l, index);
		return 0;	
	}
	return ((token**)l->data)[index];
}


bool can_be_in_keyword(char c){
	if((c>= 'a' && c <= 'z') || (c>= 'A' && c <= 'Z')){
		return true;
	}
	return false;
}

bool can_be_in_number(char c){
	if((c>= '0' && c <= '9') || c == '.'){
		return true;
	}
	return false;
}

char* operator_characters = "><=-+*/&|!,";

bool can_be_in_operator(char c){
	for(size_t i = 0; i < strlen(operator_characters); i++){
		if(operator_characters[i] == c){
			return true;
		}
	}
	return false;
}


char* separator_characters = "()[]{};,";

bool can_be_in_separator(char c){
	for(size_t i = 0; i < strlen(separator_characters); i++){
		if(separator_characters[i] == c){
			return true;
		}
	}
	return false;
}

token* new_token(TokenType type, uint8_t token_info, char* value){
	token* tok = malloc(sizeof(token));
	if(!tok){
		fprintf(stderr, "Could not allocate token!\n");
		return NULL;
	}
	tok->type = type;
	tok->token_info = token_info;
	tok->value = value;
	return tok;
}

#define KEYWORD_COUNT 2
char* KEYWORDS[KEYWORD_COUNT] = {"if", "else"};

#define OPERATOR_COUNT 18


#define GT_EQ     0  // >=
#define LT_EQ     1  // <=
#define NEQ       2  // !=
#define EQ        3  // ==
#define PLS_EQ    4  // +=
#define MIN_EQ    5  // -=
#define MUL_EQ    6  // *=
#define DIV_EQ    7  // /=
#define AND_OP    8  // &&
#define OR_OP     9  // ||
#define NOT_OP   10  // !
#define PLUS     11  // +
#define MINUS    12  // -
#define MULT     13  // *
#define DIV      14  // /
#define ASSIGN   15  // =
#define GT       16  // >
#define LT       17  // <

char* OPERATORS[OPERATOR_COUNT] = {">=", "<=", "!=", "==", "+=", "-=", "*=", "/=", "&&", "||", "!", "+", "-", "*", "/", "=", ">", "<"};


bool operator_left_to_right(size_t index) {
    switch (index) {
        // Right-to-left associativity
        case PLS_EQ:
        case MIN_EQ:
        case MUL_EQ:
        case DIV_EQ:
        case NOT_OP:
        case ASSIGN:
            return false;

        // Left-to-right associativity
        case GT_EQ:
        case LT_EQ:
        case NEQ:
        case EQ:
        case AND_OP:
        case OR_OP:
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case GT:
        case LT:
            return true;
    }
    return true; // Default to left-to-right
}

size_t operator_precedence(size_t index) {
	// https://en.cppreference.com/w/c/language/operator_precedence.html
    switch (index) {
        case GT_EQ:
        case LT_EQ:
        case GT:
        case LT:
            return 6;

        case NEQ:
        case EQ:
            return 7;

        case PLS_EQ:
        case MIN_EQ:
        case MUL_EQ:
        case DIV_EQ:
        case ASSIGN:
            return 14;

        case AND_OP:
            return 11;

        case OR_OP:
            return 12;

        case NOT_OP:
            return 1;

        case PLUS:
        case MINUS:
            return 4;

        case MULT:
            return 3;

        case DIV:
            return 2;
    }
    return 0; // Unknown or invalid index
}

token* lex_keyword(list* file_content, size_t index){
	size_t index_end = index;
	while(index_end < file_content->size && can_be_in_keyword(get_char(file_content, index_end)) ){
		index_end++;
	}
	size_t substring_length = index_end - index;

	if(substring_length == 0){
		return NULL;
	}


	char* substring = (char*)malloc(substring_length+1);
	if(!substring){
		return NULL;
	}

	memcpy(substring, &((char*)file_content->data)[index], substring_length*sizeof(char));
	substring[substring_length] = '\0';

	for(size_t i = 0; i < KEYWORD_COUNT; i++){
		if(strcmp(KEYWORDS[i], substring) == 0){
			token* tok = new_token(KEYWORD, (uint8_t)i, substring);
			return tok;
		}
	}
	free(substring);
	return NULL;
}

token* lex_number(list* file_content, size_t index){
	size_t index_end = index;
	while(index_end < file_content->size && can_be_in_number(get_char(file_content, index_end))){
		index_end++;
	}
	size_t substring_length = index_end - index;

	if(substring_length == 0){
		return NULL;
	}

	char* substring = (char*)malloc(substring_length+1);
	if(!substring){
		return NULL;
	}

	memcpy(substring, &((char*)file_content->data)[index], substring_length*sizeof(char));
	substring[substring_length] = '\0';
	return new_token(NUMBER, 0, substring);
}

token* lex_identifier(list* file_content, size_t index){
	size_t index_end = index;
	while(index_end < file_content->size && can_be_in_keyword(get_char(file_content, index_end))){
		index_end++;
	}
	size_t substring_length = index_end - index;

	if(substring_length == 0){
		return NULL;
	}

	char* substring = (char*)malloc(substring_length+1);
	if(!substring){
		return NULL;
	}

	memcpy(substring, &((char*)file_content->data)[index], substring_length*sizeof(char));
	substring[substring_length] = '\0';
	return new_token(IDENTIFIER, 0, substring);
}


token* lex_operator(list* file_content, size_t index){
	size_t index_end = index;
	while(index_end < file_content->size && can_be_in_operator(get_char(file_content, index_end))){
		index_end++;
	}
	size_t substring_length = index_end - index;

	if(substring_length == 0){
		return NULL;
	}

	char* substring = (char*)malloc(substring_length+1);
	if(!substring){
		return NULL;
	}

	memcpy(substring, &((char*)file_content->data)[index], substring_length*sizeof(char));
	substring[substring_length] = '\0';

	for(size_t i = 0; i < OPERATOR_COUNT; i++){
		if(strcmp(OPERATORS[i], substring) == 0){
			token* tok = new_token(OPERATOR, (uint8_t)i, substring);
			return tok;
		}
	}
	free(substring);
	return NULL;
}

token* lex_separator(list* file_content, size_t index){
	size_t index_end = index+1;
	if(index_end > file_content->size || !can_be_in_separator(get_char(file_content, index))){
		return NULL;
	}
	size_t substring_length = index_end - index;

	if(substring_length == 0){
		return NULL;
	}

	char* substring = (char*)malloc(substring_length+1);
	if(!substring){
		return NULL;
	}

	memcpy(substring, &((char*)file_content->data)[index], substring_length*sizeof(char));
	substring[substring_length] = '\0';
	return new_token(SEPARATOR, get_char(file_content, index), substring);
}

size_t consume_comment(list* file_content, size_t index){
	if(index+1 > file_content->size){
		return 0;
	}
	size_t index_end = index + 1;
	if(!(get_char(file_content, index) == '/' && get_char(file_content, index_end) == '/')){
		return 0;
	}
	// Consume the comment until newline
	while(index_end < file_content->size && get_char(file_content, index_end) != '\n'){
		index_end++;
	}
	return index_end - index;
}


typedef token* (*LexTokenFunction)(list* file_content, size_t index);

list* lex(list* file_content){
	size_t index = 0;
	list* tokens = new_list(sizeof(token*), 128);


	LexTokenFunction lexing_functions[] = {
		lex_keyword,
		lex_number,
		lex_identifier,
		lex_operator,
		lex_separator
	};
	char c;
	while(index < file_content->size){
		c = get_char(file_content, index);
		size_t comment_size = consume_comment(file_content, index);
		if(comment_size){
			index += comment_size;
			continue;
		}

		bool found_token = false;
		for(size_t i = 0; i < sizeof(lexing_functions) / sizeof(lexing_functions[0]); i++){
			token* tok = lexing_functions[i](file_content, index);
			if(tok){
				index += strlen(tok->value);
				push_token(tokens, tok);
				found_token = true;
				break;
			}
		}
		if(!found_token){
			index++;
		}

	}
	return tokens;
}

#endif