#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
template <class T, std::size_t N> class CircularBuffer {
    std::array<T, N> a{};
    std::size_t h = 0, t = 0, n = 0;
    bool end = false;
    std::mutex m;
    std::condition_variable c;

  public:
    void put(T x) {
        std::unique_lock<std::mutex> l(m);
        c.wait(l, [&] { return n < N || end; });
        if (end)
            return;
        a[t] = x;
        t = (t + 1) % N;
        ++n;
        c.notify_all();
    }
    bool get(T &x) {
        std::unique_lock<std::mutex> l(m);
        c.wait(l, [&] { return n || end; });
        if (!n)
            return false;
        x = a[h];
        h = (h + 1) % N;
        --n;
        c.notify_all();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> l(m);
        end = true;
        c.notify_all();
    }
};
int main() {
    CircularBuffer<char, 10> b;
    std::vector<char> read;
    std::thread t1([&] {
        std::cout << "Nhap ky tu (space de dung):\n";
        char c;
        while (std::cin.get(c)) {
            if (c == ' ')
                break;
            if (c == '\n' || c == '\r')
                continue;
            b.put(c);
        }
        b.close();
    });
    std::thread t2([&] {
        char c;
        while (b.get(c)) {
            std::cout << c << ' ';
            read.push_back(c);
        }
    });
    t1.join();
    t2.join();
    std::size_t count = 0;
    for (char c : read)
        if (c == 'A')
            ++count;
    std::cout << "\nTask 3 (sau join): so ky tu A = " << count << '\n';
}
