#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <omp.h>

using namespace std;


void odd_even_sort(vector<int>& a, int thread_count) {
    int n = (int)a.size();
    int i, tmp, phase;

#   pragma omp parallel num_threads(thread_count) \
        default(none) shared(a, n) private(i, tmp, phase)
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
#           pragma omp for
            for (i = 1; i < n; i += 2) {
                if (a[i-1] > a[i]) {
                    tmp = a[i-1]; a[i-1] = a[i]; a[i] = tmp;
                }
            }
        } else {
#           pragma omp for
            for (i = 1; i < n-1; i += 2) {
                if (a[i] > a[i+1]) {
                    tmp = a[i+1]; a[i+1] = a[i]; a[i] = tmp;
                }
            }
        }
    }
}


vector<int> generarAleatorio(int n, unsigned semilla) {
    mt19937 gen(semilla);
    uniform_int_distribution<int> dist(1, 1000000);
    vector<int> a(n);
    for (auto& x : a) x = dist(gen);
    return a;
}

vector<int> generarDuplicados(int n, unsigned semilla) {
    mt19937 gen(semilla);
    uniform_int_distribution<int> dist(1, 10);
    vector<int> a(n);
    for (auto& x : a) x = dist(gen);
    return a;
}

vector<int> generarInverso(int n, unsigned) {
    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = n - i;
    return a;
}

bool pruebasCorreccion() {
    struct Tipo {
        string nombre;
        function<vector<int>(int, unsigned)> gen;
    };
    vector<Tipo> tipos = {
        {"aleatorio", generarAleatorio},
        {"duplicados", generarDuplicados},
        {"inverso", generarInverso}
    };
    vector<int> tamanos = {1, 2, 7, 8, 100, 101, 1000, 1001};
    vector<int> hilos = {1, 2, 3, 4, 8};

    cout << "=== Pruebas de correccion ===" << endl;
    cout << left << setw(12) << "tipo" << setw(8) << "n" << setw(10) << "paridad"
         << "resultado (hilos 1,2,3,4,8)" << endl;

    bool todoOk = true;
    for (const auto& t : tipos) {
        for (int n : tamanos) {
            vector<int> original = t.gen(n, 42);
            vector<int> esperado = original;
            sort(esperado.begin(), esperado.end());

            bool ok = true;
            for (int h : hilos) {
                vector<int> a = original;
                odd_even_sort(a, h);
                if (a != esperado) ok = false;
            }
            todoOk = todoOk && ok;
            cout << left << setw(12) << t.nombre << setw(8) << n
                 << setw(10) << (n % 2 == 0 ? "par" : "impar")
                 << (ok ? "OK" : "FALLO") << endl;
        }
    }
    cout << "Resultado global: " << (todoOk ? "TODAS LAS PRUEBAS OK" : "HAY FALLOS") << "\n" << endl;
    return todoOk;
}

double medir(const vector<int>& original, int hilos, int repeticiones) {
    double mejor = 1e30;
    for (int r = 0; r < repeticiones; r++) {
        vector<int> a = original;
        double t0 = omp_get_wtime();
        odd_even_sort(a, hilos);
        double t = omp_get_wtime() - t0;
        mejor = min(mejor, t);
    }
    return mejor;
}

void mediciones() {
    vector<int> tamanos = {5000, 10000, 20000};
    vector<int> hilos = {1, 2, 4, 8};
    const int repeticiones = 3;

    cout << "=== Mediciones (arreglo aleatorio, mejor de " << repeticiones << " ejecuciones) ===" << endl;
    cout << "Procesadores disponibles: " << omp_get_num_procs() << endl << endl;

    cout << right << setw(8) << "n" << setw(9) << "hilos" << setw(14) << "tiempo (s)"
         << setw(11) << "speedup" << setw(13) << "eficiencia" << endl;

    for (int n : tamanos) {
        vector<int> original = generarAleatorio(n, 42);

        double t1 = medir(original, 1, repeticiones);   

        for (int h : hilos) {
            double tp = (h == 1) ? t1 : medir(original, h, repeticiones);
            double speedup = t1 / tp;
            double eficiencia = speedup / h;
            cout << setw(8) << n << setw(9) << h << fixed << setprecision(4)
                 << setw(14) << tp << setprecision(2)
                 << setw(10) << speedup << "x" << setw(12) << eficiencia * 100 << "%" << endl;
        }
        cout << endl;
    }
}

int main() {
    bool ok = pruebasCorreccion();
    if (!ok) {
        cout << "Se omiten las mediciones porque hay fallos de correccion." << endl;
        return 1;
    }
    mediciones();
    return 0;
}