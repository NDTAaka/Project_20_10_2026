#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
int main() {
    constexpr int N = 10;
    std::array<int, N> a{};
    int count = 0;
    std::mutex m;
    std::thread t1([&] {
        std::mt19937 g(std::random_device{}());
        std::uniform_int_distribution<int> d(0, 99);
        for (int i = 0; i < N; ++i) {
            {
                std::lock_guard<std::mutex> l(m);
                a[i] = d(g);
                count = i + 1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    std::thread t2([&] {
        while (true) {
            std::array<int, N> s{};
            int n;
            {
                std::lock_guard<std::mutex> l(m);
                n = count;
                for (int i = 0; i < n; ++i)
                    s[i] = a[i];
            }
            std::cout << "Mang hien tai: ";
            for (int i = 0; i < n; ++i)
                std::cout << s[i] << ' ';
            std::cout << '\n';
            if (n == N)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });
    t1.join();
    t2.join();
    std::sort(a.begin(), a.end());
    std::cout << "Task 3 sau join, mang tang dan: ";
    for (int x : a)
        std::cout << x << ' ';
    std::cout << '\n';
}
