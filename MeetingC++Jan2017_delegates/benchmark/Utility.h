#ifndef COROUTINEVSFUNCTION_UTILITY_H_INCLUDED
#define COROUTINEVSFUNCTION_UTILITY_H_INCLUDED

#include <experimental\coroutine>
#include <variant>

using namespace std::experimental;

namespace util
{

	class coro_task
	{
	public:
		struct promise_type
		{
			coro_task get_return_object()
			{
				return coro_task(coroutine_handle<promise_type>::from_promise(*this));
			}

			auto initial_suspend() { return suspend_never{}; }
			auto final_suspend() { return suspend_always{}; }

			void return_void() {}
		};

		coro_task() : _coroutine(nullptr) { }

		~coro_task()
		{
			if (_coroutine)
				_coroutine.destroy();
		}

		coro_task(const coro_task&) = delete;
		coro_task(coro_task&& other) :
			_coroutine(other._coroutine)
		{
			other._coroutine = nullptr;
		}

		coro_task& operator= (const coro_task&) = delete;
		coro_task& operator= (coro_task&& other)
		{
			if (this != std::addressof(other))
			{
				_coroutine = other._coroutine;
				other._coroutine = nullptr;
			}
			return *this;
		}

		void resume() { _coroutine.resume(); }

		void wait()
		{
			while (!_coroutine.done())
				_coroutine.resume();
		}

		bool done() const { return _coroutine.done(); }

	private:
		explicit coro_task(coroutine_handle<promise_type> coroutine) :
			_coroutine(coroutine)
		{ }

		coroutine_handle<promise_type> _coroutine;
	};

}

#endif // COROUTINEVSFUNCTION_UTILITY_H_INCLUDED