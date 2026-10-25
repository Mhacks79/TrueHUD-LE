// fakegame.cpp - see fakegame.h.
//
// Extracted from the bench that found four shipped crashes, so that the next plugin
// does not have to rewrite it. Nothing in here knows anything about any particular
// plugin.

#include "fakegame.h"

namespace FakeGame
{
	int checks = 0;
	int failures = 0;
	int stubFailures = 0;

	const unsigned long *	claimedBlocks = NULL;
	int						claimedBlockCount = 0;
	int						claimed = 0;

	namespace
	{
		unsigned long	s_unclaimed[64];
		int				s_unclaimedCount = 0;
		unsigned long	s_deepestStack = 0xFFFFFFFF;
		unsigned long	s_entryStack = 0;

		// Anything that goes wrong before main() would otherwise be a silent exit, so
		// this is installed first and writes what happened with no help from the C
		// runtime, which is not up yet.
		LONG CALLBACK ReportFault(PEXCEPTION_POINTERS info)
		{
			char message[256];
			const int length = wsprintfA(message, "\r\nfault %08X at %08X\r\n",
										 info->ExceptionRecord->ExceptionCode,
										 (unsigned long)info->ExceptionRecord->ExceptionAddress);
			DWORD written = 0;
			WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, length, &written, NULL);
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}

	// ------------------------------------------------------------------ results

	void Check(bool condition, const char * what, ...)
	{
		++checks;
		if(condition) return;

		++failures;

		char message[512];
		va_list args;
		va_start(args, what);
		vsnprintf(message, sizeof(message), what, args);
		va_end(args);

		printf("  FAIL  %s\n", message);
	}

	bool Near(float a, float b, float tolerance)
	{
		return fabsf(a - b) <= tolerance;
	}

	void CheckAligned(const void * address, const char * what)
	{
		Check(((unsigned long)address & 15) == 0,
			  "%s is 16-byte aligned (it is at %08lX, which is %lu past)",
			  what, (unsigned long)address, (unsigned long)address & 15);
	}

	void NoteStackDepth()
	{
		volatile unsigned char here = 0;
		const unsigned long esp = (unsigned long)&here;
		if(esp < s_deepestStack) s_deepestStack = esp;
	}

	// -------------------------------------------------------------------- pages

	bool InsideThisProgram(unsigned long address)
	{
		const unsigned char * base = (const unsigned char *)GetModuleHandleA(NULL);
		if(!base) return false;

		const unsigned long pe = *(const unsigned long *)(base + 0x3C);
		const unsigned long size = *(const unsigned long *)(base + pe + 0x18 + 0x38);
		return address >= (unsigned long)base && address < (unsigned long)base + size;
	}

	bool EnsurePage(unsigned long address, size_t length)
	{
		if(InsideThisProgram(address))
		{
			printf("  note: %08lX is inside the harness itself - refusing to write there.\n",
				   address);
			printf("        Link with /BASE:0x30000000 /FIXED.\n");
			return false;
		}

		const unsigned long pageStart = address & ~0xFFF;
		const unsigned long pageEnd = (address + length + 0xFFF) & ~0xFFF;

		MEMORY_BASIC_INFORMATION info;
		if(VirtualQuery((void *)pageStart, &info, sizeof(info)) && info.State == MEM_COMMIT)
			return true;

		if(VirtualAlloc((void *)pageStart, pageEnd - pageStart, MEM_COMMIT, PAGE_EXECUTE_READWRITE))
			return true;

		printf("  note: %08lX could not be committed (error %lu)\n", pageStart, GetLastError());
		return false;
	}

	void WriteStub(unsigned long address, const void * target)
	{
		if(!EnsurePage(address, 5)) { ++stubFailures; return; }

		unsigned char * code = (unsigned char *)address;
		code[0] = 0xE9;
		*(unsigned long *)(code + 1) = (unsigned long)target - (address + 5);
	}

	void WritePointer(unsigned long address, const void * value)
	{
		if(!EnsurePage(address, 4)) { ++stubFailures; return; }
		*(const void **)address = value;
	}

	void WriteFloat(unsigned long address, float value)
	{
		if(!EnsurePage(address, 4)) { ++stubFailures; return; }
		*(float *)address = value;
	}

	void WriteFakeImageHeader(unsigned long imageSize)
	{
		if(!EnsurePage(0x00400000, 0x200)) { ++stubFailures; return; }

		memset((void *)0x00400000, 0, 0x200);
		*(unsigned long *)(0x00400000 + 0x3C) = 0x80;							// e_lfanew
		*(unsigned long *)(0x00400000 + 0x80 + 0x18 + 0x38) = imageSize;		// SizeOfImage
	}

	// ------------------------------------------------------------------ process

	void ClaimFromInsideTheChild()
	{
		AddVectoredExceptionHandler(1, ReportFault);

		for(int i = 0; i < claimedBlockCount; i++)
		{
			// The parent staked this range out while this process was still
			// suspended, before anything of its own could be put there.
			MEMORY_BASIC_INFORMATION info;
			if(VirtualQuery((void *)claimedBlocks[i], &info, sizeof(info)) &&
			   info.State == MEM_COMMIT && info.Type == MEM_PRIVATE)
				++claimed;
			else if(VirtualAlloc((void *)claimedBlocks[i], 0x10000,
								 MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
				++claimed;
			else if(s_unclaimedCount < 64)
				s_unclaimed[s_unclaimedCount++] = claimedBlocks[i];
		}
	}

	namespace
	{
		int StakeOutAChild()
		{
			char path[MAX_PATH];
			GetModuleFileNameA(NULL, path, MAX_PATH);

			SetEnvironmentVariableA("FAKEGAME_CHILD", "1");

			STARTUPINFOA startup = { sizeof(startup) };
			PROCESS_INFORMATION process = { 0 };
			if(!CreateProcessA(path, NULL, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL,
							   &startup, &process))
			{
				printf("could not start the child process (%lu)\n", GetLastError());
				return 3;
			}

			int taken = 0;
			for(int i = 0; i < claimedBlockCount; i++)
			{
				if(VirtualAllocEx(process.hProcess, (void *)claimedBlocks[i], 0x10000,
								  MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
					++taken;
				else
					printf("could not stake out %08lX in the child (%lu)\n",
						   claimedBlocks[i], GetLastError());
			}

			printf("staked out %d of %d game address blocks in the child\n\n",
				   taken, claimedBlockCount);

			ResumeThread(process.hThread);
#ifdef FAKEGAME_TIMEOUT_MS
			if(WaitForSingleObject(process.hProcess, FAKEGAME_TIMEOUT_MS) != WAIT_OBJECT_0)
			{
				printf("  FAIL child timeout/wait failure; failures=1 (limit %lu ms)\n", (unsigned long)FAKEGAME_TIMEOUT_MS);
				// This is only our own suspended-and-resumed test child, never TESV.
				TerminateProcess(process.hProcess, 124);
				WaitForSingleObject(process.hProcess, 5000);
				CloseHandle(process.hThread);
				CloseHandle(process.hProcess);
				return 124;
			}
#else
			WaitForSingleObject(process.hProcess, INFINITE);
#endif

			DWORD code = 3;
			GetExitCodeProcess(process.hProcess, &code);
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			return (int)code;
		}
	}

	int Run(void (*build)(), void (*tests)())
	{
		setvbuf(stdout, NULL, _IONBF, 0);		// a crash must not swallow the log

		if(!GetEnvironmentVariableA("FAKEGAME_CHILD", NULL, 0))
			return StakeOutAChild();

		printf("offline bench\n");
		printf("claimed %d of %d game address blocks\n", claimed, claimedBlockCount);
		for(int i = 0; i < s_unclaimedCount; i++)
			printf("  note: %08lX was already spoken for\n", s_unclaimed[i]);
		printf("\n");

		WriteFakeImageHeader();
		if(build) build();

		if(stubFailures)
		{
			printf("\n%d addresses could not be written: the fake game is incomplete\n",
				   stubFailures);
			return 2;
		}

		volatile unsigned char top = 0;
		s_entryStack = (unsigned long)&top;
		s_deepestStack = 0xFFFFFFFF;

		if(tests) tests();

		printf("\n%d checks, %d failures\n", checks, failures);
		if(s_deepestStack != 0xFFFFFFFF)
			printf("deepest stack reached: %lu bytes\n", s_entryStack - s_deepestStack);

		return failures ? 1 : 0;
	}
}
