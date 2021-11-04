#ifndef HASHTABLE_HPP_SENTRY
#define HASHTABLE_HPP_SENTRY

#include <cstring>
#include "common.hpp"
#include "error.hpp"

template <class T>
class HashTable {
        struct Node {
                T value;
                bool state;
                const char *key;
                Node(const T& val, const char *s)
                        : value(val), state(true) { key = dupstr(s); }
                ~Node() { delete[] key; }
        };
        Node **array;
        int buffer_size;
        int buffer_used;
        int not_deleted;
        static const int default_size;
        static const double rehash_size;
public:
        HashTable();
        ~HashTable();
        bool Add(const T& value, const char *key);
        bool Remove(const char *key);
        bool Find(const char *key) const;
        T& operator[](const char *key) const;
private:
        void Resize();
        void Rehash();
        static int Hash(const char *key, int size, int size2);
};

template <class T>
const int HashTable<T>::default_size = 8;

template <class T>
const double HashTable<T>::rehash_size = 0.75;

template <class T>
HashTable<T>::HashTable()
{
        buffer_size = default_size;
        buffer_used = 0;
        not_deleted = 0;
        array = new Node*[buffer_size];
        for (int i = 0; i < buffer_size; i++)
                array[i] = 0;
}

template <class T>
HashTable<T>::~HashTable()
{
        for (int i = 0; i < buffer_size; i++) {
                if (array[i])
                        delete array[i];
        }
        delete[] array;
}

template <class T>
bool HashTable<T>::Add(const T& value, const char *key)
{
        if (not_deleted + 1 > int(rehash_size * buffer_size))
                Resize();
        else if (buffer_used > 2 * not_deleted)
                Rehash();
        int h1 = Hash(key, buffer_size, buffer_size - 1);
        int h2 = Hash(key, buffer_size, buffer_size + 1);
        int i = 0;
        int first_deleted = -1;
        while (array[h1] && i < buffer_size) {
                if (!strcmp(array[h1]->key, key) && array[h1]->state)
                        return false;
                if (!array[h1]->state && first_deleted == -1)
                        first_deleted = h1;
                h1 = (h1 + h2) % buffer_size;
                i++;
        }
        if (first_deleted == -1) {
                array[h1] = new Node(value, key);
                buffer_used++;
        } else {
                delete array[first_deleted];
                array[first_deleted] = new Node(value, key);
        }
        not_deleted++;
        return true;
}

template <class T>
bool HashTable<T>::Remove(const char *key)
{
        int h1 = Hash(key, buffer_size, buffer_size - 1);
        int h2 = Hash(key, buffer_size, buffer_size + 1);
        int i = 0;
        while (array[h1] && i < buffer_size) {
                if (!strcmp(array[h1]->key, key) && array[h1]->state) {
                        array[h1]->state = false;
                        not_deleted--;
                        return true;
                }
                h1 = (h1 + h2) % buffer_size;
                i++;
        }
        return false;
}

template <class T>
bool HashTable<T>::Find(const char *key) const
{
        int h1 = Hash(key, buffer_size, buffer_size - 1);
        int h2 = Hash(key, buffer_size, buffer_size + 1);
        int i = 0;
        while (array[h1] && i < buffer_size) {
                if (!strcmp(array[h1]->key, key) && array[h1]->state)
                        return true;
                h1 = (h1 + h2) % buffer_size;
                i++;
        }
        return false;
}

template <class T>
T& HashTable<T>::operator[](const char *key) const
{
        int h1 = Hash(key, buffer_size, buffer_size - 1);
        int h2 = Hash(key, buffer_size, buffer_size + 1);
        int i = 0;
        while (array[h1] && i < buffer_size) {
                if (!strcmp(array[h1]->key, key) && array[h1]->state)
                        return array[h1]->value;
                h1 = (h1 + h2) % buffer_size;
                i++;
        }
        throw RuntimeError("not found in table", key);
}

template <class T>
void HashTable<T>::Resize()
{
        int old_buffer_size = buffer_size;
        buffer_size <<= 1;
        buffer_used = 0;
        not_deleted = 0;
        Node **array2 = new Node*[buffer_size];
        for (int i = 0; i < buffer_size; ++i)
                array2[i] = 0;
        Node **tmp = array;
        array = array2;
        array2 = tmp;
        for (int i = 0; i < old_buffer_size; i++) {
                if (array2[i] && array2[i]->state)
                        Add(array2[i]->value, array2[i]->key);
        }
        for (int i = 0; i < old_buffer_size; i++) {
                if (array2[i])
                        delete array2[i];
        }
        delete[] array2;
}

template <class T>
void HashTable<T>::Rehash()
{
        buffer_used = 0;
        not_deleted = 0;
        Node **array2 = new Node*[buffer_size];
        for (int i = 0; i < buffer_size; ++i)
                array2[i] = 0;
        Node **tmp = array;
        array = array2;
        array2 = tmp;
        for (int i = 0; i < buffer_size; i++) {
                if (array2[i] && array2[i]->state)
                        Add(array2[i]->value, array2[i]->key);
        }
        for (int i = 0; i < buffer_size; i++) {
                if (array2[i])
                        delete array2[i];
        }
        delete[] array2;

}

template <class T>
int HashTable<T>::Hash(const char *key, int size, int size2)
{
        int hash = 0;
        for (int i = 0; key[i]; i++)
                hash = (size2 * hash + key[i]) % size;
        return (hash * 2 + 1) % size;

}

#endif

