#include "Timer.h"

using namespace std::chrono;

Timer::Timer()
{
	last = steady_clock::now();
}

float Timer::Mark()
{
	const auto old = last;
	last = steady_clock::now();

	const duration<float> elapsed = last - old;
	return elapsed.count();
}

float Timer::Peek() const
{
	const auto now = steady_clock::now();
	const duration<float> elapsed = now - last;
	return elapsed.count();
}
