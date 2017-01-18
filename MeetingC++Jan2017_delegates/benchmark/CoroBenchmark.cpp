#include <TestSuit.h>

#include <CoroTask.h>
#include "delegates.hpp"

#include <functional>

#ifdef NDEBUG
size_t testsize = static_cast<size_t>(1e7);
#else
size_t testsize = static_cast<size_t>(1e4);
#endif

#define TEST_LAMBDA [i](size_t val) { val += i; return val; }

template<template<typename R, typename...> class Func> void test_small(size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		Func<int(int)> fu;
		{
			fu = TEST_LAMBDA;
		}
		fu(i % 77);
	}
}

void benchmark_coro()
{
	auto fu_small = vool::make_test("std::function", test_small<std::function>);
	auto coro_small = vool::make_test("coroutine", test_small<coro_del>);
	auto small_category = vool::make_test_category("small", fu_small, coro_small);

	vool::suit_config config;
	config.persistent = true;
	config.repetitions = 1;
	config.x_res = 1280;
	config.y_res = 720;

	auto suit = vool::make_test_suit(config, small_category);
	suit.perform_categorys(0, testsize);
	suit.render_results();
}

int main()
{
	benchmark_coro();
}