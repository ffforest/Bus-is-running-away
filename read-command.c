// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
 static function definitions, etc.  */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "alloc.h"


int is_char(char c)
{
	if (isalnum(c))
        return 1;
    if (c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.' || c =='/' || c == ':' || c == '@' || c == '^' || c == '_')
        return 1;
    return 0;
}

int is_space(char* string)
{
	while(*string != '\0')
    {
		if(isspace(*string))
			return 1;
		string++;
	}
	return 0;
}

char* substr(int start_pos, int end_pos, char* string)
{
	size_t size = end_pos - start_pos + 1;
	char* substr = (char*)checked_malloc(size);
	memcpy((void*)substr, string + start_pos, size);
	substr[size] = '\0';
	return substr;
}

char* getTail(char* string, int start) {
	char c = string[start];
	int end = start;
	while(c != '\0') {
		end++;
		c = string[end];
	}
	return substr(start, end, string);
}

typedef struct simple_command {
	char** command;
	char* input;
	char* output;
} *simple_command_t;

typedef enum { simple_command, and, or, pipes, semicolon, open_paren, close_paren } token_type_t;

typedef struct token {
	token_type_t type;
	simple_command_t command;
	struct token* next;
} token, *token_t;

//linked list implementation of stack_node
typedef struct stack_node {
    command_t data;
    struct stack_node* next;
} stack_node, *stack_node_t;

int stackNodePush(stack_node_t* head, command_t command)
{
    stack_node_t newHead = (stack_node_t)checked_malloc(sizeof(struct stack_node));
    newHead->next = *head;
    newHead->data = command;
    *head = newHead;
    return 0;
}

command_t stackNodePop(stack_node_t* head)
{
    if(head == NULL || *head == NULL)
        return NULL;
    command_t cmd = (*head)->data;
    stack_node_t temp = (*head);
    *head=(*head)->next;
    free(temp);
    return cmd;
}

char* getLine(int (*get_next_byte) (void *), void* get_next_byte_argument) {
	
	char* buffer = (char*) checked_malloc(50*sizeof(char));
	size_t buffersize = 50;
	unsigned int i = 0;
    
	int byte = (*get_next_byte) (get_next_byte_argument);
	char c;
	
	if(byte < 0)
		return NULL;
    
	for(;;)
	{
		
		if(byte < 0 || (c = (char) byte) == '\n' ) {
			buffer[i] = '\0';
			return buffer;
		}
		if(i + 1 >= buffersize) {
			buffersize += 50;
			buffer = (char*) checked_realloc((void*) buffer, buffersize);
		}
		buffer[i] = c;
		i++;
		byte = (*get_next_byte) (get_next_byte_argument);
	}
}

//This function accepts a simple command in string format, parses it, then creates a simple command and updates the corresponding attributes of it
simple_command_t parseSimpleCommand (char* simpleCommandString) {
	const char* lp;
	int start = 0;
	int running_alpha = 0;
	bool found_word = false;
	simple_command_t simp_command = (simple_command_t) checked_malloc(sizeof(struct simple_command));
	lp = simpleCommandString;

	/* check for mismatched ordering of I/O */
	if(strchr(simpleCommandString, '<') != NULL && strchr(simpleCommandString, '>') != NULL 
		&& (strchr(simpleCommandString, '<') > strchr(simpleCommandString, '>')))
		return NULL;

	/* Skip leading whitespace characters (use ctype.h)*/
	while(isspace((int)*lp)) {
		lp++;
		start++;
	}

	if(!*lp)
		return NULL;

	/* if first non-whitespace character is not part of the word set it is invalid input. return null. */
	if(!is_char(*lp)) 	
		return NULL;

	/* found a word for the command string */
	found_word = true;
	
	int wordLimit = 5;
	char** wordList = (char**) checked_malloc(wordLimit*sizeof(char*)+1);
	int nWords = 0;
	int running_start = start;
	bool inWord = false;
	/* Found the command string */
	while(*lp != '\0' && !strchr("<>", *lp)){
		if(is_char(*lp)) {
			if(!inWord){
				inWord = true;
				running_start = lp - simpleCommandString;
			}
			running_alpha = lp - simpleCommandString;
		} else if(isspace(*lp) && inWord) {
			inWord = false;
			if(nWords == wordLimit) {
				wordLimit = 2*wordLimit;
				wordList = (char**) checked_realloc((void*) wordList, wordLimit+1);
			}
			wordList[nWords] = substr(running_start, running_alpha, simpleCommandString);
			nWords++;
		}
		lp++;
	}
	
	if(inWord) {
		if(nWords == wordLimit) {
			wordList = (char**) checked_realloc((void*) wordList, 2*wordLimit+1);
		}
		wordList[nWords] = substr(running_start, running_alpha, simpleCommandString);
		nWords++;
	}
	wordList[nWords] = '\0';
	
	
	/* update simple command with command string */
	simp_command->command = wordList;	
	//simp_command->command = substr(start, running_alpha, simpleCommandString,);
	simp_command->input = simp_command->output = NULL;

	/* check if done */
	if(!*lp && found_word)
		return simp_command;

	/* reset found word for the input and/or output */
	found_word = false;

	/* it must be either < or > if not done*/
	
	/* look for input */
	if(strchr("<", *lp)) {
		lp++;
		start = lp - simpleCommandString;
		/* Skip leading whitespace characters */
		while(isspace((int)*lp)) {
			lp++;
			start++;
		}
		if(!*lp)
			return NULL;
		/* if first non-whitespace character is not part of the word set it is invalid input. return null. */
		if(!is_char(*lp)) 	
			return NULL;
		
		found_word = true;

		/* Found the input string */
		while(*lp != '\0' && !strchr(">", *lp)){
			//lp++;
			//start++;
			if(is_char(*lp)) {
				running_alpha = lp - simpleCommandString;
			}
			lp++;
		}
		
		/* update simple command with input string */
		simp_command->input = substr(start, running_alpha,simpleCommandString);

		/* check if done */
		// Check if there are any invalid characters in file name
		if(simp_command->input != NULL && is_space(simp_command->input))
			return NULL;
		else if(!*lp  && found_word)
			return simp_command;
	}

	/* reset found word flag */
	found_word = false;

	/* look for output */
	if(strchr(">", *lp)) {
		lp++;
		start = lp - simpleCommandString;
		/* Skip leading whitespace characters */
		while(isspace((int)*lp)) {
			lp++;
			start++;
		}
		if(!*lp)
			return NULL;
		/* if first non-whitespace character is not part of the word set it is invalid input. return null. */
		if(!is_char(*lp)) 	
			return NULL;
		
		found_word = true;

		/* Found the output string */
		while(*lp != '\0' && !strchr(">", *lp)){
			//lp++;
			//start++;
			if(is_char(*lp)) {
				running_alpha = lp - simpleCommandString;
			}
			lp++;
		}
		
		/* update simple command with input string */
		simp_command->output = substr(start, running_alpha,simpleCommandString);

		/* check if done */
		// Check if there are any invalid characters in file name
		if(simp_command->output != NULL && is_space(simp_command->output))
			return NULL;
		else if(!*lp  && found_word)
			return simp_command;
	}	

	return NULL;
}

token_t createSimpleCommandToken(char* string, int start, int end) {
	char* sub = substr(start, end, string);
	simple_command_t sc = parseSimpleCommand(sub);
	if(sc == NULL) {
		return NULL;
	}
	token_t t = (token_t) checked_malloc(sizeof(struct token));
	t->type = simple_command;
	t->command = sc;
	t->next = NULL;
	return t;
}


command_t makeTree(token_t head) 
{
	stack_node_t operandTop = NULL;
	stack_node_t operatorTop = NULL;
	
	if(head == NULL) 
	{
		return NULL;
	}
    
	while (head)
	{
		if(head->type == simple_command) 
		{
			command_t operand = (command_t) checked_malloc(sizeof(struct command));
			operand->type = SIMPLE_COMMAND;
			operand->u.word = head->command->command;
			operand->input = head->command->input;
			operand->output = head->command->output;
			stackNodePush(&operandTop, operand);
		} else if(head->type == close_paren) 
		{
			return NULL;
		}

		else if(head->type == open_paren)
		{
			int paren_count = 1;

			token_t endList = head;
			head = head->next;
			token_t startList = head;
			while(head != NULL) 
			{
				if(head->type == open_paren)
					paren_count++;
				if(head->type == close_paren) 
				{
					paren_count--;
					if(paren_count == 0) 
					{
						endList->next = NULL;
						break;
					}
				}
				
				head = head->next;
				endList = endList->next;
			}
			
			if(paren_count != 0) 
			{
				return NULL;
			}
			
			command_t subShell = makeTree(startList);
			
			if(subShell != NULL) {
				command_t operand = (command_t) checked_malloc(sizeof(struct command));
				operand->type = SUBSHELL_COMMAND;
				operand->u.subshell_command = subShell;
				stackNodePush(&operandTop, operand);
			}
			endList->next = head;
		}

		else if((head->type == and || head->type == or || head->type == pipes || head->type == semicolon) && (operatorTop!=NULL && (operatorTop->data->type == PIPE_COMMAND || ((operatorTop->data->type == AND_COMMAND || operatorTop->data->type == OR_COMMAND) && head->type != pipes)))) 
		{
	
			command_t operand = stackNodePop(&operatorTop);

			operand->u.command[1] = stackNodePop(&operandTop);
			operand->u.command[0] = stackNodePop(&operandTop);
			if(operand->u.command[1] == NULL || operand->u.command[0] == NULL) {
				return NULL;
			}

			stackNodePush(&operandTop, operand);

			command_t operator = (command_t) checked_malloc(sizeof(struct command));
			switch(head->type)
			{
				case and: 
					operator->type = AND_COMMAND;
					break;
				case or:
					operator->type = OR_COMMAND;
					break;
				case pipes:
					operator->type = PIPE_COMMAND;
					break;
				case semicolon:
					operator->type = SEQUENCE_COMMAND;
					break;
				default:
					break;
			}
			stackNodePush(&operatorTop, operator);
		}
		else {

			command_t operator = (command_t) checked_malloc(sizeof(struct command));
			switch(head->type)
			{
				case and: 
					operator->type = AND_COMMAND;
					break;
				case or:
					operator->type = OR_COMMAND;
					break;
				case pipes:
					operator->type = PIPE_COMMAND;
					break;
				case semicolon:
					operator->type = SEQUENCE_COMMAND;
					break;
				default:
					break;
			}
			stackNodePush(&operatorTop, operator);
		}
		head = head->next;
	}
	while(operatorTop) 
	{
		command_t operator = stackNodePop(&operatorTop);

		operator->u.command[1] = stackNodePop(&operandTop);
		operator->u.command[0] = stackNodePop(&operandTop);
		if(operator->u.command[1] == NULL || operator->u.command[0] == NULL) 
		{
			return NULL;
		}

		stackNodePush(&operandTop, operator);
		
		
	}

	command_t tree_head = stackNodePop(&operandTop);
	if(operandTop == NULL)
		return tree_head;
	else
		return NULL;
}


struct command_stream 
{
	command_t* data;
	size_t head;
	size_t tail;
	size_t limit;
};

bool isEmpty(command_stream_t stream)  
{
	if(stream->head == stream->tail)		
		return true;
	return false;
}

command_stream_t createCommandStream(void) 
{
	command_stream_t stream = (command_stream_t) checked_malloc(sizeof(struct command_stream));
	stream->data = (command_t*)checked_malloc(50*sizeof(command_t));
	stream->head = 0;
	stream->tail = 0;
	stream->limit = 50;
	return stream;
}

void commandStreamEnqueue(command_stream_t stream, command_t data) 
{
	if((stream->tail) > (stream->limit))
	{
		stream->limit += 50;
		stream->data = (command_t*)checked_realloc((void*) (stream->data), (stream->limit));
	}
	stream->data[stream->tail] = data;
	stream->tail++;
}

command_t commandStreamDequeue(command_stream_t stream) 
{
	if (stream->tail <= stream->head)
		return NULL;
	command_t data = stream->data[stream->head];
	stream->head++;

	return data;
}


command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument) {
	token_t headDummy = (token_t) checked_malloc(sizeof(struct token));
	headDummy->next = NULL;
	token_t tail = headDummy;
    
	command_stream_t commandstream = createCommandStream();

	char* line = getLine(*get_next_byte, get_next_byte_argument);
	int i = 0;
	int startCommand = 0;
	int lineNo = 1;
	
	bool inWord = false;
	int parenDepth = 0;
	char c;
	while(line != NULL) {
		while(1) {
			c = line[i];
            
			if(isalnum(c) || c == '!' || c == '%' || c == '+' || c == ',' || c == '-' ||
               c == '.' || c == '/' || c == ':' || c == '@' || c == '^' || c == '_' || c == '<' || c == '>') {
				inWord = true;
				i++;
			} else if(isspace(c)) {
				i++;
			} else if(c == '|') {
				if(!inWord) {
					fprintf(stderr, "%d: No command given\n", lineNo);
					exit(1);
				}
                
				token_t sct = createSimpleCommandToken(line, startCommand, i-1);
				if(sct == NULL) {
					fprintf(stderr, "%d: Invalid command\n", lineNo);
					exit(1);
				}
				
				token_t orPipeToken = (token_t) checked_malloc(sizeof(struct token));
				orPipeToken->command = NULL;
				orPipeToken->next = NULL;
				sct->next = orPipeToken;
				
				if(line[i+1] == '|') {
					orPipeToken->type = or;
					i += 2;
				} else {
					orPipeToken->type = pipes;
					i++;
				}
				tail->next = sct;
				tail = orPipeToken;
				startCommand = i;
			} else if(c == '&') {
				if(!inWord) {
					fprintf(stderr, "%d: No command given\n", lineNo);
					exit(1);
				}
				if(line[i+1] != '&') {
					fprintf(stderr, "%d: invalid character '&'\n", lineNo);
					exit(1);
				}
                
				token_t sct = createSimpleCommandToken(line, startCommand, i-1);
				if(sct == NULL) {
					fprintf(stderr, "%d: Not a valid command\n", lineNo);
					exit(1);
				} else {
					token_t andToken = (token_t) checked_malloc(sizeof(struct token));
					andToken->type = and;
					andToken->command = NULL;
					andToken->next = NULL;
					sct->next = andToken;
					tail->next = sct;
					tail = andToken;
					i += 2;
					startCommand = i;
				}
			} else if(c == '(' || c == ')') {
				token_t pToken = (token_t) checked_malloc(sizeof(struct token));
				if(c == '(') {
					parenDepth++;
					pToken->type = open_paren;
				} else {
					parenDepth--;
					if(parenDepth < 0) {
						fprintf(stderr, "%d: Incorrect parentheses\n", lineNo);
						exit(1);
					}
					pToken->type = close_paren;
				}
				pToken->command = NULL;
				pToken->next = NULL;
                
				if(inWord) {
					token_t sct = createSimpleCommandToken(line, startCommand, i-1);
					if(sct == NULL) {
						tail->next = pToken;
					} else {
						sct->next = pToken;
						tail->next = sct;
					}
					tail = pToken;
				} else {
					tail->next = pToken;
					tail = pToken;
				}
				i++;
				startCommand = i;
			} else if(c == ';') {
				if(parenDepth > 0) {
					token_t sequenceToken = (token_t) checked_malloc(sizeof(struct token));
					sequenceToken->type = semicolon;
					sequenceToken->command = NULL;
					sequenceToken->next = NULL;
					if(inWord) {
						token_t sct = createSimpleCommandToken(line, startCommand, i-1);
						if(sct == NULL) {
							fprintf(stderr, "%d: Invalid command\n", lineNo);
							exit(1);
						} else {
							sct->next = sequenceToken;
							tail->next = sct;
						}
					} else {
						tail->next = sequenceToken;
					}
					i++;
					startCommand = i;
					inWord = false;
					tail = sequenceToken;
				} else {
					if(inWord) {
						token_t sct = createSimpleCommandToken(line, startCommand, i-1);
						if(sct == NULL) {
							fprintf(stderr, "%d: Invalid statement\n", lineNo);
							exit(1);
						} else {
							tail->next = sct;
							tail = sct;
						}
					}
					command_t command = makeTree(headDummy->next);
					if(command != NULL) {
						commandStreamEnqueue(commandstream, command);
						token_t curr = headDummy->next;
						while(curr != NULL) {
							token_t tempToken = curr;
							curr = curr->next;
							free(tempToken);
						}
						headDummy->next = NULL;
						tail = headDummy;
						i++;
						char* tempLine = getTail(line, i);
						free(line);
						line = tempLine;
						i = 0;
						startCommand = 0;
						inWord = false;
					} else {
						// must have a valid statement if we get a ';'
						fprintf(stderr, "%d: Invalid command\n", lineNo);
						exit(1);
					}
				}
			} else if(c == '#' || c == '\0') {
				if(inWord) {
					token_t sct = createSimpleCommandToken(line, startCommand, i-1);
					if(sct != NULL) {
						tail->next = sct;
						tail = sct;
						inWord = false;
					}
				}
				command_t command = makeTree(headDummy->next);
				if(command != NULL) {
					commandStreamEnqueue(commandstream, command);
					token_t curr = headDummy->next;
					while(curr != NULL) {
						token_t tempToken = curr;
						curr = curr->next;
						free(tempToken);
					}
					headDummy->next = NULL;
					tail = headDummy;
				}
				free(line);
				i = 0;
				startCommand = 0;
				break;
			} else {
				fprintf(stderr, "%d: Invalid character\n", lineNo);
				exit(1);
			}
		}
		line = getLine(*get_next_byte, get_next_byte_argument);
		lineNo++;
	}
	if(isEmpty(commandstream)) {
		fprintf(stderr, "%d: Invalid structure\n", lineNo);
		exit(1);
	}
	return commandstream;
}

command_t read_command_stream (command_stream_t stream) {
	return commandStreamDequeue(stream);
}
