#include "ConsoleManager.h"

#include <iostream>

using std::cout;
using std::cin;

int main()
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	ConsoleManager window = ConsoleManager::GetInstance();

	int age;
	char init;
	char fname[256];
	char lname[256];
	int colors[] = {YELLOW, DARK_GREEN};

	window << "Please enter your first name: ";
	window.GetUserInput(fname);
	window << "Please enter your middle initial: ";
	window.GetUserInput(init);
	window << "Please enter your last name: ";
	window.GetUserInput(lname);
	window << "Please enter your age: ";
	window.GetUserInput(age, false);

	window << "\n\nYour name is ";
	window << fname;
	window << " ";
	window << init;
	window << ".";
	window << " ";
	window << lname;
	window << " and you are ";
	window << age;
	window << " years old.\n\n";

	Sleep(500);

	window.FlashBackgroundColors(colors, 2, 17, 20);

	window.WaitForKeyPressAny();

	return 0;
}