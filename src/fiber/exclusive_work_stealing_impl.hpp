
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include "fiber/exclusive_work_stealing.hpp"

#include <random>

#include <boost/assert.hpp>
#include <boost/context/detail/prefetch.hpp>

#include "boost/fiber/detail/thread_barrier.hpp"
#include "boost/fiber/type.hpp"


template <int SLOT>
std::atomic< std::uint32_t > exclusive_work_stealing<SLOT>::counter_{ 0 };
template <int SLOT>
std::vector< boost::intrusive_ptr< exclusive_work_stealing<SLOT> > > exclusive_work_stealing<SLOT>::schedulers_{};

template <int SLOT>
void
exclusive_work_stealing<SLOT>::init_(std::uint32_t thread_count,
    std::vector< boost::intrusive_ptr< exclusive_work_stealing > >& schedulers) {
    // resize array of schedulers to thread_count, initilized with nullptr
    std::vector< boost::intrusive_ptr< exclusive_work_stealing > >{ thread_count, nullptr }.swap(schedulers);
}

template <int SLOT>
exclusive_work_stealing<SLOT>::exclusive_work_stealing(std::uint32_t thread_count, bool suspend) :
    id_{ counter_++ },
    thread_count_{ thread_count },
    suspend_{ suspend } {
    static boost::fibers::detail::thread_barrier b{ thread_count };
    // initialize the array of schedulers
    static std::once_flag flag;
    std::call_once(flag, &exclusive_work_stealing<SLOT>::init_, thread_count_, std::ref(schedulers_));
    // register pointer of this scheduler
    schedulers_[id_] = this;
    b.wait();
}

template <int SLOT>
void
exclusive_work_stealing<SLOT>::awakened(boost::fibers::context* ctx) noexcept {
    if (!ctx->is_context(boost::fibers::type::pinned_context)) {
        ctx->detach();
    }

    if (bundle_)
    {
        bundles_.push(ctx);
    }
    else
    {
        rqueue_.push(ctx);
    }
}

template <int SLOT>
boost::fibers::context*
exclusive_work_stealing<SLOT>::pick_next() noexcept {
    boost::fibers::context* victim = bundles_.pop();

    if (nullptr != victim)
    {
        boost::context::detail::prefetch_range(victim, sizeof(boost::fibers::context));
        if (!victim->is_context(boost::fibers::type::pinned_context)) {
            boost::fibers::context::active()->attach(victim);
        }
    }
    else
    {
        victim = rqueue_.pop();
        if (nullptr != victim) {
            boost::context::detail::prefetch_range(victim, sizeof(boost::fibers::context));
            if (!victim->is_context(boost::fibers::type::pinned_context)) {
                boost::fibers::context::active()->attach(victim);
            }
        }
        else {
            std::uint32_t id = 0;
            std::size_t count = 0, size = schedulers_.size();
            static thread_local std::minstd_rand generator{ std::random_device{}() };
            std::uniform_int_distribution< std::uint32_t > distribution{
                0, static_cast<std::uint32_t>(thread_count_ - 1) };
            do {
                do {
                    ++count;
                    // random selection of one logical cpu
                    // that belongs to the local NUMA node
                    id = distribution(generator);
                    // prevent stealing from own scheduler
                } while (id == id_);
                // steal context from other scheduler
                victim = schedulers_[id]->steal();
            } while (nullptr == victim && count < size);
            if (nullptr != victim) {
                boost::context::detail::prefetch_range(victim, sizeof(boost::fibers::context));
                BOOST_ASSERT(!victim->is_context(boost::fibers::type::pinned_context));
                boost::fibers::context::active()->attach(victim);
            }
        }
    }
    return victim;
}

template <int SLOT>
void
exclusive_work_stealing<SLOT>::suspend_until(std::chrono::steady_clock::time_point const& time_point) noexcept {
    if (suspend_) {
        if ((std::chrono::steady_clock::time_point::max)() == time_point) {
            std::unique_lock< std::mutex > lk{ mtx_ };
            cnd_.wait(lk, [this]() { return flag_; });
            flag_ = false;
        }
        else {
            std::unique_lock< std::mutex > lk{ mtx_ };
            cnd_.wait_until(lk, time_point, [this]() { return flag_; });
            flag_ = false;
        }
    }
}

template <int SLOT>
void
exclusive_work_stealing<SLOT>::notify() noexcept {
    if (suspend_) {
        std::unique_lock< std::mutex > lk{ mtx_ };
        flag_ = true;
        lk.unlock();
        cnd_.notify_all();
    }
}

template <int SLOT>
void exclusive_work_stealing<SLOT>::start_bundle()
{
    bundle_ = true;
}

template <int SLOT>
void exclusive_work_stealing<SLOT>::end_bundle()
{
    bundle_ = false;
}
