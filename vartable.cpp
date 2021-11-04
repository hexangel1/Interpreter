#include "vartable.hpp"

void VarTable::Alloc(const char *name, long size)
{
        if (table.Find(name))
                table[name].Allocate(size);
        else
                table.Add(Array(size), name);
}

void VarTable::Free(const char *name)
{
        table[name].Allocate(1);
        table.Remove(name);
}

void VarTable::SetValue(const char *name, long index, RPNConst *val)
{
        if (!table.Find(name))
                table.Add(Array(1), name);
        table[name][index].Set(val);
}

RPNConst *VarTable::GetValue(const char *name, long index) const
{
        return table[name][index].Get();
}

