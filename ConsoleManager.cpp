// TO-DO: work with floats

#include "ConsoleManager.h"

#include <ostream>

#define m_handle_in GetStdHandle( STD_INPUT_HANDLE  )
#define m_handle_out GetStdHandle( STD_OUTPUT_HANDLE )

const int MAX_STRING_LENGTH = 1024;

int ConsoleManager::m_instances = 0;

ConsoleManager::ConsoleManager()
{
	// store old console attributes to return to a default state
	GetConsoleScreenBufferInfo(m_handle_out, &m_oldConsole);
	m_att = m_oldConsole.wAttributes;
	m_size = m_oldConsole.dwSize;
	m_fillColor = (m_att & 0xF0) >> 4;

	// no text currently in buffer, so we're at point zero
	m_bufferEnd = 0;
	m_bufferPos = 0;

	// fill the console's buffer with null
	CHAR_INFO defaultChar;
	defaultChar.Attributes = BLACK;
	defaultChar.Char.UnicodeChar = '\0';
	defaultChar.Char.AsciiChar = ' ';

	m_buffer = new CHAR_INFO [m_size.X * m_size.Y];
	for(int x = 0; x < m_size.X * m_size.Y; x++)
		m_buffer[x] = defaultChar;

	// get current title and allocate space for storage
	TCHAR  sStary  [MAX_STRING_LENGTH];
	GetConsoleTitle( sStary, sizeof(sStary) / sizeof(TCHAR) );

	m_title = new char[MAX_STRING_LENGTH];

	// fill m_title with current string
	int i = 0;
	for(i; i < MAX_STRING_LENGTH && sStary[i] != 0; i++)
		m_title[i] = (char)sStary[i];
	m_title[i] = '\0';
}

ConsoleManager & ConsoleManager::GetInstance()
{
	// will only ever be created once
	static ConsoleManager m_instance;

	if(m_instances == 0)
		m_instances++;

	return m_instance;
}

ConsoleManager::~ConsoleManager()
{
	// on destruction of the ConsoleManager object,
	// restore old console text color/background color
	SetConsoleTextAttribute(m_handle_out, m_oldConsole.wAttributes);

	if(m_instances != 0)
	{
		// if a title was allocated, delete it
		if(m_title != nullptr)
		{
			delete [] m_title;
			m_title = nullptr;
		}

		// delete the console buffer
		if(m_buffer != nullptr)
			delete [] m_buffer;

		m_instances--;
	}
}

int ConsoleManager::GetBackgroundColor()
{
	return m_fillColor;
}

void ConsoleManager::SetBackgroundColor(int color)
{
	COORD origin = {0, 0};

	if(color < 0x10)
	{
		// set the background to every character in the buffer to color
		for(int x = 0; x < m_size.X * m_size.Y; x++)
			m_buffer[x].Attributes  = (m_buffer[x].Attributes & ~(0xF0))
			| (color << 4);

		m_fillColor = color;

		DrawScreen();
	}
}

void ConsoleManager::SetBackgroundColorV(int color)
{
	COORD origin = {0, 0};

	if(color < 0x10)
	{
		// set the background to every character in the buffer to color
		for(int x = 0; x < m_size.X * m_size.Y; x++)
			m_buffer[x].Attributes  = (m_buffer[x].Attributes & ~(0xF0))
			| (color << 4);

		DrawScreen();
		for(int x = 0; x < m_size.X * m_size.Y; x++)
			m_buffer[x].Attributes  = (m_buffer[x].Attributes & ~(0xF0))
			| (m_fillColor << 4);
	}
}

int ConsoleManager::GetTextColor()
{
	return m_att & 0x0F;
}

void ConsoleManager::SetTextColor(int color)
{
	if(color < 0x10)
	{
		m_att = (m_att & ~(0x0F)) | color;
		m_att |= color;
		SetConsoleTextAttribute(m_handle_out, m_att);
	}
}

char * ConsoleManager::GetTitle()
{
	return m_title;
}

void ConsoleManager::SetTitle(char * title)
{
	if(strlen(title) < MAX_STRING_LENGTH)
	{
		if(m_title != nullptr)
			delete [] m_title;

		m_title = new char[strlen(title) + 1];

		unsigned int i = 0;
		for(i; i < strlen(title); i++)
			m_title[i] = title[i];

		m_title[i] = '\0';

		LPCWSTR str;
		WCHAR str2[1024];

		MultiByteToWideChar(0, 0, m_title, 1024, str2, 1024);
		str = str2;

		SetConsoleTitle(str);
	}
}

POINT ConsoleManager::GetMousePosition()
{
	POINT pos;

	GetCursorPos(&pos);

	return pos;
}

void ConsoleManager::SetCursorPositionRel(int newX, int newY)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD newPos;

	GetConsoleScreenBufferInfo(m_handle_out, &info);

	newPos.X = info.dwCursorPosition.X + newX;
	newPos.Y = info.dwCursorPosition.Y + newY;

	m_bufferPos = newPos.X + newPos.Y * m_size.X;
	SetConsoleCursorPosition(m_handle_out, newPos);
}

void ConsoleManager::SetCursorPositionAbs(int newX, int newY)
{
	COORD newPos;

	newPos.X = newX;
	newPos.Y = newY;

	m_bufferPos = newPos.X + newPos.Y * m_size.X;

	SetConsoleCursorPosition(m_handle_out, newPos);
}

void ConsoleManager::Write(char * str, int color)
{
	AddToBuffer(str, color);
	DrawScreen();
}

void ConsoleManager::Write(char c, int color)
{
	AddToBuffer(c, color);
	DrawScreen();
}

void ConsoleManager::Write(bool b, int color)
{
	AddToBuffer(b);
	DrawScreen();
}

void ConsoleManager::Write(int x, int color)
{
	AddToBuffer(x, color);
	DrawScreen();
}

void ConsoleManager::DrawScreen(bool cursorAtEnd)
{
	SMALL_RECT rect = {0, 0, m_size.X, m_size.Y};
	COORD origin = {0, 0};
	WriteConsoleOutput(m_handle_out, m_buffer, m_size, origin, &rect);
	FlushConsoleInputBuffer(m_handle_in);
	if(cursorAtEnd)
		SetCursorPositionAbs(m_bufferEnd % m_size.X, m_bufferEnd / m_size.X);
}

void ConsoleManager::WriteLine(char * str, int color)
{	
	AddToBuffer(str, color);
	AddBreakToBuffer();
	DrawScreen();
}

void ConsoleManager::WriteLine(char c, int color)
{	
	AddToBuffer(c, color);
	AddBreakToBuffer();
	DrawScreen();
}

void ConsoleManager::WriteLine(int x, int color)
{	
	AddToBuffer(x, color);
	AddBreakToBuffer();
	DrawScreen();
}

void ConsoleManager::operator<<(int num)
{
	char buffer[256];
	Write(_itoa(num, buffer, 10));
}

void ConsoleManager::operator<<(char * str)
{
	Write(str);
}

void ConsoleManager::operator<<(char c)
{
	Write(&c);
}

int ConsoleManager::GetHeight()
{
	return m_size.Y;
}

// sorta working
void ConsoleManager::SetHeightAbs(int y)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD size;
	HWND console = GetConsoleWindow();
	RECT ConsoleRect;

	GetConsoleScreenBufferInfo(m_handle_out, &info);
	size.X = info.srWindow.Right;
	size.Y = y;
	m_size = size;

	GetWindowRect(console, &ConsoleRect); 
	MoveWindow(console, ConsoleRect.left, ConsoleRect.top, ConsoleRect.right - 1, ConsoleRect.bottom - 1, TRUE);
}

// sorta working
void ConsoleManager::SetHeightRel(int y)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD size;
	HWND console = GetConsoleWindow();
	RECT ConsoleRect;

	GetConsoleScreenBufferInfo(m_handle_out, &info);
	size.X = info.srWindow.Right;
	size.Y = info.srWindow.Bottom + y;
	m_size = size;

	GetWindowRect(console, &ConsoleRect); 
	MoveWindow(console, ConsoleRect.left, ConsoleRect.top, ConsoleRect.right, ConsoleRect.bottom, TRUE);
}

void ConsoleManager::ClearText()
{
	CHAR_INFO nullChar;
	nullChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	nullChar.Char.AsciiChar = '\0';
	nullChar.Char.UnicodeChar = '\0';

	for(int x = 0; x < m_bufferEnd; x++)
		m_buffer[x] = nullChar;

	DrawScreen();
	SetCursorPositionAbs(0, 0);
	m_bufferPos = 0;
	m_bufferEnd = 0;
}

int ConsoleManager::GetMouseXPos()
{
	POINT pos;
	GetCursorPos(&pos);

	return pos.y;
}

int ConsoleManager::GetMouseYPos()
{
	POINT pos;
	GetCursorPos(&pos);

	return pos.x;
}

int ConsoleManager::GetCursorXPos()
{
	return GetCursorPosition().X;
}

int ConsoleManager::GetCursorYPos()
{
	return GetCursorPosition().Y;
}

COORD ConsoleManager::GetCursorPosition()
{
	CONSOLE_SCREEN_BUFFER_INFO info;

	GetConsoleScreenBufferInfo(m_handle_out, &info);

	return info.dwCursorPosition;
}

void ConsoleManager::newLine(int lines)
{
	AddBreakToBuffer(lines);
	SetCursorPositionAbs(0, GetCursorYPos());
	SetCursorPositionRel(0, lines);
}

int ConsoleManager::WaitForKeyPressAny(char * waitStr)
{
	bool foundPress = false;
	int x = 0;

	if(waitStr != nullptr)
		Write(waitStr);

	while(!foundPress)
	{
		for(x = 0; x < 256; x++)
			if((char)(GetAsyncKeyState(x) >> 8))
				foundPress = true;
		Sleep(1);
	}

	return x;
}

void ConsoleManager::WaitForKeyPress(char key)
{
	key = toupper(key);

	while(!((char)GetAsyncKeyState(key)))
		Sleep(1);
}

int ConsoleManager::WaitForKeyPress(char * keys)
{
	keys = strupr(keys);
	bool found = false;
	int len = strlen(keys);
	int whichKey = -1;

	while(!found)
	{
		for(int x = 0; x < len && !found; x++)
		{
			if((char)(GetAsyncKeyState(keys[x]) >> 8))
			{
				found = true;
				whichKey = x;
			}
		}
	}

	return whichKey;
}

int ConsoleManager::IsArrowOrKeyPressed(char * chars)
{
	int pressed = IsArrowKeyPressed();

	if(pressed == -1)
		pressed = IsKeyPressed(chars);

	return pressed;
}

int ConsoleManager::IsKeyPressed(char * chars)
{
	chars = strupr(chars);
	bool found = false;
	int len = strlen(chars);
	int whichKey = -1;
	unsigned int status = 0;

	for(int x = 0; x < len && !found; x++)
	{
		GetAsyncKeyState(chars[x]);
		if(IsKeyPressed(chars[x]))
		{
			found = true;
			whichKey = x;
		}
	}

	return whichKey;
}

bool ConsoleManager::IsKeyPressed(int key)
{
	unsigned int status = GetAsyncKeyState(key);

	return (((status >> 8) > 0 || (status & 1) > 0));
}

int ConsoleManager::IsArrowKeyPressed()
{
	bool found = false;
	int whichKey = -1;

	for(int x = 0x25; x < 0x29 && !found; x++)
	{
		if((char)(GetAsyncKeyState(x) >> 8))
		{
			found = true;
			whichKey = x;
		}
	}

	return whichKey;
}

void ConsoleManager::WaitForLeftMouseClick()
{
	while(!IsLeftMouseClicked());
}

void ConsoleManager::WaitForRightMouseClick()
{
	while(!IsRightMouseClicked());
}

bool ConsoleManager::IsLeftMouseClicked()
{
	bool found = false;
	INPUT_RECORD info;

	DWORD len = 1;
	DWORD read = 0;

	ReadConsoleInput(m_handle_in, &info, len, &read);
	if(info.Event.MouseEvent.dwButtonState == VK_LBUTTON)
	{
		found = true;
		m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
	}

	return found;
}

bool ConsoleManager::IsRightMouseClicked()
{
	bool found = false;
	INPUT_RECORD info;

	DWORD len = 1;
	DWORD read = 0;

	ReadConsoleInput(m_handle_in, &info, len, &read);
	if(info.Event.MouseEvent.dwButtonState == VK_RBUTTON)
	{
		found = true;
		m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
	}

	return found;
}

bool ConsoleManager::IsLeftMouseReleased()
{
	bool found = false;
	INPUT_RECORD info;

	DWORD len = 1;
	DWORD read = 0;

	ReadConsoleInput(m_handle_in, &info, len, &read);
	if(info.Event.MouseEvent.dwButtonState != VK_LBUTTON)
	{
		found = true;
		m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
	}

	return found;
}

bool ConsoleManager::IsRightMouseReleased()
{
	bool found = false;
	INPUT_RECORD info;

	DWORD len = 1;
	DWORD read = 0;

	ReadConsoleInput(m_handle_in, &info, len, &read);
	if(info.Event.MouseEvent.dwButtonState != VK_RBUTTON)
	{
		found = true;
		m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
	}

	return found;
}

void ConsoleManager::WaitForAnyMouseClick(bool & rl)
{
	bool found = false;
	INPUT_RECORD info;

	DWORD len = 1;
	DWORD read = 0;

	while(!found)
	{
		ReadConsoleInput(m_handle_in, &info, len, &read);
		if(info.Event.MouseEvent.dwButtonState == VK_LBUTTON)
		{
			found = true;
			m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
			rl = LEFT;
		}
		else if(info.Event.MouseEvent.dwButtonState == VK_RBUTTON)
		{
			found = true;
			m_lastMouseClick = info.Event.MouseEvent.dwMousePosition;
			rl = RIGHT;
		}
	}
}

void ConsoleManager::WaitForAnyMouseRelease(bool & rl)
{
	bool found = false;

	DWORD len = 1;
	DWORD read = 0;

	WaitForAnyMouseClick(rl);

	if(rl == RIGHT)
	{
		WaitForRightMouseRelease();
	}
	else
	{
		WaitForLeftMouseRelease();
	}
}

void ConsoleManager::AddToBuffer(char c, int color, int times)
{
	CHAR_INFO curChar;
	if(color == -1)
		curChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	else
		curChar.Attributes = GetBackgroundColor() << 4 | color;
	curChar.Char.UnicodeChar = c;
	curChar.Char.AsciiChar = c;
	for(int x = 0; x < times; x++)
	{
		if(curChar.Char.AsciiChar == '\n')
		{
			AddBreakToBuffer();
		}
		else
		{
			m_buffer[m_bufferPos] = curChar;
			m_bufferPos++;
		}

		if(m_bufferPos > m_bufferEnd)
			m_bufferEnd = m_bufferPos;
	}
}

void ConsoleManager::AddToBuffer(bool b, int color)
{
	CHAR_INFO curChar;
	if(color == -1)
		curChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	else
		curChar.Attributes = GetBackgroundColor() << 4 | color;

	if(b)
	{
		curChar.Char.UnicodeChar = '1';
		curChar.Char.AsciiChar = '1';
	}
	else
	{
		curChar.Char.UnicodeChar = '0';
		curChar.Char.AsciiChar = '0';
	}

	m_buffer[m_bufferPos] = curChar;

	m_bufferPos++;

	if(m_bufferPos > m_bufferEnd)
		m_bufferEnd = m_bufferPos;
}

void ConsoleManager::AddToBuffer(char * c, int color)
{
	CHAR_INFO curChar;
	if(color == -1)
		curChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	else
		curChar.Attributes = GetBackgroundColor() << 4 | color;

	for(unsigned int x = 0; x < strlen(c); x++)
	{
		curChar.Char.UnicodeChar = c[x];
		curChar.Char.AsciiChar = c[x];

		if(curChar.Char.AsciiChar == '\n')
		{
			AddBreakToBuffer();
		}
		else
		{
			m_buffer[m_bufferPos] = curChar;
			m_bufferPos++;
		}
	}

	if(m_bufferPos > m_bufferEnd)
		m_bufferEnd = m_bufferPos;
}

void ConsoleManager::AddToBufferCentered(char * c, int color)
{
	CHAR_INFO curChar;
	if(color == -1)
		curChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	else
		curChar.Attributes = GetBackgroundColor() << 4 | color;

	AddToBuffer(' ', -1, (m_size.X - strlen(c)) / 2);

	for(unsigned int x = 0; x < strlen(c); x++)
	{
		curChar.Char.UnicodeChar = c[x];
		curChar.Char.AsciiChar = c[x];
		m_buffer[m_bufferPos + x] = curChar;
	}

	m_bufferPos += strlen(c);

	if(m_bufferPos > m_bufferEnd)
		m_bufferEnd = m_bufferPos;
}

void ConsoleManager::AddToBufferCentered(int c, int color)
{
	char buffer[256];
	AddToBufferCentered(itoa(c, buffer, 10), color);
}

void ConsoleManager::AddToBufferCentered(char c, int color)
{
	CHAR_INFO curChar;
	if(color == -1)
		curChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	else
		curChar.Attributes = GetBackgroundColor() << 4 | color;

	AddToBuffer(' ', -1, (m_size.X - 1) / 2);

	curChar.Char.UnicodeChar = c;
	curChar.Char.AsciiChar = c;
	m_buffer[m_bufferPos] = curChar;

	m_bufferPos ++;

	if(m_bufferPos > m_bufferEnd)
		m_bufferEnd = m_bufferPos;
}

void ConsoleManager::WriteCentered(char * c, int color)
{
	AddToBufferCentered(c, color);
	DrawScreen();
}

void ConsoleManager::WriteCentered(char c, int color)
{
	AddToBufferCentered(c, color);
	DrawScreen();
}

void ConsoleManager::WriteCentered(int c, int color)
{
	AddToBufferCentered(c, color);
	DrawScreen();
}

void ConsoleManager::AddToBuffer(int num, int color)
{
	char buffer[256];
	AddToBuffer(_itoa(num, buffer, 10), color);
}

void ConsoleManager::AddBreakToBuffer(int lines)
{
	CHAR_INFO nullChar;
	bool endOfLine = false;
	nullChar.Attributes = (GetBackgroundColor() << 4) | GetTextColor();
	nullChar.Char.AsciiChar = '\0';
	nullChar.Char.UnicodeChar = '\0';
	int x = 0;

	while(x < lines)
	{
		do
		{
			m_buffer[m_bufferPos] = nullChar;
			m_bufferPos++;
		}
		while(m_bufferPos % m_size.X != 0);
		x++;
	}

	if(m_bufferPos > m_bufferEnd)
		m_bufferEnd = m_bufferPos;
}

void ConsoleManager::HideCursor()
{
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(m_handle_out, &info);
	info.bVisible = false;
	SetConsoleCursorInfo(m_handle_out, &info);
}

void ConsoleManager::ShowCursor()
{
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(m_handle_out, &info);
	info.bVisible = true;
	SetConsoleCursorInfo(m_handle_out, &info);
}

void ConsoleManager::FlashBackgroundColors(int colors[], int numColors, int timePerColor, int cycles)
{
	int oldColor = GetBackgroundColor();
	for(int x = 0; x < cycles; x++)
	{
		for(int y = 0; y < numColors; y++)
		{
			SetBackgroundColor(colors[y]);
			Sleep(timePerColor);
		}
	}

	SetBackgroundColor(oldColor);
}

void ConsoleManager::GetUserInput(char & c, bool line)
{
	INPUT_RECORD info;
	DWORD len = 1;
	DWORD read = 0;
	bool firstChar = true;
	int presses = 0;
	int totChars = 0;
	int start = m_bufferPos;
	CHAR_INFO defaultChar;
	defaultChar.Attributes = m_att;
	defaultChar.Char.UnicodeChar = '\0';
	defaultChar.Char.AsciiChar = '\0';

	FlushConsoleInputBuffer(m_handle_in);

	do
	{
		ReadConsoleInputA(m_handle_in, &info, len, &read);

		if(info.Event.KeyEvent.uChar.AsciiChar >= ' ' &&
			info.Event.KeyEvent.uChar.AsciiChar <= '~'
			&& info.Event.KeyEvent.bKeyDown)
		{
			if(m_bufferEnd == m_bufferPos)
				totChars++;
			if(presses == 0)
			{
				AddToBuffer(info.Event.KeyEvent.uChar.AsciiChar);
				c = info.Event.KeyEvent.uChar.AsciiChar;
			}
			presses++;
			DrawScreen(false);
			SetCursorPositionRel(1, 0);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_BACK
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferPos != start)
		{
			presses--;
			totChars--;
			m_buffer[m_bufferPos - 1] = defaultChar;

			if(m_bufferPos == m_bufferEnd)
				m_bufferPos = m_bufferEnd = m_bufferPos - 1;
			else
				m_bufferPos--;

			SetCursorPositionRel(-1, 0);

			for(int x = m_bufferPos; x < m_bufferEnd; x++)
				m_buffer[x] = m_buffer[x + 1];

			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_DELETE
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferEnd != m_bufferPos)
		{
			presses--;
			totChars--;
			if(m_bufferEnd == m_bufferPos)
				m_bufferPos = m_bufferEnd = m_bufferEnd - 1;

			for(int x = m_bufferPos; x < m_bufferEnd; x++)
				m_buffer[x] = m_buffer[x + 1];

			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_LEFT
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferPos != start)
		{
			presses--;
			m_bufferPos--;
			SetCursorPositionRel(-1, 0);
			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferEnd != m_bufferPos)
		{
			presses++;
			m_bufferPos++;
			SetCursorPositionRel(1, 0);

		}

		Sleep(5);
	}
	while(info.Event.KeyEvent.wVirtualKeyCode != VK_RETURN);

	while(IsKeyPressed(VK_RETURN));

	SetCursorPositionAbs(GetCursorXPos() + (totChars - presses), GetCursorYPos());

	m_bufferPos += totChars - presses;

	if(line)
	{
		AddBreakToBuffer();
		SetCursorPositionRel(0, 1);
	}

	DrawScreen();
	FlushConsoleInputBuffer(m_handle_in);
}

void ConsoleManager::GetUserInput(char str[], bool line)
{
	INPUT_RECORD info;
	DWORD len = 1;
	DWORD read = 0;
	bool firstChar = true;
	int presses = 0;
	int totChars = 0;
	int start = m_bufferPos;
	CHAR_INFO defaultChar;
	defaultChar.Attributes = m_att;
	defaultChar.Char.UnicodeChar = '\0';
	defaultChar.Char.AsciiChar = '\0';

	FlushConsoleInputBuffer(m_handle_in);

	do
	{
		ReadConsoleInputA(m_handle_in, &info, len, &read);

		if(info.Event.KeyEvent.uChar.AsciiChar >= ' ' &&
			info.Event.KeyEvent.uChar.AsciiChar <= '~'
			&& info.Event.KeyEvent.bKeyDown)
		{
			if(m_bufferEnd == m_bufferPos)
				totChars++;

			InsertInBuffer(info.Event.KeyEvent.uChar.AsciiChar);
			str[presses] = info.Event.KeyEvent.uChar.AsciiChar;
			presses++;
			DrawScreen(false);
			SetCursorPositionRel(1, 0);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_BACK
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferPos != start)
		{
			presses--;
			totChars--;
			m_buffer[m_bufferPos - 1] = defaultChar;

			if(m_bufferPos == m_bufferEnd)
				m_bufferPos = m_bufferEnd = m_bufferPos - 1;
			else
				m_bufferPos--;

			SetCursorPositionRel(-1, 0);

			for(int x = m_bufferPos; x < m_bufferEnd; x++)
				m_buffer[x] = m_buffer[x + 1];

			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_DELETE
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferEnd != m_bufferPos)
		{
			presses--;
			totChars--;
			if(m_bufferEnd == m_bufferPos)
				m_bufferPos = m_bufferEnd = m_bufferEnd - 1;

			for(int x = m_bufferPos; x < m_bufferEnd; x++)
				m_buffer[x] = m_buffer[x + 1];

			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_LEFT
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferPos != start)
		{
			presses--;
			m_bufferPos--;
			SetCursorPositionRel(-1, 0);
			DrawScreen(false);
		}
		else if(info.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT
			&& info.Event.KeyEvent.bKeyDown
			&& m_bufferEnd != m_bufferPos)
		{
			presses++;
			m_bufferPos++;
			SetCursorPositionRel(1, 0);

		}

		Sleep(5);
	}
	while(info.Event.KeyEvent.wVirtualKeyCode != VK_RETURN);

	str[totChars] = '\0';

	while(IsKeyPressed(VK_RETURN));

	SetCursorPositionAbs(GetCursorXPos() + (totChars - presses), GetCursorYPos());

	m_bufferPos += totChars - presses;

	if(line)
	{
		AddBreakToBuffer();
		SetCursorPositionRel(0, 1);
	}

	DrawScreen();
	FlushConsoleInputBuffer(m_handle_in);
}

void ConsoleManager::GetUserInput(int & x, bool line)
{
	char buffer[256];
	GetUserInput(buffer, line);

	x = atoi(buffer);
}

COORD ConsoleManager::GetLastMouseClick()
{
	return m_lastMouseClick;
}

bool ConsoleManager::IsAnyMouseClicked()
{
	return (IsLeftMouseClicked() || IsRightMouseClicked());
}

void ConsoleManager::WaitForRightMouseRelease()
{
	while(!IsRightMouseReleased());
}

void ConsoleManager::WaitForLeftMouseRelease()
{
	while(!IsLeftMouseReleased());
}

void ConsoleManager::InsertInBuffer(char c)
{
	CHAR_INFO tempChar;
	tempChar.Char.AsciiChar = '\0';

	for(int x = m_bufferPos + 1; x < m_bufferEnd; x++)
	{
		tempChar = m_buffer[x];
		m_buffer[x] = m_buffer[x-1];
		m_buffer[x-1] = tempChar;
	}
	
	m_buffer[m_bufferEnd] = tempChar;

	tempChar.Attributes = m_att;
	tempChar.Char.AsciiChar = c;
	tempChar.Char.UnicodeChar = c;
	m_buffer[m_bufferPos] = tempChar;

	m_bufferEnd++;
}