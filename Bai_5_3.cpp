#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
template <class T, std::size_t N> class OverwriteRing {
    std::array<T, N> a{};
    std::size_t h = 0, n = 0;
    mutable std::mutex m;

  public:
    void put(T x) {
        std::lock_guard<std::mutex> l(m);
        if (n < N) {
            a[(h + n) % N] = x;
            ++n;
        } else {
            a[h] = x;
            h = (h + 1) % N;
        }
    }
    std::array<T, N> snapshot(std::size_t &z) const {
        std::lock_guard<std::mutex> l(m);
        z = n;
        std::array<T, N> r{};
        for (std::size_t i = 0; i < n; ++i)
            r[i] = a[(h + i) % N];
        return r;
    }
};
int main() {
    OverwriteRing<int, 10> b;
    std::atomic<bool> done(false);
    std::thread t1([&] {
        for (int i = 1; i <= 25; ++i) {
            b.put(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        done = true;
    });
    std::thread t2([&] {
        while (!done) {
            std::size_t n;
            auto x = b.snapshot(n);
            std::cout << "Du lieu: ";
            for (std::size_t i = 0; i < n; ++i)
                std::cout << x[i] << ' ';
            std::cout << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(350));
        }
        std::size_t n;
        auto x = b.snapshot(n);
        std::cout << "Cuoi cung (giu 10 gia tri moi nhat): ";
        for (std::size_t i = 0; i < n; ++i)
            std::cout << x[i] << ' ';
        std::cout << '\n';
    });
    t1.join();
    t2.join();
}
