#include "Machine.hpp"

namespace basic {

void Machine::transfer_context_data(Context *p_context) {
	if (!p_context)
		return;
	if (m_terminated.load(std::memory_order_acquire))
		p_context->Terminate();

	std::scoped_lock input_lock{m_input_mutex};
	while (!m_inputs.empty()) {
		p_context->PushInput(m_inputs.front());
		m_inputs.pop();
	}
}

struct ReturnCaller {
	std::function<void()> func;
	inline ~ReturnCaller() { func(); }
};

ExecuteResult Machine::execute(std::unique_ptr<Program> program, std::unique_ptr<Context> context,
                               const std::function<void()> &callback) {
#define UNWRAP_ASSIGN(L_VALUE, RESULT) \
	do { \
		auto result = RESULT; \
		if (result.IsError()) \
			return {std::move(program), std::move(context), result.PopError()}; \
		L_VALUE = result.PopValue(); \
	} while (false)

#define UNWRAP(RESULT) \
	do { \
		auto result = RESULT; \
		if (result.IsError()) \
			return {std::move(program), std::move(context), result.PopError()}; \
	} while (false)

#define RET_ERROR(ERR) \
	return { \
		std::move(program), std::move(context), RuntimeError { ERR } \
	}

	// Call callback function when return
	ReturnCaller return_caller{callback};

	if (context == nullptr) {
		std::unique_ptr<Context> new_context;
		UNWRAP_ASSIGN(new_context, Context::Create(*program));
		context = std::move(new_context);
	}

	while (true) {
		transfer_context_data(context.get());

		if (context->IsTerminated())
			RET_ERROR(ErrTerminate{});

		const Statement *p_stmt;
		UNWRAP_ASSIGN(p_stmt, program->GetStatement(context->GetLine()));
		UNWRAP(p_stmt->Run(*program, context.get()));
	}

	return {std::move(program), std::move(context), {}};
}

} // namespace basic