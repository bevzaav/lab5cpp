#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>

using namespace std;

mutex cout_mutex;

void process(const string& name, int duration_sec) {
    this_thread::sleep_for(chrono::seconds(duration_sec));

    lock_guard<mutex> lock(cout_mutex);
    cout << name << " completed (" << duration_sec << "s)" << endl;
}

void slow(const string& name) {
    process(name, 7);
}

void quick(const string& name) {
    process(name, 1);
}

void work() {
    auto start_time = chrono::steady_clock::now();

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Work started..." << endl;
    }


    // A -> B -> C1 -> D1 -> F (Критичний шлях: 7+7+1+1+1 = 17с)
    // C2 -> D1
    // D2 -> F


    auto future_chain_abc = async(launch::async, []() {
        slow("A");      // 7s
        slow("B");      // 7s
        quick("C1");    // 1s
    });

    quick("C2");

    quick("D2");

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Main thread waiting for A->B->C1 chain..." << endl;
    }

    future_chain_abc.get();

    quick("D1");

    quick("F");

    auto end_time = chrono::steady_clock::now();
    auto elapsed_ns = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Work is done! Total time: " << elapsed_ns << " seconds." << endl;
    }
}

int main() {
    work();
    return 0;
}