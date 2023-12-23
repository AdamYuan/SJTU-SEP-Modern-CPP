#pragma once

#include "Context.hpp"
#include "Program.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

namespace basic {

struct ExecuteResult {
	std::unique_ptr<Program> program;
	std::unique_ptr<Context> context;
	RuntimeResult<void> result;
};

class Machine {
private:
	// modify context from another thread
	std::atomic_bool m_terminated;
	std::queue<String> m_inputs;
	std::mutex m_input_mutex;

	// async object
	std::future<ExecuteResult> m_result_future;

	void transfer_context_data(Context *p_context);
	ExecuteResult execute(std::unique_ptr<Program> program, std::unique_ptr<Context> context,
	                      const std::function<void()> &callback);

public:
	inline static std::unique_ptr<Machine> Execute(std::unique_ptr<Program> program, std::unique_ptr<Context> context,
	                                               const std::function<void()> &callback) {
		auto machine = std::make_unique<Machine>();
		machine->m_terminated.store(false, std::memory_order_release);
		machine->m_result_future =
		    std::async(&Machine::execute, machine.get(), std::move(program), std::move(context), callback);
		return machine;
	}
	inline static ExecuteResult GetResult(std::unique_ptr<Machine> *p_machine) {
		ExecuteResult result = (*p_machine)->m_result_future.get();
		(*p_machine)->transfer_context_data(result.context.get());
		*p_machine = nullptr;
		return result;
	}
	/* inline static std::optional<ExecuteResult> CheckResult(std::unique_ptr<Machine> *p_machine) {
	    if ((*p_machine)->m_result_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	        return GetResult(p_machine);
	    return std::nullopt;
	} */

	inline ~Machine() { m_terminated.store(true, std::memory_order_release); }

	inline void Terminate() { m_terminated.store(true, std::memory_order_release); }
	inline void PushInput(StringView string) {
		std::scoped_lock input_lock{m_input_mutex};
		m_inputs.emplace(string);
	}
};

} // namespace basic
