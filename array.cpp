#include "array.hpp"
#include "engine.hpp"
#include "error.hpp"
#include "common.hpp"

Variable::Variable()
{
        type = int_type;
        value.integer = 0;
}

Variable::Variable(const Variable& var)
{
        type = var.type;
        if (var.type == string_type)
                value.string = dupstr(var.value.string);
        else
                value = var.value;
}

Variable::~Variable()
{
        if (type == string_type)
                delete[] value.string;
}

Variable& Variable::operator=(const Variable& var)
{
        type = var.type;
        if (var.type == string_type)
                value.string = dupstr(var.value.string);
        else
                value = var.value;
        return *this;
}

void Variable::Set(RPNConst *val)
{
        if (type == string_type)
                delete[] value.string;
        RPNBool *b = dynamic_cast<RPNBool*>(val); 
        RPNInt *i = dynamic_cast<RPNInt*>(val);
        RPNDouble *d = dynamic_cast<RPNDouble*>(val);
        RPNString *s = dynamic_cast<RPNString*>(val);
        if (b) {
                type = bool_type;
                value.boolean = b->Get();
        } else if (i) {
                type = int_type;
                value.integer = i->Get();
        } else if (d) {
                type = double_type;
                value.real = d->Get();
        } else if (s) {
                type = string_type;
                value.string = dupstr(s->Get());
        } else {
                type = int_type;
                value.integer = 0;
        }
}

RPNConst *Variable::Get() const
{
        switch (type) {
        case bool_type:
                return new RPNBool(value.boolean);
        case int_type:
                return new RPNInt(value.integer);
        case double_type:
                return new RPNDouble(value.real);
        case string_type:
                return new RPNString(value.string, false, false);
        }
        return 0;
}

Array::Array(unsigned long size)
{
        var = new Variable[size];
        allocated = size;
}

Array::Array(const Array& arr)
{
        var = new Variable[arr.allocated];
        allocated = arr.allocated;
        for (unsigned long i = 0; i < allocated; i++)
                var[i] = arr.var[i];
}

Array::~Array()
{
        delete[] var;
}

void Array::Allocate(unsigned long size)
{
        if (size == 0)
                throw RuntimeError("bad allocation", "Array");
        unsigned long copy = size < allocated ? size : allocated;
        allocated = size;
        Variable *tmp = new Variable[allocated];
        for (unsigned long i = 0; i < copy; i++)
                tmp[i] = var[i];
        delete[] var;
        var = tmp;
}

Variable& Array::operator[](unsigned long index)
{
        if (index >= allocated)
                throw RuntimeError("segmentation fault", "Array");
        return var[index];
}

