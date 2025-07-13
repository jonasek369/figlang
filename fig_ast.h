#ifndef FIG_AST
#define FIG_AST 1

#include "fig_ds.h"
#include "fig_lex.h"
#include <assert.h>

typedef struct ExprNode{
	token* op;
	struct ExprNode* operand1;
	struct ExprNode* operand2;
} ExprNode;


void print_ast(ExprNode* node, int depth) {
    if (node == NULL) return;

    // Indent based on depth
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Print operator/function/number
    if (node->op) {
        printf("%s\n", node->op->value);  // Adjust this if token_info is not a string
    } else {
        printf("(null)\n");
    }

    // Recursively print operands
    print_ast(node->operand1, depth + 1);
    print_ast(node->operand2, depth + 1);
}

ExprNode* new_exprnode(token* tok, ExprNode* op1, ExprNode* op2){
	ExprNode* node = malloc(sizeof(ExprNode));
	if(!node){
		fprintf(stderr, "Could not allocated memory for exprnode!\n");
	}
	node->op = tok;
	node->operand1 = op1;
	node->operand2 = op2;
	return node;
}

void push_exprnode(list* l, ExprNode* node){
	if(!l || !node){
		fprintf(stderr, "Null list or token passed!\n");
		return;
	}
	if(l->size+1 > l->capacity){
		realloc_list(l);
	}
	((ExprNode**)l->data)[l->size] = node;
	l->size++;
}

ExprNode* pop_exprnode(list* l){
	if(!l){
		fprintf(stderr, "Null list while popping!\n");
		return NULL;
	}
	if(l->size == 0){
		fprintf(stderr, "Cannot pop from empty list\n");
		return NULL;
	}
	ExprNode* node = ((ExprNode**)l->data)[l->size-1];
	((ExprNode**)l->data)[l->size-1] = NULL;
	l->size--;
	return node;
}


ExprNode* get_exprnode(list* l, size_t index){
	if(index > l->size){
		fprintf(stderr, "Out of bounds error!\n");
		return 0;	
	}
	return ((ExprNode**)l->data)[index];
}

// TODO: Fix ast 
// based on: https://softwareengineering.stackexchange.com/questions/254074/how-exactly-is-an-abstract-syntax-tree-created
/*ExprNode* generate_ast(list* tokens){
	list* opStack = new_list(sizeof(token), 32);
	list* exprStack = new_list(sizeof(ExprNode), 32);
	for(size_t i = 0; i < tokens->size; i++){
		token* tok = get_token(tokens, i);
		if(tok->type == SEPARATOR && *tok->value == '('){
			printf("parentheses!\n");
			push_token(opStack, tok);
		}else if(tok->type == NUMBER){
			ExprNode* node = new_exprnode(tok, NULL ,NULL);
			push_exprnode(exprStack, node);
		}else if(tok->type == OPERATOR){

			while(opStack->size > 0 && operator_precedence(get_token(opStack, opStack->size-1)->token_info) <= operator_precedence(tok->token_info)){

				token* op = pop_token(opStack);
				ExprNode* node;
				if(operator_left_to_right(op->token_info)){
				    ExprNode* e1 = pop_exprnode(exprStack);
				    ExprNode* e2 = pop_exprnode(exprStack);
				    node = new_exprnode(op, e1, e2);
				}else{
				    ExprNode* e1 = pop_exprnode(exprStack);
				    ExprNode* e2 = pop_exprnode(exprStack);
				    node = new_exprnode(op, e2, e1);
				}

				push_exprnode(exprStack, node);
			}
			push_token(opStack, tok);
		}else if(tok->type == SEPARATOR && *tok->value == ')'){
			while(opStack->size > 0 && *(get_token(opStack, opStack->size-1)->value) != '('){
				token* op = pop_token(opStack);
				ExprNode* node;
				if(operator_left_to_right(op->token_info)){
				    ExprNode* e1 = pop_exprnode(exprStack);
				    ExprNode* e2 = pop_exprnode(exprStack);
				    node = new_exprnode(op, e1, e2);
				}else{
				    ExprNode* e1 = pop_exprnode(exprStack);
				    ExprNode* e2 = pop_exprnode(exprStack);
				    node = new_exprnode(op, e2, e1);
				}

				push_exprnode(exprStack, node);
			}
		}else{
			fprintf(stderr, "Error while making AST!\n");
		}
	}

	while(opStack->size > 0){
		token* op = pop_token(opStack);
		ExprNode* node;
		if(operator_left_to_right(op->token_info)){
		    ExprNode* e1 = pop_exprnode(exprStack);
		    ExprNode* e2 = pop_exprnode(exprStack);
		    node = new_exprnode(op, e1, e2);
		}else{
		    ExprNode* e1 = pop_exprnode(exprStack);
		    ExprNode* e2 = pop_exprnode(exprStack);
		    node = new_exprnode(op, e2, e1);
		}

		push_exprnode(exprStack, node);
	}

	return pop_exprnode(exprStack);
}*/

token* top_token(list* tokens){
	return get_token(tokens, tokens->size-1);
}

void pop_push_operator(list* opStack, list* exprStack){
	token* op = pop_token(opStack);
	ExprNode* right = pop_exprnode(exprStack);
	ExprNode* left  = pop_exprnode(exprStack);
	ExprNode* node = new_exprnode(op, left, right);
	push_exprnode(exprStack, node);
}

// based on https://en.wikipedia.org/wiki/Shunting_yard_algorithm
ExprNode* generate_ast(list* tokens){
	list* opStack = new_list(sizeof(token), 32);
	list* exprStack = new_list(sizeof(ExprNode), 32);
	for(size_t i = 0; i < tokens->size; i++){
		token* tok = get_token(tokens, i);
		if(tok->type == NUMBER){
			ExprNode* node = new_exprnode(tok, NULL ,NULL);
			push_exprnode(exprStack, node);
		}
		else if(tok->type == IDENTIFIER){
			push_token(opStack, tok);
		}else if(tok->type == OPERATOR){
			while(
				opStack->size > 0 && 
				!(top_token(opStack)->type == SEPARATOR && top_token(opStack)->token_info == '(') && // is left parenthesis
				(
					operator_precedence(top_token(opStack)->token_info) < operator_precedence(tok->token_info) || (
						operator_precedence(top_token(opStack)->token_info) == operator_precedence(tok->token_info) && 
						operator_left_to_right(tok->token_info)
					)
				) // Using C precedence which is flipped (smaller higher precedence)
				){
				pop_push_operator(opStack, exprStack);
			}
			push_token(opStack, tok);
		}else if(tok->type == SEPARATOR && tok->token_info == ','){
			while(!(top_token(opStack)->type == SEPARATOR && top_token(opStack)->token_info == '(')){
				pop_push_operator(opStack, exprStack);
			}
		}else if((tok->type == SEPARATOR && tok->token_info == '(')){
			push_token(opStack, tok);
		}else if((tok->type == SEPARATOR && tok->token_info == ')')){
			while(opStack->size > 0 && !(top_token(opStack)->type == SEPARATOR && top_token(opStack)->token_info == '(')){
				assert(opStack->size != 0 && "Operator stack should not be empty here!");
				pop_push_operator(opStack, exprStack);
			}
			if(opStack->size>0){
				assert((top_token(opStack)->type == SEPARATOR && top_token(opStack)->token_info == '(') && "Top operator should be left parenthesis");
			}
			

			if (opStack->size == 0) {
			    fprintf(stderr, "Mismatched parentheses: No '(' found for ')'\n");
			    return NULL;
			}

			pop_token(opStack); // discard ')'

			if(opStack->size > 0 && top_token(opStack)->type == IDENTIFIER){
				pop_push_operator(opStack, exprStack);
			}
		}
	}
	while(opStack->size > 0){
		assert(!(top_token(opStack)->type == SEPARATOR && (top_token(opStack)->token_info == '(' || top_token(opStack)->token_info == ')')) && "Parenthesis mismatch!");
		pop_push_operator(opStack, exprStack);
	}

	return get_exprnode(exprStack, 0); // Show be root now
}


int compute_ast(ExprNode* node){
	if(!node || !node->op){
		fprintf(stderr, "Invalid node");
		return 0;
	}
	switch(node->op->type){
		case NUMBER: {
			return atoi(node->op->value);
		}
		case OPERATOR: {
			int left = compute_ast(node->operand1);
			int right = compute_ast(node->operand2);

			if(node->op->token_info == PLUS) return left + right;
			if(node->op->token_info == MINUS) return left - right;
			if(node->op->token_info == MULT) return left * right;
			if(node->op->token_info == DIV) return left / right;
		}
	}
}



#endif