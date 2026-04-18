/*
Copyright (c) 2025, 2026 Gary Huang (deepkh@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace cppstuff {
namespace thread {

template <typename R, typename P> class Msg {
  private:
    int type;
    R request;
    std::promise<P> response;
    std::future<P> response_future;

  public:
    Msg(int type, R request)
        : type(type), request(request), response_future(response.get_future()) {}
    int getType() const { return type; }
    void setType(int type) { type = type; }
    const R &getRequest() const { return request; }
    void setResponse(P value) { response.set_value(value); }
    std::future<P> &getResponse() { return response_future; }
};

template <typename T> class MsgQueue {
  private:
    std::queue<std::shared_ptr<T>> queue;
    std::mutex mutex;
    std::condition_variable cond;

  public:
    void Push(const std::shared_ptr<T> data) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(data);
        cond.notify_one();
    }

    std::shared_ptr<T> Pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this] { return !queue.empty(); }); // Wait for data

        auto data = queue.front();
        queue.pop();
        return data;
    }

    void Clear() {
        std::unique_lock<std::mutex> lock(mutex);
        queue.clear();
    }

    bool Size() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }
};

} // namespace thread
} // namespace cppstuff
