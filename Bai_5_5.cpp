#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
template <class T, std::size_t N> class CircularBuffer {
    std::array<T, N> a{};
    std::size_t h = 0, t = 0, n = 0;
    bool end = false;
    std::mutex mutex;
    std::condition_variable cv;

  public:
    void put(T x) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return n < N || end; });
        if (end)
            return;
        a[t] = x;
        t = (t + 1) % N;
        ++n;
        cv.notify_all();
    }
    bool get(T &x) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return n || end; });
        if (!n)
            return false;
        x = a[h];
        h = (h + 1) % N;
        --n;
        cv.notify_all();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> lock(mutex);
        end = true;
        cv.notify_all();
    }
};
int main() {
    CircularBuffer<int, 5> b;
    std::thread t1([&] {
        for (int i = 1; i <= 20; ++i) {
            b.put(i);
            std::cout << "Task 1 ghi " << i << " (mutex bao ve buffer)\n";
        }
        b.close();
    });
    std::thread t2([&] {
        int x;
        while (b.get(x))
            std::cout << "Task 2 doc " << x << " (du lieu hop le)\n";
    });
    t1.join();
    t2.join();
}
