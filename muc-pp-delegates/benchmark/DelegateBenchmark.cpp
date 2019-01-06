#include <TestSuit.h>

#include "delegate.hpp"

//#define LARGE

#ifdef NDEBUG
size_t testsize = static_cast<size_t>(1e6);
#else
size_t testsize = static_cast<size_t>(1e4);
#endif

#ifdef LARGE
#define TEST_LAMBDA [arr](size_t val) { val += arr[0]; return val; }

using dummy_t = int[10];
static constexpr size_t cap_size = sizeof(dummy_t);

void test_small_pure(size_t size)
{
	dummy_t arr;
	for (size_t i = 0; i < size; ++i)
	{
		delegate<size_t(dummy_t, size_t), spec::pure> fu;
		{
			fu = [](dummy_t arr, size_t val) { arr[0] += val; return val; };
		}
		arr[0] = i;
		fu(arr, i % 77);
	}
}

template<typename Func> void test_small(size_t size)
{
	dummy_t arr;
	for (size_t i = 0; i < size; ++i)
	{
		Func fu;
		{
			fu = TEST_LAMBDA;
		}
		arr[0] = i;
		fu(i % 77);
	}
}
#else
#define TEST_LAMBDA [i](size_t val) { val += i; return val; }

static constexpr size_t cap_size = detail::default_capacity;

void test_small_pure(size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		delegate<size_t(size_t, size_t), spec::pure> fu;
		{
			fu = [](size_t i, size_t val) { val += i; return val; };
		}
		fu(i, i % 77);
	}
}

template<typename Func> void test_small(size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		Func fu;
		{
			fu = TEST_LAMBDA;
		}
		fu(i % 77);
	}
}
#endif // LARGE

void benchmark()
{
	auto fu_small = vool::make_test("std::function",
		test_small<std::function<size_t(size_t)>>);

	auto inplace = vool::make_test("inplace",
		test_small<delegate<size_t(size_t), spec::inplace, cap_size>>);

	auto inplace_triv = vool::make_test("inplace triv",
		test_small<delegate<size_t(size_t), spec::inplace_triv, cap_size>>);

	auto pure = vool::make_test("pure", test_small_pure);

	auto small_category = vool::make_test_category(
		"small",
		fu_small, inplace, inplace_triv, pure
	);

	vool::suit_config config;
	config.persistent = true;
	config.repetitions = 5;
	config.x_res = 1280;
	config.y_res = 720;

	auto suit = vool::make_test_suit(config, small_category);
	suit.perform_categorys(0, testsize);
	suit.render_results();
}

int main()
{
	benchmark();
}