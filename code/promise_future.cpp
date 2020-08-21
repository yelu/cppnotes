#include <iostream>
#include <thread>
#include <future>

using namespace std;

void wait_for_data(const future<int>& f) {
  // blocking until data available
  int data = f.get();  
  cout << "received" << data << endl;
}

void make_data(promise<int>& p) {
    cout << "task completed, data ready" << endl;
    p.set_value(1);
}

int main() {
    promise<int> p;
    future<int> f = p.get_future();
    future<int> f1 = f.then(
        [](auto& f) {
            int data = f.get();
            return data * data;
            })
    thread consumer(wait_for_data, f1);
    thread producer(make_data, p);
    consumer.join();
    producer.join();

    return 0;
}