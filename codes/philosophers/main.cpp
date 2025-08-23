#include <iostream>
#include <thread>
#include <memory>
#include <string>
#include <mutex>
#include <chrono>
#include <array>
#include <vector>
#include <condition_variable>

struct Fork { std::mutex m; };

static const int N = 4;

std::mutex g;
std::condition_variable cv;
int rightsHeld = 0;
std::vector<bool> hasRight(N, false);
int breaker = 0;        // el que “suelta” su derecho para evitar bloqueo
int turn = -1;          // turno de quién come
int finished = 0;

class Philosopher {
    int id;
    std::string name;
    std::shared_ptr<Fork> left, right;
public:
    Philosopher(int i, std::string n) : id(i), name(std::move(n)) {}

    void setForks(std::shared_ptr<Fork> L, std::shared_ptr<Fork> R) { left = L; right = R; }

    void runOnce() {
        // todos tienen el tenedor derecho
        {
            std::unique_lock<std::mutex> lock(g);
            hasRight[id] = true;
            rightsHeld++;
            std::cout << name << " tiene el tenedor derecho\n";

            if (rightsHeld == N) {
                int neighbor = (breaker + 1) % N;
                std::cout << "Todos tienen el tenedor derecho, "
                          << name_of(breaker) << " lo suelta y permite que "
                          << name_of(neighbor) << " tome su izquierdo.\n";
                hasRight[breaker] = false;
                rightsHeld--;
                turn = neighbor;
                cv.notify_all();
            } else {
                cv.wait(lock, [] { return turn != -1; });
            }
        }

        // Espera turno para comer
        {
            std::unique_lock<std::mutex> lock(g);
            cv.wait(lock, [&]{ return turn == id; });
        }

        // tomar ambos tenedores
        {
            std::scoped_lock both(left->m, right->m);
            std::cout << name << " empezo a comer\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << name << " termino de comer\n";
        } // aquí ya se soltaron los dos tenedores

        // tenedores disponibles
        std::cout << name << " solto ambos tenedores sobre la mesa\n";

        // pasa turno y termina
        {
            std::lock_guard<std::mutex> lock(g);
            finished++;
            if (finished < N) { turn = (turn + 1) % N; cv.notify_all(); }
        }
    }

    static const char* name_of(int i) {
        static const char* names[N] = {"Shrek","Donkey","Fiona","Puss"};
        return names[i];
    }
};

int main() {
    auto f1 = std::make_shared<Fork>();
    auto f2 = std::make_shared<Fork>();
    auto f3 = std::make_shared<Fork>();
    auto f4 = std::make_shared<Fork>();

    Philosopher shrek(0, "Shrek");
    Philosopher donkey(1, "Donkey");
    Philosopher fiona(2, "Fiona");
    Philosopher puss (3, "Puss");

    shrek.setForks(f1, f2);
    donkey.setForks(f2, f3);
    fiona.setForks(f3, f4);
    puss.setForks (f4, f1);

    std::thread t1([&]{ shrek.runOnce();   });
    std::thread t2([&]{ donkey.runOnce();  });
    std::thread t3([&]{ fiona.runOnce();   });
    std::thread t4([&]{ puss.runOnce();    });

    t1.join(); t2.join(); t3.join(); t4.join();
    std::cout << "Todos terminaron.\n";
    return 0;
}
