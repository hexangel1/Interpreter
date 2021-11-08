#ifndef HASHTABLE_HPP_SENTRY
#define HASHTABLE_HPP_SENTRY

#include <cstring>
#include "common.hpp"
#include "error.hpp"

template <class T>
class HashTable {
        struct Node {
                T data;
                const char *key;
                bool is_deleted;
                Node(const T& val, const char *str)
                        : data(val), key(dupstr(str)), is_deleted(false) {}
                ~Node() { delete[] key; }
        };
        Node **array;
        int array_size;
        int array_used;
        int not_deleted;
        static const int initial_size;
        static const double rehash_size;
public:
        HashTable();
        ~HashTable();
        bool Add(const T& data, const char *key);
        bool Remove(const char *key);
        bool Find(const char *key) const;
        T& operator[](const char *key) const;
private:
        void Resize();
        void Rehash();
        static int Hash(const char *key, int size, int k);
};

template <class T>
const int HashTable<T>::initial_size = 8;

template <class T>
const double HashTable<T>::rehash_size = 0.75;

template <class T>
HashTable<T>::HashTable()
{
        array_size = initial_size;
        array_used = 0;
        not_deleted = 0;
        array = new Node*[initial_size];
        for (int i = 0; i < array_size; i++)
                array[i] = 0;
}

template <class T>
HashTable<T>::~HashTable()
{
        for (int i = 0; i < array_size; i++) {
                if (array[i])
                        delete array[i];
        }
        delete[] array;
}

template <class T>
bool HashTable<T>::Add(const T& data, const char *key)
{
        if (not_deleted + 1 > int(rehash_size * array_size))
                Resize();
        else if (array_used > 2 * not_deleted)
                Rehash();
        int h1 = Hash(key, array_size, array_size - 1);
        int h2 = Hash(key, array_size, array_size + 1);
        int first_deleted = -1;
        for (int i = 0; i < array_size && array[h1]; i++) {
                if (!strcmp(array[h1]->key, key) && !array[h1]->is_deleted)
                        return false;
                if (array[h1]->is_deleted && first_deleted == -1)
                        first_deleted = h1;
                h1 = (h1 + h2) % array_size;
        }
        if (first_deleted == -1) {
                array[h1] = new Node(data, key);
                array_used++;
        } else {
                delete array[first_deleted];
                array[first_deleted] = new Node(data, key);
        }
        not_deleted++;
        return true;
}

template <class T>
bool HashTable<T>::Remove(const char *key)
{
        int h1 = Hash(key, array_size, array_size - 1);
        int h2 = Hash(key, array_size, array_size + 1);
        for (int i = 0; i < array_size && array[h1]; i++) {
                if (!strcmp(array[h1]->key, key)) {
                        if (array[h1]->is_deleted)
                                return false;
                        array[h1]->is_deleted = true;
                        not_deleted--;
                        return true;
                }
                h1 = (h1 + h2) % array_size;
        }
        return false;
}

template <class T>
bool HashTable<T>::Find(const char *key) const
{
        int h1 = Hash(key, array_size, array_size - 1);
        int h2 = Hash(key, array_size, array_size + 1);
        for (int i = 0; i < array_size && array[h1]; i++) {
                if (!strcmp(array[h1]->key, key) && !array[h1]->is_deleted)
                        return true;
                h1 = (h1 + h2) % array_size;
        }
        return false;
}

template <class T>
T& HashTable<T>::operator[](const char *key) const
{
        int h1 = Hash(key, array_size, array_size - 1);
        int h2 = Hash(key, array_size, array_size + 1);
        for (int i = 0; i < array_size && array[h1]; i++) {
                if (!strcmp(array[h1]->key, key) && !array[h1]->is_deleted)
                        return array[h1]->data;
                h1 = (h1 + h2) % array_size;
        }
        throw RuntimeError("not found in table", key);
}

template <class T>
void HashTable<T>::Resize()
{
        int old_array_size = array_size;
        array_size <<= 1;
        array_used = 0;
        not_deleted = 0;
        Node **array2 = new Node*[array_size];
        for (int i = 0; i < array_size; ++i)
                array2[i] = 0;
        Node **tmp = array;
        array = array2;
        array2 = tmp;
        for (int i = 0; i < old_array_size; i++) {
                if (array2[i] && !array2[i]->is_deleted)
                        Add(array2[i]->data, array2[i]->key);
        }
        for (int i = 0; i < old_array_size; i++) {
                if (array2[i])
                        delete array2[i];
        }
        delete[] array2;
}

template <class T>
void HashTable<T>::Rehash()
{
        array_used = 0;
        not_deleted = 0;
        Node **array2 = new Node*[array_size];
        for (int i = 0; i < array_size; ++i)
                array2[i] = 0;
        Node **tmp = array;
        array = array2;
        array2 = tmp;
        for (int i = 0; i < array_size; i++) {
                if (array2[i] && !array2[i]->is_deleted)
                        Add(array2[i]->data, array2[i]->key);
        }
        for (int i = 0; i < array_size; i++) {
                if (array2[i])
                        delete array2[i];
        }
        delete[] array2;

}

template <class T>
int HashTable<T>::Hash(const char *key, int size, int k)
{
        int hash = 0;
        for (int i = 0; key[i]; i++)
                hash = (hash * k + key[i]) % size;
        hash = (hash * 2 + 1) % size;
        return hash;
}

#endif

