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

typedef struct
{
	char command[MAX_COMMAND_SIZE+1];
	int cursor_pos;
	int command_size;
}line_t;

void printStatus(line_t *line)
{
	printf("\n\r%s\n\r", line->command);
	printf("cur %d | siz %d\n\r", line->cursor_pos, line->command_size);
}

void line_init(line_t *line)
{
	line->cursor_pos = 0;
	line->command_size = 0;
	strcpy(line->command, "");
	line->command[MAX_COMMAND_SIZE] = '\0';
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

int addChar(line_t *line, char chr)
{
	int i;

	if (line->command_size >= MAX_COMMAND_SIZE)
		return BUFFER_FULL;

	if (line->cursor_pos < line->command_size)
	{
		sendChar(chr);
		for (i=line->command_size; i>=line->cursor_pos; i--)
			line->command[i+1] = line->command[i];

		line->command[line->cursor_pos] = chr;
		sendString(&(line->command[line->cursor_pos+1]));

		line->cursor_pos++;
		line->command_size++;
		for (i=line->command_size; i>line->cursor_pos; i--)
			sendChar('\b');
	}
	else
	{
		line->command[line->cursor_pos] = chr;
		sendChar(chr);
		line->cursor_pos++;
		line->command_size++;
		line->command[line->cursor_pos] = '\0';
	}

	return 0;
}

int handleLeft(line_t *line)
{
	if (line->cursor_pos <= 0)
		return 0;

	line->cursor_pos--;
	sendChar('\b');

	return 0;
}

int handleRight(line_t *line)
{
	if (line->cursor_pos >= line->command_size)
		return 0;

	sendChar(line->command[line->cursor_pos]);
	line->cursor_pos++;

	return 0;
}

void handleArrow(line_t *line, char arrow)
{
	arrow = toupper((int)arrow);

	switch(arrow)
	{
	case ARROW_UP:
		//printf("up\n\r");
		printStatus(line);
		break;
	case ARROW_DOWN:
		printf("down\n\r");
		break;
	case ARROW_RIGHT:
		//printf("right\n\r");
		handleRight(line);
		break;
	case ARROW_LEFT:
		handleLeft(line);
		break;
	default:
		break;
	}
}

char checkEscape(line_t *line, char escape, char chr)
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
		handleArrow(line, chr);
		escape = 0;
	}

	return 0;
}

int handleCommand(line_t *line)
{
	sendString("\n\r");
	sendString("got: ");
	sendString(line->command);
	sendString("\n\r");
	line_init(line);

	return 0;
}

int handleBackSpace(line_t *line)
{
	int i;

	if (line->cursor_pos <= 0)
		return 0;

	if (line->cursor_pos < line->command_size)
	{
		sendChar('\b');
		for (i=line->cursor_pos-1; i<line->command_size-1; i++)
		{
			sendChar(line->command[i+1]);
			line->command[i] = line->command[i+1];
		}
		line->command[i] = '\0';
		sendChar(' ');

		for (i=line->command_size; i>line->cursor_pos-1; i--)
			sendChar('\b');
	}
	else
	{
		line->command[line->cursor_pos-1] = '\0';
		sendString("\b \b");
	}

	line->cursor_pos--;
	line->command_size--;

	return 0;
}

int handleSpecial(line_t *line, char chr)
{
	switch(chr)
	{
	case '\r': /* carriage return */
		handleCommand(line);
		break;
	case '\b': /* backspace or ctrl+h */
	case 127:
		handleBackSpace(line);
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

void mainLoop(line_t *line)
{
	char chr;
	char escape;

	for (;;)
	{
		chr = get_char();

		if (DEBUG && chr == '\0')
			break;

		if (chr < 0)
			continue;

		if (escape != 0 || chr == ESC_CODE)
		{
			escape = checkEscape(line, escape, chr);
			continue;
		}

		if (chr < 27 || chr == 127)
		{
			handleSpecial(line, chr);
		}
		else
		{
			addChar(line, chr);
		}
	}

	escape = 0;
	line_init(line);
}

int main(int argc, char *argv[])
{
	line_t line;

    initscr();
    cbreak();
    noecho();

    mainLoop(&line);
    /*for (;;)
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

		if (chr < 27 || chr == 127)
		{
			handleSpecial(chr);
		}
		else
		{
			addChar(chr);
		}
	}*/
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
