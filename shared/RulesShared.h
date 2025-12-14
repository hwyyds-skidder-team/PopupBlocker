#pragma once
#include <windows.h>
#include <cstdint>

constexpr wchar_t kRulesMappingName[] = L"Local\\PopupBlockerRules_v1";
constexpr wchar_t kRulesMutexName[] = L"Local\\PopupBlockerRulesMutex_v1";

constexpr uint32_t kRulesMagic = 0x50424C4B; // 'PBLK'
constexpr int kMaxRules = 128;

enum class Action : uint32_t {
	None = 0,
	Hide = 1,
	RemoveTopmost = 2,
	Block = 3,
};

struct Rule {
	uint32_t enabled; // 0/1

	
	wchar_t processName[64];   // case-insensitive exact
	wchar_t className[64];     // optional, case-insensitive exact

	uint32_t styleMustHaveMask;
	uint32_t styleMustHaveValue;

	uint32_t exStyleMustHaveMask;
	uint32_t exStyleMustHaveValue;

	int32_t minW, minH;
	int32_t maxW, maxH;

	Action action;
};

struct SharedRules {
	uint32_t magic;
	volatile LONG seq;      // seqlock
	uint32_t paused;        // 0/1  (UI ¿ØÖÆ£¬hook Ö»¶Á)
	uint32_t ruleCount;
	Rule rules[kMaxRules];
};