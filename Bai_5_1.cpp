#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

template <class T, std::size_t N> class CircularBuffer {
    std::array<T, N> a{};
    std::size_t head = 0, tail = 0, size = 0;
    bool closed = false;
    mutable std::mutex m;
    std::condition_variable cv;

  public:
    void put(const T &x) {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [&] { return size < N || closed; });
        if (closed)
            return;
        a[tail] = x;
        tail = (tail + 1) % N;
        ++size;
        cv.notify_all();
    }
    bool takeAll(std::array<T, N> &out, std::size_t &count) {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [&] { return size > 0 || closed; });
        if (!size)
            return false;
        count = size;
        for (std::size_t i = 0; i < count; ++i) {
            out[i] = a[head];
            head = (head + 1) % N;
            --size;
        }
        cv.notify_all();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> l(m);
        closed = true;
        cv.notify_all();
    }
};

int main() {
    CircularBuffer<int, 10> b;
    std::thread t1([&] {
        std::cout << "Nhap so nguyen (so am de dung):\n";
        int x;
        while (std::cin >> x && x >= 0)
            b.put(x);
        b.close();
    });
    std::thread t2([&] {
        std::array<int, 10> s{};
        std::size_t n;
        while (b.takeAll(s, n)) {
            long sum = 0;
            std::cout << "Cac phan tu vua co trong buffer: ";
            for (std::size_t i = 0; i < n; ++i) {
                std::cout << s[i] << ' ';
                sum += s[i];
            }
            std::cout << " | Trung binh: " << double(sum) / n << "\n";
        }
    });
    t1.join();
    t2.join();
}
