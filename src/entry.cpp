#include "entry.h"
#include "git_sha1.h"

#include <yaoosl/runtime/yaoosl.h>
#include <yaoosl/runtime/yaoosl_code.h>
#include <yaoosl/runtime/yaoosl_util.h>
#include <tclap/CmdLine.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

static int console_width()
{
#if _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
	{
		return 80;
	}
	columns = csbi.srWindow.Right - csbi.srWindow.Left;
	return columns;
#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_row;
#endif
}

static bool prompt_yesno(std::string str)
{
	std::cout << str << "([y] = yes, [n] = no)" << std::endl;
	char c = '\0';
	while (c != 'y' && c != 'n')
	{
		c = std::getchar();
	}
	return c == 'y';
}


#if defined(_WIN32) && defined(_DEBUG)
int main_wrapped(int argc, char** argv);
int main(int argc, char** argv)
{
	// _crtBreakAlloc = 22621;
	main_wrapped(argc, argv);

#if defined(_WIN32) && defined(_DEBUG)
	_CrtMemDumpAllObjectsSince(NULL);
#endif
}
int main_wrapped(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
	TCLAP::CmdLine cmd("Simple console for yaoosl code execution.", ' ', std::string{ VERSION_FULL } +" (" + g_GIT_SHA1 + ")");
	TCLAP::MultiArg<std::string> inputArg("i", "input", "Loads provided yaoosl file from disk. Will be executed before actual input.", false, "PATH");
	cmd.add(inputArg);

	TCLAP::SwitchArg automatedFlag("a", "automated", "Will disable prompts.");
	cmd.add(automatedFlag);

	TCLAP::SwitchArg noWelcomeFlag("", "no-welcome", "Disables welcome print.");
	cmd.add(noWelcomeFlag);

	TCLAP::SwitchArg debugSymbolsFlag("d", "debug-symbols", "Enables debug symbols in generated bytecode.");
	cmd.add(debugSymbolsFlag);

	TCLAP::SwitchArg verboseFlag("v", "verbose", "Enables additional output of the CLI.");
	cmd.add(verboseFlag);

	cmd.getArgList().reverse();
	cmd.parse(argc, argv);


	auto vm = yaoosl_create_virtualmachine();
	std::chrono::time_point<std::chrono::steady_clock> start;
	bool fail_flag = false;
	bool v = verboseFlag.getValue();
	std::vector<yaooslcodehandle> codehandles;
	for (auto& input : inputArg.getValue())
	{
		if (v) { std::cout << "Parsing '" << input << "' started." << std::endl; start = std::chrono::high_resolution_clock::now(); }
		auto res = yaoosl_code_parse_file(input.c_str(), debugSymbolsFlag.getValue());
		if (res)
		{
			codehandles.push_back(res);
		}
		else
		{
			fail_flag = true;
		}
		if (v) { std::cout << "Parsing '" << input << "' ended after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << std::endl; }
	}
	if (fail_flag) { return -1; }
	for (size_t i = 0; i < codehandles.size(); i++)
	{
		if (v) { std::cout << "Executing '" << inputArg.getValue()[i] << "' started." << std::endl; start = std::chrono::high_resolution_clock::now(); }
		if (!yaoosl_util_execute_code(vm, codehandles[i]))
		{
			if (!automatedFlag.getValue() || !prompt_yesno("continue?"))
			{
				break;
			}
		}
		if (v) { std::cout << "Executing '" << inputArg.getValue()[i] << "' ended after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << ". "; }
		free(codehandles[i]->bytes);
		free(codehandles[i]);
		if (v) { std::cout << "With cleanup, after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << std::endl; }
	}
	if (fail_flag) { return -1; }


	if (!automatedFlag.getValue())
	{
		bool quit = false;
		while (true)
		{
			if (!noWelcomeFlag.getValue())
			{
				std::cout << "You can disable this message using `--suppress-welcome`." << std::endl <<
					"Please enter your YAOOSL code." << std::endl <<
					"To run the code, Press [ENTER] twice." << std::endl <<
					"To exit, write `quit` in a single line." << std::endl <<
					"If you enjoy this language, consider donating: https://paypal.me/X39" << std::endl;
			}

			std::string line;
			std::stringstream sstream;
			int i = 0;
			do
			{
				std::cout << std::setw(5) << i++ << "| ";
				std::getline(std::cin, line);
				sstream << line << std::endl;
				if (line == "quit")
				{
					quit = true;
					break;
				}
			} while (!line.empty());
			if (quit)
			{
				break;
			}

			// Parse input
			if (v) { std::cout << "Parsing of input started." << std::endl; start = std::chrono::high_resolution_clock::now(); }
			auto res = yaoosl_code_parse_contents(sstream.str().c_str(), false, "__console.ys");
			if (!res)
			{
				continue;
			}
			if (v) { std::cout << "Parsing of input ended after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << std::endl; }

#if _DEBUG
			auto codestring = yaoosl_code_to_string(res);
			std::cout << codestring << std::endl;
			delete[] codestring;
#endif

			// Execute input
			if (v) { std::cout << "Execution of input started." << std::endl; start = std::chrono::high_resolution_clock::now(); }
			if (yaoosl_util_execute_code(vm, res))
			{
				if (v) { std::cout << "Execution of input ended after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << ". "; }
				free(res->bytes);
				free(res);
				if (v) { std::cout << "With cleanup, after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << std::endl; }
			}
			else
			{
				if (v) { std::cout << "Execution of input failed after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << ". "; }
				free(res->bytes);
				free(res);
				if (v) { std::cout << "With cleanup, after " << (std::chrono::high_resolution_clock::now() - start) / std::chrono::milliseconds(1) << std::endl; }
			}
			std::cout << std::string(console_width(), '-') << std::endl;
		}
	}

	yaoosl_destroy_virtualmachine(vm);
}