#include "labtable.hpp"

bool LabTable::AddLabel(RPNItem *ptr, const char *lab)
{
        return table.Add(ptr, lab);
}

RPNItem *LabTable::GetLabel(const char *lab) const
{
        return table[lab];
}

