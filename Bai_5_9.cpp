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
    CircularBuffer<int, 6> b;
    std::mutex turnMutex;
    std::condition_variable turnCv;
    int turn = 0;
    constexpr int PAIRS = 10;
    std::thread even([&] {
        for (int i = 0; i < PAIRS; ++i) {
            std::unique_lock<std::mutex> l(turnMutex);
            turnCv.wait(l, [&] { return turn == 0; });
            b.put(2 * i);
            turn = 1;
            l.unlock();
            turnCv.notify_all();
        }
    });
    std::thread odd([&] {
        for (int i = 0; i < PAIRS; ++i) {
            std::unique_lock<std::mutex> l(turnMutex);
            turnCv.wait(l, [&] { return turn == 1; });
            b.put(2 * i + 1);
            turn = 0;
            l.unlock();
            turnCv.notify_all();
        }
    });
    std::thread reader([&] {
        int x, expected = 0;
        bool valid = true;
        while (expected < 2 * PAIRS && b.get(x)) {
            std::cout << x << ' ';
            if (x != expected)
                valid = false;
            ++expected;
        }
        std::cout << "\nThu tu " << (valid ? "toan ven" : "LOI") << "; da doc " << expected
                  << " phan tu\n";
    });
    even.join();
    odd.join();
    b.close();
    reader.join();
}
