#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
template <class T, std::size_t N> class CircularBuffer {
    std::array<T, N> a{};
    std::size_t h = 0, t = 0, n = 0;
    bool end = false;
    std::mutex m;
    std::condition_variable cv;

  public:
    bool put(T x) {
        std::unique_lock<std::mutex> l(m);
        if (n == N) {
            std::cout << "Buffer day! Beep!\a\n";
            cv.wait(l, [&] { return n < N || end; });
        }
        if (end)
            return false;
        a[t] = x;
        t = (t + 1) % N;
        ++n;
        cv.notify_all();
        return true;
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
    CircularBuffer<int, 5> b;
    std::thread t1([&] {
        std::cout << "Nhap 0..9 (EOF de dung):\n";
        int x;
        while (std::cin >> x) {
            if (x < 0 || x > 9) {
                std::cout << "Chi chap nhan [0,9]\n";
                continue;
            }
            if (!b.put(x))
                break;
        }
        b.close();
    });
    std::thread t2([&] {
        int x;
        while (b.get(x))
            std::cout << "Lay ra: " << x << '\n';
    });
    t1.join();
    t2.join();
}
