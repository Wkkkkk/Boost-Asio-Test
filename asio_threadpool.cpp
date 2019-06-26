/*
 * Copyright (c) 2018 Ally of Intelligence Technology Co., Ltd. All rights reserved.
 *
 * Created by WuKun on 5/21/19.
 * Contact with:wk707060335@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http: *www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t size) :
        io_context_(size),
        work_guard_(boost::asio::make_work_guard(io_context_))
    {
        for (std::size_t i = 0; i < size; ++i) {
            group_.create_thread([&](){ io_context_.run(); });
        }
    }

    ~ThreadPool() {
        work_guard_.reset();
        group_.join_all();
    }

    // Add new work item to the pool.
    template<class F>
    void Enqueue(F f) {
        boost::asio::post(io_context_, f);
    }

private:
    boost::thread_group group_;
    boost::asio::io_context io_context_;

    typedef boost::asio::io_context::executor_type ExecutorType;
    boost::asio::executor_work_guard<ExecutorType> work_guard_;
};

std::mutex g_io_mutex;

int main ( int argc, char* argv[] ) {
    int thread_num = std::thread::hardware_concurrency();
    std::cout << "thread num: " << thread_num<< std::endl;

    ThreadPool pool(thread_num);
    // Queue a bunch of work items.
    for (int i = 0; i < 8; ++i) {
        auto f = [i] {
            {
                std::lock_guard<std::mutex> lock(g_io_mutex);
                std::cout << "Hello" << "(" << i << ") " << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));

            {
                std::lock_guard<std::mutex> lock(g_io_mutex);
                std::cout << "World" << "(" << i << ")" << std::endl;
            }
        };

        pool.Enqueue(f);
    }

    return 0;
}