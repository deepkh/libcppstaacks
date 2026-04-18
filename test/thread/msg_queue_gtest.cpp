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

#include <cppstuff/thread/msg_queue.hpp>
#include <gtest/gtest.h>

#include <sys/wait.h> // WIFEXITED, etc.
#include <unistd.h>   // fork()

using namespace cppstuff::thread;

struct Request {
    int id;
    std::string data;
};

struct Response {
    bool success;
    std::string message;
};

enum MsgType { CALIBRATE = 1, DISABLE_EIS, FINISH };

class ProducerConsumerGtest : public ::testing::Test {};

MsgQueue<Msg<Request, Response>> msgQueue2;

static void process_msg() {
    while (true) {
        auto msg = msgQueue2.Pop();
        switch (msg->getType()) {
        case CALIBRATE: {
            Response response{true, "Calibration successful"};
            msg->setResponse(response);
            break;
        }
        case DISABLE_EIS: {
            Response response{false, "EIS Disabled"};
            msg->setResponse(response);
            break;
        }
        case FINISH: {
            Response response{false, "FINISH"};
            msg->setResponse(response);
            return;
        }
        default: {
            Response response{false, "Unknown message type"};
            msg->setResponse(response);
            break;
        }
        }
    }
}

TEST_F(ProducerConsumerGtest, ProcessMsg) {
    // Launch a thread to process the message asynchronously
    std::thread processingThread(process_msg);

    for (int i = 0; i < 100; i++) {
        // Create a Msg object
        auto type = i % 2 == 0 ? CALIBRATE : DISABLE_EIS;
        if (i == 99) {
            type = FINISH;
        }
        // auto request = Request();
        // request.id = i;
        // request.data = "Calibration Data";

        auto message = std::make_shared<Msg<Request, Response>>(type, Request{i, "XXXXXXXXX"});
        msgQueue2.Push(message);

        if (type == DISABLE_EIS) {
            continue;
        }

        // Get the response (it will block until the promise is set)
        auto response = message->getResponse().get();
        // Output the result
        std::cout << "Msg processed. Success: " << response.success << ", Msg: " << response.message
                  << std::endl;
    }

    // Wait for the processing thread to finish
    processingThread.join();
}

#define TOTAL 100000
MsgQueue<Msg<Request, Response>> msgQueue; // Create a thread-safe queue

static void run_producer() {
    int64_t sum = 0;
    for (int i = 0; i < TOTAL; i++) {
        auto request = Request();
        request.id = i;
        request.data = "Calibration Data";
        auto message = std::make_shared<Msg<Request, Response>>(CALIBRATE, request);
        msgQueue.Push(message);
        auto response = message->getResponse().get();
        // printf("P:%d '%s'\n", i, response.message.c_str());
        sum += i;
    }
    printf("P TOTAL: %ld\n", sum);
}

static void run_consumer() {
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
        // printf("\t\tC:%d '%s'\n", value, data.c_str());
        sum += value;
    }
    printf("C TOTAL:%ld\n", sum);
}

TEST_F(ProducerConsumerGtest, RunProducerConsumer) {
    std::thread producer_thread(run_producer);
    std::thread consumer_thread(run_consumer);

    producer_thread.join();
    // msgQueue.Push(Msg<int>(Msg<int>::CALIBRATE, std::make_shared<int>(-999)));
    consumer_thread.join();
}

// main() entry for GoogleTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
