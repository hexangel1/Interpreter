#ifndef VARTABLE_HPP_SENTRY
#define VARTABLE_HPP_SENTRY

#include "hashtable.hpp"
#include "array.hpp"

class VarTable {
        HashTable<Array> table;
public:
        VarTable() {}
        void Alloc(const char *name, long size);
        void Free(const char *name);
        void SetValue(const char *name, long index, RPNConst *val);
        RPNConst *GetValue(const char *name, long index) const;
};

#endif

