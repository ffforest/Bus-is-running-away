// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "alloc.h"


int is_char(char c) {
	if (isalpha(c) || isnumber(c) || c == '!' || c == '%' || c == '+' || c == ',' || c == '-' ||
            c == '.' || c =='/' || c == ':' || c == '@' || c == '^' || c == '_')
        return 1;
    return 0;
}

int is_space(char* string) {
	while(*string != '\0') {
		if(isspace(*string))
			return 1;
		string++;
	}
	return 0;
}

typedef struct simple_command{
	char** command;
	char* input;
	char* output;
} simple_command, *simple_command_t;

typedef enum {SIMPLE_CMD, SEMICOLON, PIPELINE, AND, OR, LEFT_PAREN, RIGHT_PAREN} token_type_t;

typedef struct token {
	token_type_t type;
	simple_command_t command;
	struct token* next;
} token, *token_t;

//linked list implementation of stack_node
typedef struct stack_node {
    command_t cmd;
    struct stack_node* next;
} stack_node, *stack_node_t;

int stackNodePush(stack_node_t* head, command_t command) {
    if (head == NULL || *head == NULL)
        return 1;
    stack_node_t newHead = (stack_node_t)checked_malloc(sizeof(struct stack_node));
    newHead->next = *head;
    newHead->cmd = command;
    *head = newHead;
    return 0;
}

command_t stackNodePop(stack_node_t* head) {
    if(head == NULL || *head == NULL)
        return NULL;
    command_t cmd = (*head)->cmd;
    *head=(*head)->next;
    stack_node_t temp = *head;
    free(temp);
    return cmd;
}

/* FIXME: Define the type 'struct command_stream' here.  This should
 complete the incomplete type declaration in command.h.  */

//doubly linked list implementation of command_stream
struct command_stream {
    command_t cmd;
    struct command_stream* prev;
    struct command_stream* next;
};

command_stream_t createCommandStream() {
    command_stream_t newcmd = checked_malloc(sizeof(struct command_stream));
    newcmd->prev = NULL;
    newcmd->next = NULL;
    return newcmd;
}

int empty(command_stream_t cmd){
    return (cmd->next == NULL);
}

void commandStreamPush(command_stream_t head, command_t cmd)
{
    command_stream_t newcmd = checked_malloc(sizeof(struct command_stream));
    newcmd->cmd = cmd;
    newcmd->next = head;
    newcmd->prev = head->prev;
    head->prev->next = newcmd;
    head->prev = newcmd;
}

//remove the second item from the doubly linked list
command_t commandStreamPop(command_stream_t head)
{
    command_stream_t temp = head->next;
    command_t cmd = temp->cmd;
    head->next->next->prev = head;
	head->next = head->next->next;
    free(temp);
    return cmd;
}

char* substr(int start_pos, int end_pos, char* string) {
	size_t size = end_pos - start_pos + 1;
	char* substr = (char*)checked_malloc(size);
	memcpy((void*)substr, string + start_pos, size);
	substr[size] = '\0';
	return substr;
}

//This function accepts a simple command in string format, parses it, then creates a simple command and updates the corresponding attributes of it
simple_command_t parseSimpleCommand (char* simpleCommandString)
{
    simple_command_t t = (simple_command_t)checked_malloc(sizeof(struct simple_command));
    char* p = simpleCommandString;
    int start_pos = 0;
    
    int found_word = false;
    
    //If there are both input and output redirections, i.e. '<' and '>', the '>' must be on the right, as indicated in the spec
    if((strchr(simpleCommandString, '>') < strchr(simpleCommandString, '<')) && strchr(simpleCommandString, '<') != NULL && strchr(simpleCommandString, '>') != NULL)
		return NULL;
    
    //if(!*p)
		//return NULL;
    
    //if the first character is not a character listed above in is_char() function, return NULL
	if(!is_char(*p))
		return NULL;
    
    //if there are spaces at the beginning, skip them
    while(isspace(*p))
    {
		p++;
		start_pos++;
	}
    
    /* found a word for the command string */
	found_word = true;
	
    const int max_word = 16;
	int initialsize = max_word*sizeof(char*);
	char** wordList = (char**)checked_malloc(initialsize);
	int nWords = 0;
	int word_start = start_pos;
    int word_end = 0;
	int isInWord = false;
	/* Found the command string */
	while(*p != '\0' && (*p != '<' && *p != '>'))
    {
		if(is_char(*p))
        {
			if(isInWord != false)
            {
				isInWord = true;
				word_start = p - simpleCommandString;
			}
			word_end = p - simpleCommandString;
		}
        else if(isspace(*p) && isInWord)
        {
			isInWord = false;
			if(nWords == max_word)
            {
				initialsize *= 2;
				wordList = (char**)checked_realloc((void*) wordList, initialsize);
			}
			wordList[nWords] = substr(word_start, word_end, simpleCommandString);
			nWords++;
		}
		p++;
	}
    return t;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");
    
    return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
    return 0;//commandStreamPop(s);
}


