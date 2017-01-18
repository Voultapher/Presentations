#include <vector>
#include <algorithm>
#include <iostream>

#include "Utility.h"

#include <TestSuit.h>

#include <experimental\coroutine>
#include <functional>
#include <thread>

void task()
{
	volatile std::vector<std::string> vec = {
		"short"/*, "a", "a little longer, and even more"*/
	};
}

struct func_ptr
{
	using type = void(*)();
};

class coro_queue
{
public:
	template<typename F> void add_task(F&& func)
	{
		tasks_.emplace_back(
			[](auto func) -> util::coro_task
			{
				co_await std::experimental::suspend_always{};
				func();
			}(std::forward<F>(func))
		);
	}

	void run_all_tasks()
	{
		for (auto& task : tasks_)
			task.resume();
	}

	void clear()
	{
		tasks_.clear();
	}
private:
	std::vector<util::coro_task> tasks_;
};

template<typename T> class simple_queue
{
public:
	using func_t = typename T;

	void add_task(func_t func)
	{
		tasks_.push_back(std::move(func));
	}

	void run_all_tasks()
	{
		for (auto& task : tasks_)
			task();
	}

	void clear()
	{
		tasks_.clear();
	}
private:
	std::vector<func_t> tasks_;
};

template<typename T> void fill_queue(T& queue, size_t testsize)
{
	for (size_t i = 0; i < testsize; ++i)
		queue.add_task(task);
}

void benchmark(const std::string& name, size_t testsize)
{
	simple_queue<std::function<void()>> sq;
	simple_queue<func_ptr::type> fq;
	coro_queue cq;

	// --- CLEAR
	auto clear = vool::make_test(
		"clear",
		[&sq, &cq, &fq](size_t size) { sq.clear(); cq.clear(); fq.clear(); }
	);
	//clear.flag_invisible();

	// --- FILL
	auto simple_fill = vool::make_test(
		"fill function<void()>",
		[&sq](size_t size) { fill_queue(sq, size); }
	);
	auto fptr_fill = vool::make_test(
		"fill func_ptr::type>",
		[&fq](size_t size) { fill_queue(fq, size); }
	);
	auto coro_fill = vool::make_test(
		"fill coro task",
		[&cq](size_t size) { fill_queue(cq, size); }
	);

	auto fill_category = vool::make_test_category(
		"fill",
		clear, simple_fill, fptr_fill, coro_fill
	);

	// --- RUN
	auto simple_run = vool::make_test(
		"run std::function<void()>",
		[&sq](size_t size) { sq.run_all_tasks(); }
	);
	auto coro_run = vool::make_test(
		"run coro task>",
		[&cq](size_t size) { cq.run_all_tasks(); }
	);

	auto run_category = vool::make_test_category(
		"fill-run",
		clear,
		simple_fill, simple_run,
		coro_fill, coro_run
	);

	// --- TESTSUIT
	vool::suit_config config;
	config.filename = name.c_str();
	config.png_output = true;
	config.repetitions = 1;

	auto suit = vool::make_test_suit(
		config,
		fill_category, run_category
	);

	suit.perform_categorys(0, testsize);
	suit.render_results();
}

void test()
{
	simple_queue<std::function<void()>> sq;
	coro_queue cq;

	fill_queue(sq, 10);
	fill_queue(cq, 10);

	sq.run_all_tasks();
	cq.run_all_tasks();

	std::cout << "Size of coro_task: " << sizeof(util::coro_task) << '\n';
	std::cout << "Size of std::function<void()>: " << sizeof(std::function<void()>) << '\n';
}

void func_ptr_test(func_ptr::type f)
{
	std::cout << "huhu\n";
	std::vector<decltype(f)> vec{
		f,
		([]() { std::cout << "observable side effect\n"; })
	};

	std::cout << "f_ptr size: " << sizeof(f) << '\n';
	std::cout << "func_ptr size: " << sizeof(func_ptr) << '\n';

	for (const auto& fu : vec)
		fu();
}

void name_te()
{
	std::cout << "name_te() called\n";
}

int main()
{
#ifdef NDEBUG
	std::string name = "Release";
	size_t testsize = static_cast<size_t>(1e4);
#else
	std::string name = "Debug";
	size_t testsize = 10;
#endif

	try
	{
		benchmark(name, testsize);

		//test();
		//func_ptr_test(name_te);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << '\n';
	}

	std::cin.get();
	return 0;
}