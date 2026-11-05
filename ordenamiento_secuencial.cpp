#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

void odd_even_sort(vector<int>& a) {
    int n = (int)a.size();
    int tmp;

    for (int phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            for (int i = 1; i < n; i += 2) {
                if (a[i-1] > a[i]) {
                    tmp = a[i-1]; a[i-1] = a[i]; a[i] = tmp;
                }
            }
        } else {
            for (int i = 1; i < n-1; i += 2) {
                if (a[i] > a[i+1]) {
                    tmp = a[i+1]; a[i+1] = a[i]; a[i] = tmp;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    int n = 20000;
    if (argc > 1) n = stoi(argv[1]);

    mt19937 gen(42);
    uniform_int_distribution<int> dist(1, 1000000);
    vector<int> a(n);
    for (auto& x : a) x = dist(gen);

    vector<int> copia = a;

    if (n <= 20) {
        cout << "Antes:   ";
        for (int x : a) cout << x << " ";
        cout << endl;
    }

    auto t0 = chrono::steady_clock::now();
    odd_even_sort(a);
    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();

    if (n <= 20) {
        cout << "Despues: ";
        for (int x : a) cout << x << " ";
        cout << endl;
    }

    sort(copia.begin(), copia.end());
    cout << "n = " << n << endl;
    cout << "Ordenado correctamente: " << (a == copia ? "si" : "NO") << endl;
    cout << "Tiempo: " << ms << " ms" << endl;

    return 0;
}