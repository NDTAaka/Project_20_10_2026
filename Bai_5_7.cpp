#include <array>
#include <iostream>
#include <random>
#include <thread>
int main() {
    constexpr int N = 8;
    std::array<int, N> A{}, B{};
    std::thread t1([&] {
        std::mt19937 g(11);
        for (auto &x : A)
            x = int(g() % 10);
        std::cout << "Task 1 tao A xong\n";
    });
    std::thread t2([&] {
        std::mt19937 g(22);
        for (auto &x : B)
            x = int(g() % 10);
        std::cout << "Task 2 tao B xong\n";
    });
    t1.join();
    t2.join();
    long long dot = 0;
    for (int i = 0; i < N; ++i)
        dot += 1LL * A[i] * B[i];
    std::cout << "A: ";
    for (int x : A)
        std::cout << x << ' ';
    std::cout << "\nB: ";
    for (int x : B)
        std::cout << x << ' ';
    std::cout << "\nTask 3 (sau join): A.B = " << dot << '\n';
}
