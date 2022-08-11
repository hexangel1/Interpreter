#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "engine.hpp"
#include "error.hpp"

void RPNElem::Push(RPNItem **stack, RPNElem *unit)
{
        RPNItem *tmp = new RPNItem;
        tmp->elem = unit;
        tmp->next = *stack;
        *stack = tmp;
}

RPNElem *RPNElem::Pop(RPNItem **stack)
{
        if (!*stack)
                return 0;
        RPNElem *unit = (*stack)->elem;
        RPNItem *tmp = *stack;
        *stack = (*stack)->next;
        delete tmp;
        return unit;
}

void RPNJump::Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                       LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNLabel *lab = dynamic_cast<RPNLabel*>(operand1);
        if (!lab)
                throw RuntimeError("operand1 not RPNLabel", "RPNJump");
        *cur_cmd = lab->Get();
        delete operand1;
}

void RPNJumpFalse::Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                            LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *cond = dynamic_cast<RPNValue*>(operand1);
        if (!cond)
                throw RuntimeError("operand1 not RPNValue", "RPNJumpFalse");
        RPNElem *operand2 = Pop(stack);
        RPNLabel *lab = dynamic_cast<RPNLabel*>(operand2);
        if (!lab)
                throw RuntimeError("operand2 not RPNLabel", "RPNJumpFalse");
        *cur_cmd = cond->GetBool() ? (*cur_cmd)->next : lab->Get();
        delete operand1;
        delete operand2;
}

void RPNConst::Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                        LabTable& L, VarTable& V) const
{
        Push(stack, Clone());
        *cur_cmd = (*cur_cmd)->next;
}

void RPNFunction::Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                           LabTable& L, VarTable& V) const
{
        RPNElem *retval = Call(stack, L, V);
        if (retval)
                Push(stack, retval);
        *cur_cmd = (*cur_cmd)->next;
}

RPNElem *RPNFunNull::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        return 0;
}

RPNElem *RPNFunAlloc::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *size = dynamic_cast<RPNValue*>(operand1);
        if (!size)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAlloc");
        RPNElem *operand2 = Pop(stack);
        RPNValue *name = dynamic_cast<RPNValue*>(operand2);
        if (!name)
                throw RuntimeError("operand2 not RPNValue", "RPNFunAlloc");
        V.Alloc(name->GetString(), size->GetInt());
        delete operand1;
        delete operand2;
        return 0;
}

RPNElem *RPNFunFree::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *name = dynamic_cast<RPNValue*>(operand1);
        if (!name)
                throw RuntimeError("operand1 not RPNValue", "RPNFunFree");
        V.Free(name->GetString());
        delete operand1;
        return 0;
}

RPNElem *RPNFunLab::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *name = dynamic_cast<RPNValue*>(operand1);
        if (!name)
                throw RuntimeError("operand1 not RPNValue", "RPNFunLab");
        RPNItem *addr = L.GetLabel(name->GetString());
        delete operand1;
        return new RPNLabel(addr);
}

RPNElem *RPNFunVar::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunVar");
        RPNValue *retval = V.GetValue(addr->Name(), addr->Index());
        delete operand1;
        return retval;
}

RPNElem *RPNFunInc::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunInc");
        RPNValue *value = V.GetValue(addr->Name(), addr->Index());
        RPNValue *q = new RPNValue(value->GetInt() + 1);
        V.SetValue(addr->Name(), addr->Index(), q);
        delete operand1;
        delete q;
        return 0;
}

RPNElem *RPNFunDec::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunDec");
        RPNValue *value = V.GetValue(addr->Name(), addr->Index());
        RPNValue *q = new RPNValue(value->GetInt() - 1);
        V.SetValue(addr->Name(), addr->Index(), q);
        delete operand1;
        delete q;
        return 0;
}

RPNElem *RPNFunAssign::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *value = dynamic_cast<RPNValue*>(operand1);
        if (!value)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAssign");
        RPNElem *operand2 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand2);
        if (!addr)
                throw RuntimeError("operand2 not RPNAddr", "RPNFunAssign");
        V.SetValue(addr->Name(), addr->Index(), value);
        delete operand1;
        delete operand2;
        return 0;
};

RPNElem *RPNFunIndex::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *index = dynamic_cast<RPNValue*>(operand1);
        if (!index)
                throw RuntimeError("operand1 not RPNValue", "RPNFunIndex");
        RPNElem *operand2 = Pop(stack);
        RPNValue *name = dynamic_cast<RPNValue*>(operand2);
        if (!name)
                throw RuntimeError("operand2 not RPNValue", "RPNFunIndex");
        RPNElem *retval = new RPNAddr(name->GetString(), index->GetInt());
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunPlus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunPlus");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunPlus");
        RPNElem *retval;
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunPlus");
        switch (i1->Type()) {
        case int_type:
                retval = new RPNValue(i2->GetInt() + i1->GetInt());
                break;
        case double_type:
                retval = new RPNValue(i2->GetDouble() + i1->GetDouble());
                break;
        case string_type:
                retval = new RPNValue(concatenate(i2->GetString(), 
                                                  i1->GetString()));
                break;
        default:
                throw RuntimeError("data type mismatch", "RPNFunPlus");
        }
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMinus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunMinus");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunMinus");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunMinus");
        RPNElem *retval;
        switch (i1->Type()) {
        case int_type:
                retval = new RPNValue(i2->GetInt() - i1->GetInt());
                break;
        case double_type:
                retval = new RPNValue(i2->GetDouble() - i1->GetDouble());
                break;
        default:
                throw RuntimeError("data type mismatch", "RPNFunMinus");
        }
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMul::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunMul");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunMul");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunMul");  
        RPNElem *retval;
        switch (i1->Type()) {
        case int_type:
                retval = new RPNValue(i2->GetInt() * i1->GetInt());
                break;
        case double_type:
                retval = new RPNValue(i2->GetDouble() * i1->GetDouble());
                break;
        default:
                throw RuntimeError("data type mismatch", "RPNFunMul");
        }
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunDiv::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunDiv");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunDiv");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunDiv");  
        RPNElem *retval;
        switch (i1->Type()) {
        case int_type:
                if (!i1->GetInt())
                        throw RuntimeError("division by zero", "RPNFunDiv");
                retval = new RPNValue(i2->GetInt() / i1->GetInt());
                break;
        case double_type:
                if (!i1->GetDouble())
                        throw RuntimeError("division by zero", "RPNFunDiv");
                retval = new RPNValue(i2->GetDouble() / i1->GetDouble());
                break;
        default:
                throw RuntimeError("data type mismatch", "RPNFunDiv");
        }
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMod::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunMod");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunMod");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunMod");
        if (!i1->GetInt())
                throw RuntimeError("modulo by zero", "RPNFunMod");
        long res = i2->GetInt() % i1->GetInt();
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunUMinus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunUMinus");
        RPNElem *retval;
        switch (i1->Type()) {
        case int_type:
                retval = new RPNValue(-i1->GetInt());
                break;
        case double_type:
                retval = new RPNValue(-i1->GetDouble());
                break;
        default:
                throw RuntimeError("data type mismatch", "RPNFunUMinus");
        }
        delete operand1;
        return retval;
}

RPNElem *RPNFunEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *b1 = dynamic_cast<RPNValue*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunEQ");
        RPNElem *operand2 = Pop(stack);
        RPNValue *b2 = dynamic_cast<RPNValue*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunEQ");
        bool res = (b2->GetBool() && b1->GetBool()) ||
                   (!b2->GetBool() && !b1->GetBool());
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunXOR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *b1 = dynamic_cast<RPNValue*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunXOR");
        RPNElem *operand2 = Pop(stack);
        RPNValue *b2 = dynamic_cast<RPNValue*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunXOR");
        bool res = (b2->GetBool() && !b1->GetBool()) ||
                   (!b2->GetBool() && b1->GetBool());
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunOR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *b1 = dynamic_cast<RPNValue*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunOR");
        RPNElem *operand2 = Pop(stack);
        RPNValue *b2 = dynamic_cast<RPNValue*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunOR");
        bool res = b2->GetBool() || b1->GetBool();
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunAND::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *b1 = dynamic_cast<RPNValue*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAND");
        RPNElem *operand2 = Pop(stack);
        RPNValue *b2 = dynamic_cast<RPNValue*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunAND");
        bool res = b2->GetBool() && b1->GetBool();
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunNOT::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *b1 = dynamic_cast<RPNValue*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunNOT");
        bool res = !b1->GetBool();
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunEQU::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunEQU");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunEQU");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunNEQ");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() == i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() == i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() == i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) == 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunNEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunNEQ");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunNEQ");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunNEQ");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() != i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() != i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() != i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) != 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunGTR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunGTR");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunGTR");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunGTR");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() > i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() > i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() > i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) > 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunLSS::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunLSS");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunLSS");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunLSS");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() < i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() < i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() < i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) < 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunGEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunGEQ");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunGEQ");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunGEQ");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() >= i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() >= i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() >= i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) >= 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunLEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunLEQ");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunLEQ");
        if (i1->Type() != i2->Type())
                throw RuntimeError("data type mismatch", "RPNFunLEQ");  
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i2->GetBool() <= i1->GetBool();
                break;
        case int_type:
                res = i2->GetInt() <= i1->GetInt();
                break;
        case double_type:
                res = i2->GetDouble() <= i1->GetDouble();
                break;
        case string_type:
                res = strcmp(i2->GetString(), i1->GetString()) <= 0;
                break;
        }
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunPrint::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand;
        RPNValue *argv[16];
        int i;
        for (i = 0; (operand = Pop(stack)); i++) {
                argv[i] = dynamic_cast<RPNValue*>(operand);
                if (argv[i])
                        continue;
                throw RuntimeError("operand not RPNValue", "RPNFunPrint");
        }
        for (i--; i >= 0; i--) {
                printf("%s", argv[i]->GetString());
                delete argv[i];
        }
        return 0;
}

RPNElem *RPNFunScan::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *i1 = dynamic_cast<RPNAddr*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunScan");
        char buff[1024];
        fgets(buff, 1023, stdin);
        buff[strlen(buff) - 1] = 0;
        RPNValue *val = new RPNValue(buff);
        V.SetValue(i1->Name(), i1->Index(), val);
        delete val;
        delete operand1;
        return 0;
}

RPNElem *RPNFunCastBool::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCastBool");
        bool res;
        switch (i1->Type()) {
        case bool_type:
                res = i1->GetBool();
                break;
        case int_type:
                res = static_cast<bool>(i1->GetInt());
                break;
        case double_type:
                res = static_cast<bool>(i1->GetDouble());
                break;
        case string_type:
                res = !strcmp(i1->GetString(), "true");
                break;
        }
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunCastInt::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCastInt");
        long res;
        switch (i1->Type()) {
        case bool_type:
                res = static_cast<long>(i1->GetBool());
                break;
        case int_type:
                res = i1->GetInt();
                break;
        case double_type:
                res = static_cast<long>(i1->GetDouble());
                break;
        case string_type:
                res = atol(i1->GetString());
                break;
        }
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunCastDouble::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCastDouble");
        double res;
        switch (i1->Type()) {
        case bool_type:
                res = static_cast<double>(i1->GetBool());
                break;
        case int_type:
                res = static_cast<double>(i1->GetInt());
                break;
        case double_type:
                res = i1->GetDouble();
                break;
        case string_type:
                res = atof(i1->GetString());
                break;
        }
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunCastString::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCastString");
        char res[128];
        switch (i1->Type()) {
        case bool_type:
                sprintf(res, "%s", i1->GetBool() ? "true" : "false");
                break;
        case int_type:
                sprintf(res, "%ld", i1->GetInt());
                break;
        case double_type:
                sprintf(res, "%lf", i1->GetDouble());
                break;
        case string_type:
                return i1;
        }
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunRand::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunRand");
        if (i1->GetInt() < 0)
                throw RuntimeError("operand must be > 0", "RPNFunRand");
        long res = (long)((double)(i1->GetInt() + 1) * rand() / 
                   (double)RAND_MAX);
        return new RPNValue(res);
}

RPNElem *RPNFunAbs::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAbs");
        RPNElem *retval;
        switch (i1->Type()) {
        case int_type:
                retval = new RPNValue(i1->GetInt() >= 0 ?
                                      i1->GetInt() : -i1->GetInt());
                break;
        case double_type:
                retval = new RPNValue(i1->GetDouble() >= 0 ?
                                      i1->GetDouble() : -i1->GetDouble());
                break;
        default:
                 throw RuntimeError("data type mismatch", "RPNFunAbs");
        }
        delete operand1;
        return retval;
}

RPNElem *RPNFunPow::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunPow");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunPow");
        double res = pow(i2->GetDouble(), i1->GetInt());
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunSqrt::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunSqrt");
        double res = sqrt(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunSin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunSin");
        double res = sin(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunCos::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCos");
        double res = cos(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunTan::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunTan");
        double res = tan(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunAsin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAsin");
        double res = asin(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunAcos::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAcos");
        double res = acos(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunAtan::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAtan");
        double res = atan(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunAtan2::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunAtan2");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNValue", "RPNFunAtan2");
        double res = atan2(i2->GetDouble(), i1->GetDouble());
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunExp::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunExp");
        double res = exp(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunLog::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunLog");
        double res = log(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunCeil::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunCeil");
        double res = ceil(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunFloor::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunFloor");
        double res = floor(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunTrunc::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunTrunc");
        double res = trunc(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunRound::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunRound");
        double res = round(i1->GetDouble());
        delete operand1;
        return new RPNValue(res);
}

RPNElem *RPNFunMax::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunMax");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i1)
                throw RuntimeError("operand2 not RPNValue", "RPNFunMax");
        double res = i2->GetDouble() > i1->GetDouble() ?
                     i2->GetDouble() : i1->GetDouble();
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

RPNElem *RPNFunMin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNValue *i1 = dynamic_cast<RPNValue*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNValue", "RPNFunMin");
        RPNElem *operand2 = Pop(stack);
        RPNValue *i2 = dynamic_cast<RPNValue*>(operand2);
        if (!i1)
                throw RuntimeError("operand2 not RPNValue", "RPNFunMin");
        double res = i2->GetDouble() < i1->GetDouble() ? 
                     i2->GetDouble() : i1->GetDouble();
        delete operand1;
        delete operand2;
        return new RPNValue(res);
}

