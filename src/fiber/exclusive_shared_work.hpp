#pragma once

#include <condition_variable>
#include <chrono>
#include <deque>
#include <mutex>

#include <boost/config.hpp>

#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/scheduler.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif

namespace boost {
    namespace fibers {
        namespace algo {

            template <int SLOT>
            class BOOST_FIBERS_DECL exclusive_shared_work : public algorithm {
            private:
                typedef std::deque< context* >  rqueue_type;
                typedef scheduler::ready_queue_type lqueue_type;

                static rqueue_type     	rqueue_;
                static std::mutex   	rqueue_mtx_;

                lqueue_type            	lqueue_{};
                std::mutex              mtx_{};
                std::condition_variable cnd_{};
                bool                    flag_{ false };
                bool                    suspend_{ false };

            public:
                exclusive_shared_work() = default;

                exclusive_shared_work(bool suspend) :
                    suspend_{ suspend } {
                }

                exclusive_shared_work(exclusive_shared_work const&) = delete;
                exclusive_shared_work(exclusive_shared_work&&) = delete;

                exclusive_shared_work& operator=(exclusive_shared_work const&) = delete;
                exclusive_shared_work& operator=(exclusive_shared_work&&) = delete;

                void awakened(context* ctx) noexcept override;

                context* pick_next() noexcept override;

                bool has_ready_fibers() const noexcept override {
                    std::unique_lock< std::mutex > lock{ rqueue_mtx_ };
                    return !rqueue_.empty() || !lqueue_.empty();
                }

                void suspend_until(std::chrono::steady_clock::time_point const& time_point) noexcept override;

                void notify() noexcept override;
            };

        }
    }
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#include "fiber/exclusive_shared_work_impl.hpp"
