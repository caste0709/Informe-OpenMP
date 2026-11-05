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
    omp_lock_t cerrojo;

    nodo(int val, nodo* s = nullptr) {
        n = val;
        sgte = s;
        omp_init_lock(&cerrojo);
    }

    ~nodo() { omp_destroy_lock(&cerrojo); }
};

class Lista {
    nodo* head;
    omp_lock_t cerrojoHead;

public:
    Lista() {
        head = nullptr;
        omp_init_lock(&cerrojoHead);
    }

    ~Lista() {
        nodo* temp = head;
        while (temp != nullptr) {
            nodo* sig = temp->sgte;
            delete temp;
            temp = sig;
        }
        omp_destroy_lock(&cerrojoHead);
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
        omp_set_lock(&cerrojoHead);
        nodo* curr = head;
        if (curr == nullptr) {
            omp_unset_lock(&cerrojoHead);
            return false;
        }
        omp_set_lock(&curr->cerrojo);
        omp_unset_lock(&cerrojoHead);

        while (true) {
            if (curr->n == valor) {
                omp_unset_lock(&curr->cerrojo);
                return true;
            }
            nodo* sig = curr->sgte;
            if (sig == nullptr) {
                omp_unset_lock(&curr->cerrojo);
                return false;
            }
            omp_set_lock(&sig->cerrojo);      
            omp_unset_lock(&curr->cerrojo);   
            curr = sig;
        }
    }

    bool eliminar(int valor) {
        omp_set_lock(&cerrojoHead);
        nodo* curr = head;
        if (curr == nullptr) {
            omp_unset_lock(&cerrojoHead);
            return false;
        }
        omp_set_lock(&curr->cerrojo);

        if (curr->n == valor) {
            head = curr->sgte;
            omp_unset_lock(&curr->cerrojo);
            omp_unset_lock(&cerrojoHead);
            delete curr;
            return true;
        }
        omp_unset_lock(&cerrojoHead);


        nodo* prev = curr;
        curr = prev->sgte;
        while (curr != nullptr) {
            omp_set_lock(&curr->cerrojo);
            if (curr->n == valor) {
                prev->sgte = curr->sgte;     
                omp_unset_lock(&curr->cerrojo);
                omp_unset_lock(&prev->cerrojo);
                delete curr;                 
                return true;
            }
            omp_unset_lock(&prev->cerrojo);
            prev = curr;
            curr = prev->sgte;
        }
        omp_unset_lock(&prev->cerrojo);
        return false;
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