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
        if (*stack) {
                RPNElem *unit = (*stack)->elem;
                RPNItem *tmp = *stack;
                *stack = (*stack)->next;
                delete tmp;
                return unit;
        }
        return 0;
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
        RPNBool *cond = dynamic_cast<RPNBool*>(operand1);
        if (!cond)
                throw RuntimeError("operand1 not RPNBool", "RPNJumpFalse");
        RPNElem *operand2 = Pop(stack);
        RPNLabel *lab = dynamic_cast<RPNLabel*>(operand2);
        if (!lab)
                throw RuntimeError("operand2 not RPNLabel", "RPNJumpFalse");
        *cur_cmd = cond->Get() ? (*cur_cmd)->next : lab->Get();
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
        RPNInt *size = dynamic_cast<RPNInt*>(operand1);
        if (!size)
                throw RuntimeError("operand1 not RPNInt", "RPNFunAlloc");
        RPNElem *operand2 = Pop(stack);
        RPNString *name = dynamic_cast<RPNString*>(operand2);
        if (!name)
                throw RuntimeError("operand2 not RPNString", "RPNFunAlloc");
        V.Alloc(name->Get(), size->Get());
        delete operand1;
        delete operand2; 
        return 0;
}

RPNElem *RPNFunFree::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNString *name = dynamic_cast<RPNString*>(operand1);
        if (!name)
                throw RuntimeError("operand1 not RPNString", "RPNFunFree");
        V.Free(name->Get());
        delete operand1;
        return 0;
}

RPNElem *RPNFunLab::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNString *name = dynamic_cast<RPNString*>(operand1);
        if (!name)
                throw RuntimeError("operand1 not RPNString", "RPNFunLab");
        RPNItem *addr = L.GetLabel(name->Get());
        delete operand1;
        return new RPNLabel(addr);
}

RPNElem *RPNFunVar::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunVar");
        RPNConst *retval = V.GetValue(addr->Name(), addr->Index());
        delete operand1;
        return retval;
}

RPNElem *RPNFunInc::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunInc");
        RPNConst *value = V.GetValue(addr->Name(), addr->Index());
        RPNInt *q = dynamic_cast<RPNInt*>(value);
        if (!q)
                throw RuntimeError("var must have integral type", "RPNFunInc");
        q = new RPNInt(q->Get() + 1);
        V.SetValue(addr->Name(), addr->Index(), q);
        delete operand1;
        delete value;
        delete q;
        return 0;
}

RPNElem *RPNFunDec::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNAddr *addr = dynamic_cast<RPNAddr*>(operand1);
        if (!addr)
                throw RuntimeError("operand1 not RPNAddr", "RPNFunDec");
        RPNConst *value = V.GetValue(addr->Name(), addr->Index());
        RPNInt *q = dynamic_cast<RPNInt*>(value);
        if (!q)
                throw RuntimeError("var must have integral type", "RPNFunDec");
        q = new RPNInt(q->Get() - 1);
        V.SetValue(addr->Name(), addr->Index(), q);
        delete operand1;
        delete value;
        delete q;
        return 0;
}

RPNElem *RPNFunAssign::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNConst *value = dynamic_cast<RPNConst*>(operand1);
        if (!value)
                throw RuntimeError("operand1 not RPNConst", "RPNFunAssign");
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
        RPNInt *index = dynamic_cast<RPNInt*>(operand1);
        if (!index)
                throw RuntimeError("operand1 not RPNInt", "RPNFunIndex");
        RPNElem *operand2 = Pop(stack);
        RPNString *name = dynamic_cast<RPNString*>(operand2);
        if (!name)
                throw RuntimeError("operand2 not RPNString", "RPNFunIndex");
        RPNElem *retval = new RPNAddr(name->Get(), index->Get());
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunPlus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunPlus");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunPlus");
        RPNElem *retval;
        if (i1 && i2)
                retval = new RPNInt(i2->Get() + i1->Get());
        else if (d1 && d2)
                retval = new RPNDouble(d2->Get() + d1->Get());
        else if (s1 && s2)
                retval = new RPNString(concatenate(s2->Get(), s1->Get()),
                                       true, false);
        else
                throw RuntimeError("data type mismatch", "RPNFunPlus");
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMinus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!i1 && !d1)
                throw RuntimeError("operand1", "RPNFunMinus");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!i2 && !d2)
                throw RuntimeError("operand2", "RPNFunMinus");
        RPNElem *retval;
        if (i1 && i2)
                retval = new RPNInt(i2->Get() - i1->Get());
        else if (d1 && d2)
                retval = new RPNDouble(d2->Get() - d1->Get());
        else
                throw RuntimeError("data type mismatch", "RPNFunMinus");
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMul::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!i1 && !d1)
                throw RuntimeError("operand1", "RPNFunMul");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!i2 && !d2)
                throw RuntimeError("operand2", "RPNFunMul");
        RPNElem *retval;
        if (i1 && i2)
                retval = new RPNInt(i2->Get() * i1->Get());
        else if (d1 && d2)
                retval = new RPNDouble(d2->Get() * d1->Get());
        else
                throw RuntimeError("data type mismatch", "RPNFunMul");
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunDiv::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!i1 && !d1)
                throw RuntimeError("operand1", "RPNFunDiv");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!i2 && !d2)
                throw RuntimeError("operand2", "RPNFunDiv");
        RPNElem *retval;
        if (i1 && i2) {
                if (!i1->Get())
                        throw RuntimeError("division by zero", "RPNFunDiv");
                retval = new RPNInt(i2->Get() / i1->Get());
        } else if (d1 && d2) {
                if (!d1->Get())
                        throw RuntimeError("division by zero", "RPNFunDiv");
                retval = new RPNDouble(d2->Get() / d1->Get());
        } else {
                throw RuntimeError("data type mismatch", "RPNFunDiv");
        }
        delete operand1;
        delete operand2;
        return retval;
}

RPNElem *RPNFunMod::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNInt", "RPNFunMod");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        if (!i2)
                throw RuntimeError("operand2 not RPNInt", "RPNFunMod");
        if (!i1->Get())
                throw RuntimeError("modulo by zero", "RPNFunMod");
        long res = i2->Get() % i1->Get();
        delete operand1;
        delete operand2;
        return new RPNInt(res);
}

RPNElem *RPNFunUMinus::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!i1 && !d1)
                throw RuntimeError("operand1", "RPNFunUnaryMinus");
        RPNElem *retval;
        if (i1)
                retval = new RPNInt(-i1->Get());
        else
                retval = new RPNDouble(-d1->Get());
        delete operand1;
        return retval;
}

RPNElem *RPNFunEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNBool", "RPNFunEQ");
        RPNElem *operand2 = Pop(stack);
        RPNBool *b2 = dynamic_cast<RPNBool*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNBool", "RPNFunEQ");
        bool res = (b2->Get() && b1->Get()) || (!b2->Get() && !b1->Get());
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunXOR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNBool", "RPNFunXOR");
        RPNElem *operand2 = Pop(stack);
        RPNBool *b2 = dynamic_cast<RPNBool*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNBool", "RPNFunXOR");
        bool res = (b2->Get() && !b1->Get()) || (!b2->Get() && b1->Get());
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunOR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNBool", "RPNFunOR");
        RPNElem *operand2 = Pop(stack);
        RPNBool *b2 = dynamic_cast<RPNBool*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNBool", "RPNFunOR");
        bool res = b2->Get() || b1->Get();
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunAND::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNBool", "RPNFunAND");
        RPNElem *operand2 = Pop(stack);
        RPNBool *b2 = dynamic_cast<RPNBool*>(operand2);
        if (!b2)
                throw RuntimeError("operand2 not RPNBool", "RPNFunAND");
        bool res = b2->Get() && b1->Get();
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunNOT::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        if (!b1)
                throw RuntimeError("operand1 not RPNBool", "RPNFunNOT");
        bool res = !b1->Get();
        delete operand1;
        return new RPNBool(res);
}

RPNElem *RPNFunEQU::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunEQU");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunEQU");
        bool res;
        if (i1 && i2)
                res = i2->Get() == i1->Get();
        else if (d1 && d2)
                res = d2->Get() == d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) == 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunEQU");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunNEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunNEQ");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunNEQ");
        bool res;
        if (i1 && i2)
                res = i2->Get() != i1->Get();
        else if (d1 && d2)
                res = d2->Get() != d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) != 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunNEQ");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunGTR::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunGTR");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunGTR");
        bool res;
        if (i1 && i2)
                res = i2->Get() > i1->Get();
        else if (d1 && d2)
                res = d2->Get() > d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) > 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunGTR");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunLSS::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1); 
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunLSS");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunLSS");
        bool res;
        if (i1 && i2)
                res = i2->Get() < i1->Get();
        else if (d1 && d2)
                res = d2->Get() < d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) < 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunLSS");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunGEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunGEQ");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunGEQ");
        bool res;
        if (i1 && i2)
                res = i2->Get() >= i1->Get();
        else if (d1 && d2)
                res = d2->Get() >= d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) >= 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunGEQ");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunLEQ::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!i1 && !d1 && !s1)
                throw RuntimeError("operand1", "RPNFunLEQ");
        RPNElem *operand2 = Pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt*>(operand2);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        RPNString *s2 = dynamic_cast<RPNString*>(operand2);
        if (!i2 && !d2 && !s2)
                throw RuntimeError("operand2", "RPNFunLEQ");
        bool res;
        if (i1 && i2)
                res = i2->Get() <= i1->Get();
        else if (d1 && d2)
                res = d2->Get() <= d1->Get();
        else if (s1 && s2)
                res = strcmp(s2->Get(), s1->Get()) <= 0;
        else
                throw RuntimeError("data type mismatch", "RPNFunLEQ");
        delete operand1;
        delete operand2;
        return new RPNBool(res);
}

RPNElem *RPNFunPrint::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand;
        RPNString *argv[16];
        int i;
        for (i = 0; (operand = Pop(stack)); i++) {
                argv[i] = dynamic_cast<RPNString*>(operand);
                if (argv[i])
                        continue;
                throw RuntimeError("operand not RPNString", "RPNFunPrint");
        }
        for (i--; i >= 0; i--) {
                printf("%s", argv[i]->Get());
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
        RPNConst *val = new RPNString(buff, false, false);
        V.SetValue(i1->Name(), i1->Index(), val);
        delete val;
        delete operand1;
        return 0;
}

RPNElem *RPNFunCastBool::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!b1 && !i1 && !d1 && !s1)
                throw RuntimeError("Bad cast", "RPNFunBool");
        bool res;
        if (b1)
                res = b1->Get();
        else if (i1)
                res = static_cast<bool>(i1->Get());
        else if (d1)
                res = static_cast<bool>(d1->Get());
        else
                res = !strcmp(s1->Get(), "true");
        delete operand1;
        return new RPNBool(res);
}

RPNElem *RPNFunCastInt::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!b1 && !i1 && !d1 && !s1)
                throw RuntimeError("Bad cast", "RPNFunInt");
        long res;
        if (b1)
                res = static_cast<long>(b1->Get());
        else if (i1)
                res = i1->Get();
        else if (d1)
                res = static_cast<long>(d1->Get());
        else
                res = atol(s1->Get());
        delete operand1;
        return new RPNInt(res);
}

RPNElem *RPNFunCastDouble::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!b1 && !i1 && !d1 && !s1)
                throw RuntimeError("Bad cast", "RPNFunDouble");
        double res;
        if (b1)
                res = static_cast<double>(b1->Get());
        else if (i1)
                res = static_cast<double>(i1->Get());
        else if (d1)
                res = d1->Get();
        else
                res = atof(s1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunCastString::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNBool *b1 = dynamic_cast<RPNBool*>(operand1);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        RPNString *s1 = dynamic_cast<RPNString*>(operand1);
        if (!b1 && !i1 && !d1 && !s1)
                throw RuntimeError("Bad cast", "RPNFunDouble");
        char res[128];
        if (b1)
                sprintf(res, "%s", b1->Get() ? "true" : "false");
        else if (i1)
                sprintf(res, "%ld", i1->Get());
        else if (d1)
                sprintf(res, "%lf", d1->Get());
        else
                return s1;
        delete operand1;
        return new RPNString(res);
}

RPNElem *RPNFunRand::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNInt", "RPNFunRand");
        if (i1->Get() < 0)
                throw RuntimeError("operand must be > 0", "RPNFunRand");
        long res = (long)((double)(i1->Get() + 1) * rand() / (double)RAND_MAX);
        return new RPNInt(res);
}

RPNElem *RPNFunAbs::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!i1 && !d1)
                throw RuntimeError("operand1 not RPNInt", "RPNFunAbs");
        RPNElem *retval;
        if (i1)
                retval = new RPNInt(i1->Get() >= 0 ? i1->Get() : -i1->Get());
        else
                retval = new RPNDouble(d1->Get() >= 0 ? d1->Get() : -d1->Get());
        delete operand1;
        return retval;
}

RPNElem *RPNFunPow::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt*>(operand1);
        if (!i1)
                throw RuntimeError("operand1 not RPNInt", "RPNFunPow");
        RPNElem *operand2 = Pop(stack);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!d2)
                throw RuntimeError("operand2 not RPNDouble", "RPNFunPow");
        double res = pow(d2->Get(), i1->Get());
        delete operand1;
        delete operand2;
        return new RPNDouble(res);
}

RPNElem *RPNFunSqrt::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunSqrt");
        double res = sqrt(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunSin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunSin");
        double res = sin(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunCos::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunCos");
        double res = cos(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunTan::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunTan");
        double res = tan(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunAsin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunAsin");
        double res = asin(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunAcos::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunAcos");
        double res = acos(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunAtan::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunAtan");
        double res = atan(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunAtan2::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunAtan2");
        RPNElem *operand2 = Pop(stack);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!d2)
                throw RuntimeError("operand2 not RPNDouble", "RPNFunAtan2");
        double res = atan2(d2->Get(), d1->Get());
        delete operand1;
        delete operand2;
        return new RPNDouble(res);
}

RPNElem *RPNFunExp::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunExp");
        double res = exp(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunLog::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunLog");
        double res = log(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunCeil::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunCeil");
        double res = ceil(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunFloor::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunFloor");
        double res = floor(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunTrunc::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunTrunc");
        double res = trunc(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunRound::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunRound");
        double res = round(d1->Get());
        delete operand1;
        return new RPNDouble(res);
}

RPNElem *RPNFunMax::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunMax");
        RPNElem *operand2 = Pop(stack);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!d1)
                throw RuntimeError("operand2 not RPNDouble", "RPNFunMax");
        double res = d2->Get() > d1->Get() ? d2->Get() : d1->Get();
        delete operand1;
        delete operand2;
        return new RPNDouble(res);
}

RPNElem *RPNFunMin::Call(RPNItem **stack, LabTable& L, VarTable& V) const
{
        RPNElem *operand1 = Pop(stack);
        RPNDouble *d1 = dynamic_cast<RPNDouble*>(operand1);
        if (!d1)
                throw RuntimeError("operand1 not RPNDouble", "RPNFunMin");
        RPNElem *operand2 = Pop(stack);
        RPNDouble *d2 = dynamic_cast<RPNDouble*>(operand2);
        if (!d1)
                throw RuntimeError("operand2 not RPNDouble", "RPNFunMin");
        double res = d2->Get() < d1->Get() ? d2->Get() : d1->Get();
        delete operand1;
        delete operand2;
        return new RPNDouble(res);
}

