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
                delete []value.string;
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

void Variable::Set(RPNValue *val)
{
        if (type == string_type)
                delete []value.string;
        switch (val->Type()) {
        case bool_type:
                type = bool_type;
                value.boolean = val->GetBool();
                break;
        case int_type:
                type = int_type;
                value.integer = val->GetInt();
                break;
        case double_type:
                type = double_type;
                value.real = val->GetDouble();
                break;
        case string_type:
                type = string_type;
                value.string = dupstr(val->GetString());
        }
}

RPNValue *Variable::Get() const
{
        switch (type) {
        case bool_type:
                return new RPNValue(value.boolean);
        case int_type:
                return new RPNValue(value.integer);
        case double_type:
                return new RPNValue(value.real);
        case string_type:
                return new RPNValue(value.string);
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

