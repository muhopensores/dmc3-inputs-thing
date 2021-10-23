#pragma once

#include <deque>

namespace utility {

	template <typename T>
	struct FixedQueue {

		size_t max_size = 15;
		std::deque<T> m_deq;

		// NOTE(): this is me pretending to know modern c++
		inline void push(const T&& element) {
			if (m_deq.size() == max_size) {
				m_deq.pop_back();
			}                   //same here lmaoo
			m_deq.emplace_front(std::move(element));
		}
	};
}