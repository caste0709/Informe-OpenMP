#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <omp.h>

using namespace std;

class nodo {
public:
    int n;
    nodo* sgte;

    nodo(int val, nodo* s = nullptr) {
        n = val;
        sgte = s;
    }
};

class Lista {
    nodo* head;
    omp_lock_t candado;

public:
    Lista() {
        head = nullptr;
        omp_init_lock(&candado);
    }

    ~Lista() {
        nodo* temp = head;
        while (temp != nullptr) {
            nodo* sig = temp->sgte;
            delete temp;
            temp = sig;
        }
        omp_destroy_lock(&candado);
    }

    Lista(const Lista&) = delete;
    Lista& operator=(const Lista&) = delete;

    void insertar(int valor) {
        nodo* nuevo = new nodo(valor);

        if (head == nullptr) {
            head = nuevo;
            return;
        }

        nodo* temp = head;
        while (temp->sgte != nullptr) {
            temp = temp->sgte;
        }
        temp->sgte = nuevo;
    }

    bool buscar(int valor) {
        bool encontrado = false;

        omp_set_lock(&candado);
        nodo* temp = head;
        while (temp != nullptr) {
            if (temp->n == valor) {
                encontrado = true;
                break;
            }
            temp = temp->sgte;
        }
        omp_unset_lock(&candado);

        return encontrado;
    }

    bool eliminar(int valor) {
        bool eliminado = false;

        omp_set_lock(&candado);

        if (head != nullptr) {
            if (head->n == valor) {
                nodo* temp = head;
                head = head->sgte;
                delete temp;
                eliminado = true;
            } else {
                nodo* prev = head;
                while (prev->sgte != nullptr && prev->sgte->n != valor) {
                    prev = prev->sgte;
                }
                if (prev->sgte != nullptr) {
                    nodo* objetivo = prev->sgte;
                    prev->sgte = objetivo->sgte;   
                    delete objetivo;               
                    eliminado = true;
                }
            }
        }

        omp_unset_lock(&candado);
        return eliminado;
    }

    void imprimir() {
        nodo* temp = head;
        while (temp != nullptr) {
            cout << temp->n << " -> ";
            temp = temp->sgte;
        }
        cout << "NULL" << endl;
    }
};

struct Operacion {
    bool esBusqueda;
    int valor;
};

int main() {
    Lista l;

    size_t tam = 100;
    mt19937 gen(42);
    uniform_int_distribution<int> dist(1, 100);

    for (size_t i = 0; i < tam; i++) {
        l.insertar(dist(gen));
    }
    cout << "Lista inicial:" << endl;
    l.imprimir();

    const long nops = 200;
    mt19937 genOps(7);
    uniform_int_distribution<int> distValor(1, 100);
    uniform_int_distribution<int> distTipo(0, 1);

    vector<Operacion> ops(nops);
    for (auto& op : ops) {
        op.esBusqueda = (distTipo(genOps) == 0);
        op.valor = distValor(genOps);
    }

    int hilos = omp_get_max_threads();
    vector<long> opsPorHilo(hilos, 0);
    long encontradas = 0, eliminadas = 0;

    auto t0 = chrono::steady_clock::now();

    #pragma omp parallel for schedule(static) reduction(+:encontradas, eliminadas)
    for (long i = 0; i < nops; i++) {
        opsPorHilo[omp_get_thread_num()]++;

        if (ops[i].esBusqueda) {
            if (l.buscar(ops[i].valor)) encontradas++;
        } else {
            if (l.eliminar(ops[i].valor)) eliminadas++;
        }
    }

    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();

    cout << "\nLista final:" << endl;
    l.imprimir();

    cout << "\nHilos usados: " << hilos << endl;
    for (int h = 0; h < hilos; h++) {
        cout << "  hilo " << h << ": " << opsPorHilo[h] << " operaciones" << endl;
    }
    cout << "Busquedas: " << encontradas << endl;
    cout << "Eliminaciones: " << eliminadas << endl;
    cout << "Tiempo: " << ms << " ms" << endl;

    return 0;
}