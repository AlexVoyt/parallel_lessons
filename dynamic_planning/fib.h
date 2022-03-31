#include "tasks.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

class control_state_fib_async : public control_state
{
    std::atomic<unsigned> m_refs = 2;
	unsigned m_r1, m_r2;
	unsigned* m_result_ptr;
    std::shared_ptr<control_state> m_parent;
public:
	control_state_fib_async(unsigned* result_ptr,
		std::shared_ptr<control_state> parent_state)
        : m_result_ptr(result_ptr), m_parent(parent_state) {}

    ~control_state_fib_async()
    {
    }

    static std::shared_ptr<control_state_fib_async> create(unsigned* result_ptr,
                                                           std::shared_ptr<control_state> parent_state)
    {
        return std::make_shared<control_state_fib_async>(result_ptr, parent_state);
    }

	virtual void at_exit() override
	{
        if(--m_refs == 0)
        {
            std::shared_ptr<control_state> parent = m_parent;
            *m_result_ptr = m_r1 + m_r2;
            parent->at_exit();
        }
	}
	unsigned* result1_ptr()
	{
		return &m_r1;
	}
	unsigned* result2_ptr()
	{
		return &m_r2;
	}
};

class control_state_fib_blocking : public control_state
{
	std::mutex m_mtx;
	std::condition_variable m_cv;
	std::atomic<bool> m_ready;
    unsigned* m_result;
public:
	control_state_fib_blocking(unsigned* result)
    : m_ready(std::atomic<bool>(false)), m_result(result) {}
	virtual void at_exit() override
	{
		m_ready.store(true, std::memory_order_release);
		//m_ready.store(true);
		m_cv.notify_all();
	}
	void wait()
	{
		std::unique_lock<std::mutex> lock{m_mtx};
		//while (!bool(m_ready))
		while (!m_ready.load(std::memory_order_acquire))
			m_cv.wait(lock);
	}

    unsigned* result() const
    {
        return m_result;
    }
};

class task_fib : public task
{
	unsigned m_n;
    std::shared_ptr<control_state> m_ctrl;
	unsigned* m_result_ptr;
public:
	task_fib(unsigned n, std::shared_ptr<control_state> ctrl, unsigned* result_ptr):m_n(n), m_ctrl(ctrl), m_result_ptr(result_ptr) {}
	void run() override
	{
		if (m_n <= 2)
        {
			*m_result_ptr = 1;
            m_ctrl->at_exit();
        }
        else
        {
            // auto control_state_async = control_state_fib_async(m_result_ptr, m_ctrl);
            std::shared_ptr<control_state_fib_async> ctrl = control_state_fib_async::create(m_result_ptr, m_ctrl);

            auto task1 = std::make_unique<task_fib>(task_fib(m_n - 1, ctrl, ctrl->result1_ptr()));
            auto task2 = std::make_unique<task_fib>(task_fib(m_n - 2, ctrl, ctrl->result2_ptr()));
            load_balancer::get().add_task(std::move(task1));
            load_balancer::get().add_task(std::move(task2));
        }
	}
};
