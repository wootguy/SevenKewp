#pragma once
#include <extdll.h>
#include <mutex>

class ThreadSafeInt {
public:
	EXPORT int getValue();
	EXPORT void setValue(int value);

	EXPORT void inc();
	EXPORT void dec();

private:
	std::mutex mutex;
	int value;
};