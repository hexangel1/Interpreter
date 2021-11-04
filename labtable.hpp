#ifndef LABTABLE_HPP_SENTRY
#define LABTABLE_HPP_SENTRY

#include "hashtable.hpp"

class RPNItem;

class LabTable {
        HashTable<RPNItem*> table;
public:
        LabTable() {}
        bool AddLabel(RPNItem *ptr, const char *lab);
        RPNItem *GetLabel(const char *lab) const;
};

#endif

