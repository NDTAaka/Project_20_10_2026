#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif
template <class T, std::size_t N> class CircularBuffer {
    std::array<T, N> a{};
    std::size_t h = 0, t = 0, n = 0;
    bool end = false;
    std::mutex m;
    std::condition_variable cv;

  public:
    void put(T x) {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [&] { return n < N || end; });
        if (end)
            return;
        a[t] = x;
        t = (t + 1) % N;
        ++n;
        cv.notify_all();
    }
    bool get(T &x) {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [&] { return n || end; });
        if (!n)
            return false;
        x = a[h];
        h = (h + 1) % N;
        --n;
        cv.notify_all();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> l(m);
        end = true;
        cv.notify_all();
    }
};
int main() {
    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    CircularBuffer<int, 10> b;
    std::thread t1([&] {
        std::cout << "Nhap so (so am de dung):\n";
        int x;
        while (std::cin >> x && x >= 0)
            b.put(x);
        b.close();
    });
    std::thread t2([&] {
        int x;
        while (b.get(x))
            std::cout << "Task 2 hien thi: " << x << '\n';
    });
    t1.join();
    t2.join();
#ifdef _WIN32
    MEMORYSTATUSEX s{};
    s.dwLength = sizeof(s);
    GlobalMemoryStatusEx(&s);
    unsigned long long freeRam = s.ullAvailPhys;
#else
    struct sysinfo s{};
    sysinfo(&s);
    unsigned long long freeRam = static_cast<unsigned long long>(s.freeram) * s.mem_unit;
#endif
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count();
    std::cout << "Task 3 (sau join): RAM con trong = " << freeRam
              << " byte\nThoi gian chuong trinh = " << ms << " ms\n";
}
