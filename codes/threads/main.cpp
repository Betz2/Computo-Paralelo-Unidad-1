#include <iostream>
#include <thread>
#include <array>
#include <random>
#include <algorithm>

class Summation {
    int number;  // id del "thread"
    int total;   // suma acumulada
public:
    Summation() : number(0), total(0) {}
    explicit Summation(int num) : number(num), total(0) {}

    void doSum() {
        
        std::mt19937 gen(12345u + static_cast<unsigned>(number));
        std::uniform_int_distribution<int> dist(1, 1000);

        int acc = 0;
        for (int i = 0; i < 100; ++i) {
            acc += dist(gen);
        }
        total = acc; 
    }

    int getNumber() const { return number; }
    int getTotal()  const { return total;  }
    void setNumber(int n) { number = n;    }

    static bool compare(const Summation& a, const Summation& b) {
        return a.getTotal() > b.getTotal();
    }
};

int main() {
    const int N = 10;

    std::array<Summation, N> sums;
    std::array<std::thread,  N> ths;

    for (int i = 0; i < N; ++i) {
        sums[i].setNumber(i + 1);
    }

    for (int i = 0; i < N; ++i) {
        ths[i] = std::thread(&Summation::doSum, &sums[i]);
    }

    for (int i = 0; i < N; ++i) ths[i].join();

    for (int i = 0; i < N; ++i) {
        std::cout << "El thread #" << sums[i].getNumber()
                  << " sumo: " << sums[i].getTotal() << "\n";
    }

    // se ordena y muestra el mayor
    std::sort(sums.begin(), sums.end(), Summation::compare);
    std::cout << "El thread con mayor puntuacion fue el #"
              << sums[0].getNumber()
              << " y sumo: " << sums[0].getTotal() << "\n";

    return 0;
}
