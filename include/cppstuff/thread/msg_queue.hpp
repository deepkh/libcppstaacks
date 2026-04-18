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
