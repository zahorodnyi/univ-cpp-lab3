// Compiler: C++20 (Apple Clang compatible / GCC 11+ / MSVC)
// Note: Replaced osyncstream with mutex because macOS libc++ does not support syncstream yet.
#include <iostream>
#include <vector>
#include <thread>
#include <latch>
#include <barrier>
#include <mutex>
#include <chrono>

struct Counts {
    static constexpr int A = 6;
    static constexpr int B = 9;
    static constexpr int C = 8;
    static constexpr int D = 7;
    static constexpr int E = 5;
    static constexpr int F = 7;
    static constexpr int G = 5;
    static constexpr int H = 4;
    static constexpr int I = 9;
    static constexpr int J = 7;
};

class Workflow {
public:
    void run() {
        print("Calculation started.\n");

        std::vector<std::thread> threads;
        threads.reserve(4);

        threads.emplace_back([this] {
            start_barrier.arrive_and_wait();

            process('a', Counts::A);
            a_done.count_down();

            process('c', Counts::C);
            c_done.count_down();

            d_done.wait();
            f_done.wait();
            g_done.wait();
            process('i', Counts::I);
        });

        threads.emplace_back([this] {
            start_barrier.arrive_and_wait();

            process('b', Counts::B);
            b_done.count_down();

            e_done.wait();

            process('f', Counts::F);
            f_done.count_down();

            process('h', Counts::H);
            h_done.count_down();
        });

        threads.emplace_back([this] {
            start_barrier.arrive_and_wait();

            a_done.wait();
            process('d', Counts::D);
            d_done.count_down();

            f_done.wait();
            g_done.wait();
            process('j', Counts::J);
        });

        threads.emplace_back([this] {
            start_barrier.arrive_and_wait();

            a_done.wait();
            process('e', Counts::E);
            e_done.count_down();

            b_done.wait();
            process('g', Counts::G);
            g_done.count_down();
        });

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        print("Calculation finished.\n");
    }

private:
    std::barrier<> start_barrier{4};
    std::latch a_done{1};
    std::latch b_done{1};
    std::latch c_done{1};
    std::latch d_done{1};
    std::latch e_done{1};
    std::latch f_done{1};
    std::latch g_done{1};
    std::latch h_done{1};

    mutable std::mutex io_mutex;

    void print(const std::string& msg) {
        std::lock_guard<std::mutex> lock(io_mutex);
        std::cout << msg;
    }

    void process(char name, int count) {
        for (int i = 1; i <= count; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "Action " << i << " from set " << name << " completed.\n";
        }
    }
};

int main() {
    Workflow workflow;
    workflow.run();
    return 0;
}