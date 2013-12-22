#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <ncurses/ncurses.h>
#include <string.h>

#define DEBUG 0

#define MAX_COMMAND_SIZE 63

#define BUFFER_FULL -10;

#define ESC_CODE 0x1b

#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'

char teststr[MAX_COMMAND_SIZE+1] = "abc\e[Dx";
char command[MAX_COMMAND_SIZE+1];
int cursor_pos;
int command_size;

void printStatus()
{
	printf("\n\r%s\n\r", command);
	printf("cur %d | siz %d\n\r", cursor_pos, command_size);
}

void cli_init()
{
	cursor_pos = 0;
	command_size = 0;
	command[0] = '\0';
	command[MAX_COMMAND_SIZE] = '\0';
}

int sendChar(char ch)
{
	printf("%c", ch);
	return 0;
}

int sendString(char *str)
{
	printf("%s", str);
	return 0;
}

int addChar(char chr)
{
	int i;

	if (command_size >= MAX_COMMAND_SIZE)
		return BUFFER_FULL;

	if (cursor_pos < command_size)
	{
		sendChar(chr);
		for (i=command_size; i>=cursor_pos; i--)
			command[i+1] = command[i];

		command[cursor_pos] = chr;
		sendString(&command[cursor_pos+1]);

		cursor_pos++;
		command_size++;
		for (i=command_size; i>cursor_pos; i--)
			sendChar('\b');
	}
	else
	{
		command[cursor_pos] = chr;
		sendChar(chr);
		cursor_pos++;
		command_size++;
		command[cursor_pos] = '\0';
	}

	return 0;
}

int handleLeft()
{
	if (cursor_pos <= 0)
		return 0;

	cursor_pos--;
	sendChar('\b');

	return 0;
}

int handleRight()
{
	if (cursor_pos >= command_size)
		return 0;

	sendChar(command[cursor_pos]);
	cursor_pos++;

	return 0;
}

void handleArrow(char arrow)
{
	arrow = toupper(arrow);

	switch(arrow)
	{
	case ARROW_UP:
		//printf("up\n\r");
		printStatus();
		break;
	case ARROW_DOWN:
		printf("down\n\r");
		break;
	case ARROW_RIGHT:
		//printf("right\n\r");
		handleRight();
		break;
	case ARROW_LEFT:
		handleLeft();
		break;
	default:
		break;
	}
}

char checkEscape(char escape, char chr)
{
	if (escape == 0)
	{
		if (chr == ESC_CODE)
			return ESC_CODE;
		else
			return 0;
	}

	if (chr == '[')
	{
		return chr;
	}

	if (escape == '[')
	{
		handleArrow(chr);
		escape = 0;
	}

	return 0;
}

int handleCommand()
{
	sendString("\n\r");
	cursor_pos = 0;
	command_size = 0;
	command[0] = '\0';

	return 0;
}

int handleBackSpace()
{
	int i;

	if (cursor_pos <= 0)
		return 0;

	if (cursor_pos < command_size)
	{
		sendChar('\b');
		for (i=cursor_pos-1; i<command_size-1; i++)
		{
			sendChar(command[i+1]);
			command[i] = command[i+1];
		}
		command[i] = '\0';
		sendChar(' ');

		for (i=command_size; i>cursor_pos-1; i--)
			sendChar('\b');
	}
	else
	{
		command[cursor_pos-1] = '\0';
		sendString("\b \b");
	}

	cursor_pos--;
	command_size--;

	return 0;
}

int handleSpecial(char chr)
{
	switch(chr)
	{
	case '\r': /* carriage return */
		handleCommand();
		break;
	case '\b': /* backspace or ctrl+h */
	case 127:
		handleBackSpace();
		break;
	default:
		break;
	}

	return 0;
}

int test()
{
	static int z = 0;
	if (DEBUG)
		return teststr[z++];

	return 'x';
}

int get_char()
{
	if (DEBUG)
		return test();

	return getc(stdin);
}

int main(int argc, char *argv[])
{
	char chr;
	char escape;
    initscr();
    cbreak();
    noecho();

    cli_init();

    escape = 0;
    for (;;)
	{
    	chr = get_char();

    	if (DEBUG && chr == '\0')
    		break;

    	if (chr < 0)
    		continue;

    	if (escape != 0 || chr == ESC_CODE)
    	{
    		escape = checkEscape(escape, chr);
   			continue;
    	}

		if (chr < 25 || chr == 127)
		{
			handleSpecial(chr);
		}
		else
		{
			addChar(chr);
		}
	}
    endwin();
    return(0);
}

int main3(void)
{
	char chr;
	struct termios term;

	initscr();

	tcgetattr(fileno(stdin), &term);
	/* turn off canonical mode and echo */
	term.c_lflag &= ~ICANON;
	term.c_lflag &= ~ECHO;
	tcsetattr(fileno(stdin), TCSANOW, &term);

	for (;;)
	{
		chr = getc(stdin);
		printf("%c %d %x\n\r", chr, chr, chr);
		//fflush(stdout);
	}

	return EXIT_SUCCESS;
}
