#ifndef ConsoleManager_H
#define ConsoleManager_H

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#define RIGHT 0
#define LEFT 1

#define BLACK 0
#define DARK_BLUE 1
#define DARK_GREEN 2
#define DARK_AQUA 3
#define DARK_CYAN 3
#define DARK_RED 4
#define DARK_PURPLE 5
#define DARK_PINK 5
#define DARK_MAGENTA 5
#define DARK_YELLOW 6
#define DARK_WHITE 7
#define GRAY 8
#define BLUE 9
#define GREEN 10
#define AQUA 11
#define CYAN 11
#define RED 12
#define PURPLE 13
#define PINK 13
#define MAGENTA 13
#define YELLOW 14
#define WHITE 15

enum Direction 
{ 
	LEFTARROW = 0x25, 
	UP  = 0x26, 
	RIGHTARROW = 0x27, 
	DOWN = 0x28 
};

#include <Windows.h>
#include <istream>

class ConsoleManager
{
public:
	~ConsoleManager();

	static ConsoleManager & GetInstance();

	// poll for user input
	void GetUserInput(char & c, bool line = true);
	void GetUserInput(char str[], bool line = true);
	void GetUserInput(int & x, bool line = true);

	// Add to current output buffer
	void AddToBufferCentered(char * c, int color = -1);
	void AddToBufferCentered(bool b, int color = -1);
	void AddToBufferCentered(int c, int color = -1);
	void AddToBufferCentered(char c, int color = -1);
	void AddToBuffer(char c, int color = -1, int times = 1);
	void AddToBuffer(char * c, int color = -1);
	void AddToBuffer(bool b, int color = -1);
	void AddToBuffer(int num, int color = -1);
	void AddBreakToBuffer(int lines = 1);

	void InsertInBuffer(char c);

	// working
	void HideCursor();
	void ShowCursor();

	// loop until mouse clicks/releases
	void WaitForLeftMouseClick();
	void WaitForRightMouseClick();
	void WaitForAnyMouseClick(bool & rl);
	void WaitForRightMouseRelease();
	void WaitForLeftMouseRelease();
	void WaitForAnyMouseRelease(bool & rl);

	// check for mosue clicks/releases
	bool IsLeftMouseClicked();
	bool IsRightMouseClicked();
	bool IsAnyMouseClicked();
	bool IsRightMouseReleased();
	bool IsLeftMouseReleased();
	COORD GetLastMouseClick();

	// check for various key presses
	int IsKeyPressed(char * chars);
	int IsArrowKeyPressed();
	int IsArrowOrKeyPressed(char * chars);
	bool IsKeyPressed(int key);

	// loop until keypress
	int WaitForKeyPressAny(char * waitStr = nullptr);
	void WaitForKeyPress(char key);
	int WaitForKeyPress(char * keys);

	// working?
	int GetHeight();
	void SetHeightAbs(int y);
	void SetHeightRel(int y);

	// Write to screen
	void Write(char * str, int color = -1);
	void Write(bool b, int color = -1);
	void Write(char c, int color = -1);
	void Write(int x, int color = -1);
	void WriteLine(char * str, int color = -1);
	void WriteLine(bool b, int color = -1);
	void WriteLine(char c, int color = -1);
	void WriteLine(int x, int color = -1);
	void WriteCentered(char * c, int color = -1);
	void WriteCentered(char c, int color = -1);
	void WriteCentered(int c, int color = -1);

	// Clear output buffer/screen
	void ClearText();

	// add new line
	void newLine(int lines = 1);

	// calls Write
	void operator<<(int num);
	void operator<<(char c);
	void operator<<(char * str);

	// Get mouse position
	POINT GetMousePosition();
	int GetMouseXPos();
	int GetMouseYPos();

	// manipulate bg color
	int GetBackgroundColor();
	void SetBackgroundColor(int color);
	void SetBackgroundColorV(int color);

	// manipulate text color
	int GetTextColor();
	void SetTextColor(int color);

	// manipulate console title
	void SetTitle(char * title);
	char * GetTitle();

	// Manipulate console cursor
	COORD GetCursorPosition();
	int GetCursorXPos();
	int GetCursorYPos();
	void SetCursorPositionRel(int newX, int newY);
	void SetCursorPositionAbs(int newX, int newY);

	// refresh screen with output buffer
	void DrawScreen(bool cursorAtEnd = true);

	// fun stuff
	void FlashBackgroundColors(int colors[], int numColors, int timePerColor, int cycles);

private:
	ConsoleManager();
	COORD m_size;
	char * m_title;
	WORD m_att;
	CONSOLE_SCREEN_BUFFER_INFO m_oldConsole;
	int m_fillColor;
	CHAR_INFO * m_buffer;
	int m_bufferEnd;
	int m_bufferPos;
	static int m_instances;
	COORD m_lastMouseClick;
};


#endif