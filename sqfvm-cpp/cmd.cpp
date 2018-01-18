#include "full.h"

sqf::commandmap& sqf::commandmap::get(void)
{
	static sqf::commandmap map;
	return map;
}

void sqf::commandmap::init(void)
{
	add(nullar(L"test", L"foo", [](const virtualmachine* vm) -> value_s { vm->out() << L"works"; return value_s(); }));
}
