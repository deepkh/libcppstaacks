#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>


struct Request {
  int id;
  std::string data;
};

struct Response {
  bool success;
  std::string message;
};

enum MsgType { CALIBRATE=1, DISABLE_EIS, FINISH };

template <typename R, typename P>
class Msg {
 private:
  int type;
  R request;
  std::promise<P> response;
  std::future<P> response_future;

 public:
  Msg(int type, R request) : type(type), request(request), response_future(response.get_future()) {}
  int getType() const { return type; }
  void setType(int type) { type = type; }
  const R& getRequest() const { return request; }
  void setResponse(P value) { response.set_value(value); }
  std::future<P>& getResponse() { return response_future; }
};

template <typename T>
class MsgQueue {
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
    cond.wait(lock, [this] { return !queue.empty(); });  // Wait for data

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


MsgQueue<Msg<Request, Response>> msgQueue2;

void processMsg() {
  while(true) {
    auto msg = msgQueue2.Pop();
#if 1
    switch (msg->getType()) {
      case CALIBRATE:
        {
          Response response{true, "Calibration successful"};
          msg->setResponse(response);
          break;
        }
      case DISABLE_EIS:
        {
          Response response{false, "EIS Disabled"};
          msg->setResponse(response);
          break;
        }
      case FINISH:
        {
          Response response{false, "FINISH"};
          msg->setResponse(response);
          return;
        }
      default:
        {
          Response response{false, "Unknown message type"};
          msg->setResponse(response);
          break;
        }
    }
#endif
  }
}

#define TOTAL 100000

MsgQueue<Msg<Request, Response>> msgQueue;  // Create a thread-safe queue

void producer() {
  int64_t sum = 0;
  for (int i = 0; i < TOTAL; i++) {
    auto request = Request();
    request.id = i;
    request.data = "Calibration Data";
    auto message = std::make_shared<Msg<Request, Response>>(CALIBRATE, request);
    msgQueue.Push(message);
    auto response = message->getResponse().get();
    //printf("P:%d '%s'\n", i, response.message.c_str());
    sum += i;
  }
  printf("P TOTAL: %ld\n", sum);
}

void consumer() {
  int64_t sum = 0;
  for (int i = 0; i < TOTAL; ++i) {
    std::shared_ptr<Msg<Request, Response>> msg = msgQueue.Pop();
    int value = msg->getRequest().id;
    std::string data = msg->getRequest().data;
    if (value == -999) {
      break;
    }
    std::string str;
    str.resize(32);
    sprintf(&str[0], "OOOOOOOOOK %d", i);
    Response response{false, str};
    msg->setResponse(response);
    //printf("\t\tC:%d '%s'\n", value, data.c_str());
    sum += value;
  }
  printf("C TOTAL:%ld\n", sum);
}

int main() {

  std::thread producer_thread(producer);
  std::thread consumer_thread(consumer);

  producer_thread.join();
  // msgQueue.Push(Msg<int>(Msg<int>::CALIBRATE, std::make_shared<int>(-999)));
  consumer_thread.join();

#if 0
  // Launch a thread to process the message asynchronously
  std::thread processingThread(processMsg);

  for (int i=0; i<100; i++) {
    // Create a Msg object
    auto type = i%2 == 0 ? CALIBRATE : DISABLE_EIS;
    if (i == 99) {
      type = FINISH;
    }
    //auto request = Request();
    //request.id = i;
    //request.data = "Calibration Data";

    auto message = std::make_shared<Msg<Request, Response>>(type, Request{i, "XXXXXXXXX"});
    msgQueue2.Push(message);

    if (type == DISABLE_EIS) {
      continue;
    }
    
    // Get the response (it will block until the promise is set)
    auto response = message->getResponse().get();
    // Output the result
    std::cout << "Msg processed. Success: " << response.success << ", Msg: " << response.message << std::endl;
  }

  // Wait for the processing thread to finish
  processingThread.join();
#endif
  



  return 0;
}

