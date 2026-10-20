#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
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
        a[t] = std::move(x);
        t = (t + 1) % N;
        ++n;
        cv.notify_all();
    }
    bool get(T &x) {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [&] { return n || end; });
        if (!n)
            return false;
        x = std::move(a[h]);
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
    CircularBuffer<std::string, 10> b;
    std::vector<std::string> lines;
    std::thread t1([&] {
        std::cout << "Nhap cac dong (dong rong de dung):\n";
        std::string s;
        while (std::getline(std::cin, s) && !s.empty())
            b.put(s);
        b.close();
    });
    std::thread t2([&] {
        std::string s;
        while (b.get(s)) {
            std::cout << "Da nhan: " << s << '\n';
            lines.push_back(s);
        }
    });
    t1.join();
    t2.join();
    std::size_t best = 0;
    std::string chosen;
    for (const auto &s : lines) {
        std::istringstream in(s);
        std::string w;
        std::size_t n = 0;
        while (in >> w)
            ++n;
        if (n > best) {
            best = n;
            chosen = s;
        }
    }
    std::cout << "Dong dai nhat: \"" << chosen << "\"; so tu = " << best << '\n';
}
