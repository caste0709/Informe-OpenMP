#include <iostream>
#include <chrono>
#include <random>
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

public:
    Lista() { 
        head = nullptr; 
    }

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
        nodo* temp = head;
        while (temp != nullptr) {
            if (temp->n == valor) {
                return true;
            }
            temp = temp->sgte;
        }
        return false;
    }

    bool eliminar(int valor) {
        if (head == nullptr) {
            return false;
        }

        if (head->n == valor) {
            nodo* temp = head;
            head = head->sgte;
            delete temp;
            return true;
        }

        nodo* temp = head;
        while (temp->sgte != nullptr && temp->sgte->n != valor) {
            temp = temp->sgte;
        }

        if (temp->sgte == nullptr) {
            return false;
        }

        nodo* eliminado = temp->sgte;
        temp->sgte = eliminado->sgte;
        delete eliminado;
        return true;
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

int main() {
    Lista l;

    size_t tam = 100;
    mt19937 gen(42);
    uniform_int_distribution<int> dist(1, 100);

    for(size_t i = 0; i < tam; i++){
        l.insertar(dist(gen));
    }
    l.imprimir();    

    return 0;
}