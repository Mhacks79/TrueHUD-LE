// fakegame.h - the reusable half of an offline SKSE LE test bench.
//
// A plugin for this platform ends every sentence in one of two places: a raw pointer
// into the game's memory, or a call through a virtual table. Neither can be checked
// by reading a log, and both end the session when they are wrong. So the plugin's own
// code is built against a fake Skyrim instead: the addresses it calls are reserved
// inside this process, stubs are written at them, a world of known shape is put in
// front of it, and what it decides is measured.
//
// This header is everything that is the SAME for every plugin. What is different -
// the stubs, the structures, the world, the tests - goes in your own file.
//
//   #include "../core/fakegame.h"
//
//   static const unsigned long kClaimedBlocks[] = { 0x00450000, ... };   // yours
//   FAKEGAME_CLAIM(kClaimedBlocks)
//
//   int main() { return FakeGame::Run(BuildMyFakeGame, RunMyTests); }
//
// Build it OUTSIDE the game's address range or it will overwrite itself:
//
//   cl /EHsc /W3 /MT ... /link /DYNAMICBASE:NO /BASE:0x30000000 /FIXED
//
// 32-bit, like the game.

#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

namespace FakeGame
{
	// ---------------------------------------------------------------- results

	extern int checks;
	extern int failures;
	extern int stubFailures;

	// Every assertion goes through this, so a run ends with one number that means
	// something.
	void Check(bool condition, const char * what, ...);

	bool Near(float a, float b, float tolerance = 0.05f);

	// ANYTHING HANDED TO HAVOK MUST SIT ON A MULTIPLE OF SIXTEEN.
	//
	// The engine moves its vectors with instructions that fault outright otherwise,
	// and a 32-bit stack local is aligned to four. Call this from every stub that
	// receives a structure from the plugin. It is the single highest-value check in
	// here: it catches, on every cast, the bug that took a mod's players down on
	// every load for three releases. See rules/FATAL.md F1.
	void CheckAligned(const void * address, const char * what);

	// Call from the deepest stub. Run reports the worst case at the end: an engine
	// worker thread's stack is whatever the engine gave it, and a search that nests
	// kilobyte structures adds up faster than it looks.
	void NoteStackDepth();

	// --------------------------------------------------------------- the pages

	// True when this address belongs to the harness itself rather than to the fake
	// game. It must never be written to: a stub there overwrites THIS PROGRAM, and
	// the symptom is a crash somewhere else entirely. Linking at 0x30000000 is what
	// prevents it; this is what notices when that was forgotten.
	bool InsideThisProgram(unsigned long address);

	bool EnsurePage(unsigned long address, size_t length);

	// A five byte jmp at a game address, so the plugin's call lands in your stub.
	void WriteStub(unsigned long address, const void * target);
	void WritePointer(unsigned long address, const void * value);
	void WriteFloat(unsigned long address, float value);

	// A PE header at 0x00400000, so a plugin that tells an engine object from a
	// stray value by looking at where its vtable lives gets a sensible answer.
	// WITHOUT THIS, a validated read layer rejects every fake object and the whole
	// suite passes by doing nothing. Called by Run.
	void WriteFakeImageHeader(unsigned long imageSize = 0x01C00000);

	// ------------------------------------------------------------- the process

	// The claimed blocks, declared by your file through FAKEGAME_CLAIM.
	extern const unsigned long *	claimedBlocks;
	extern int						claimedBlockCount;
	extern int						claimed;

	// Claims them in the child, before the C runtime can put anything there. Used by
	// the TLS callback that FAKEGAME_CLAIM installs; you do not call it.
	void ClaimFromInsideTheChild();

	// The whole run: stakes out a suspended copy of this process, lets it go, and in
	// the child builds the fake game and runs the tests. Returns a process exit code.
	//
	// The two-process dance is not decoration. A process cannot reliably reserve the
	// game's address range inside itself - by the time main() runs, the C runtime has
	// already put its heap down there.
	int Run(void (*build)(), void (*tests)());
}

// Installs the TLS callback that claims the blocks before the runtime starts, and
// points the core at your list. Put it at file scope, once.
#define FAKEGAME_CLAIM(blocks)                                                        \
	static void NTAPI FakeGame_ClaimCallback(PVOID, DWORD reason, PVOID)              \
	{                                                                                 \
		if(reason != DLL_PROCESS_ATTACH) return;                                      \
		FakeGame::claimedBlocks     = (blocks);                                       \
		FakeGame::claimedBlockCount = (int)(sizeof(blocks) / sizeof((blocks)[0]));     \
		FakeGame::ClaimFromInsideTheChild();                                          \
	}                                                                                 \
	_Pragma("comment(linker, \"/INCLUDE:__tls_used\")")                               \
	_Pragma("comment(linker, \"/INCLUDE:_p_fakegame_claim\")")                         \
	_Pragma("section(\".CRT$XLB\", long, read)")                                      \
	extern "C" __declspec(allocate(".CRT$XLB"))                                        \
	const PIMAGE_TLS_CALLBACK p_fakegame_claim = FakeGame_ClaimCallback;
