#ifndef ENGINE_HPP_SENTRY
#define ENGINE_HPP_SENTRY

#include "vartable.hpp"
#include "labtable.hpp"
#include "common.hpp"
#include "error.hpp"

struct RPNItem {
        class RPNElem *elem;
        RPNItem *next;
};

class RPNElem {
public:
        virtual ~RPNElem() {}
        virtual void Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                              LabTable& L, VarTable& V) const = 0;
protected:
        static void Push(RPNItem **stack, RPNElem *unit);
        static RPNElem *Pop(RPNItem **stack);
};

class RPNJump : public RPNElem {
public:
        RPNJump() {}
        virtual ~RPNJump() {}
        virtual void Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                              LabTable& L, VarTable& V) const;
};

class RPNJumpFalse : public RPNElem {
public:
        RPNJumpFalse() {}
        virtual ~RPNJumpFalse() {}
        virtual void Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                              LabTable& L, VarTable& V) const;
};

class RPNConst : public RPNElem {
public:
        virtual ~RPNConst() {}
        virtual void Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                              LabTable& L, VarTable& V) const;
        virtual RPNElem *Clone() const = 0;
};

class RPNAddr : public RPNConst {
        const char *name;
        long index;
public:
        RPNAddr(const char *str, long num = 0) { name = dupstr(str); index = num; }
        virtual ~RPNAddr() { delete []name; }
        virtual RPNElem *Clone() const { return new RPNAddr(name, index); }
        const char *Name() const { return name; }
        long Index() const { return index; }
};

class RPNLabel : public RPNConst {
        RPNItem *value;
public:
        RPNLabel(RPNItem *val) { value = val; }
        virtual ~RPNLabel() {}
        virtual RPNElem *Clone() const { return new RPNLabel(value); }
        RPNItem *Get() const { return value; }
};

class RPNValue : public RPNConst {
        DataType type;
        union {
                bool boolean;
                long integer;
                double real;
                char *string;
        } value;
public:
        RPNValue(bool val) : type(bool_type) { value.boolean = val; }
        RPNValue(long val) : type(int_type) { value.integer = val; }
        RPNValue(double val) : type(double_type) { value.real = val; }
        RPNValue(const char *val) : type(string_type)
                { value.string = dupstr(val); }
        RPNValue(const RPNValue &RPNVal) : type(RPNVal.type) {
                if (type != string_type)
                        value = RPNVal.value;
                else
                        value.string = dupstr(RPNVal.value.string);
        }
        virtual ~RPNValue() { if (type == string_type) delete []value.string; }
        virtual RPNElem *Clone() const { return new RPNValue(*this); }
        DataType Type() const { return type; }
        bool GetBool() const {
                if (type != bool_type)
                        throw RuntimeError("RPNValue", "data type mismatch");
                return value.boolean;
        }
        long GetInt() const {
                if (type != int_type)
                        throw RuntimeError("RPNValue", "data type mismatch");
                return value.integer; 
        }
        double GetDouble() const {
                if (type != double_type)
                        throw RuntimeError("RPNValue", "data type mismatch");
                return value.real;
        }
        const char *GetString() const {
                if (type != string_type)
                        throw RuntimeError("RPNValue", "data type mismatch");
                return value.string;
        }
};

class RPNFunction : public RPNElem {
public:
        virtual ~RPNFunction() {}
        virtual void Evaluate(RPNItem **cur_cmd, RPNItem **stack,
                              LabTable& L, VarTable& V) const;
        virtual RPNElem *Call(RPNItem **stack,
                              LabTable& L, VarTable& V) const = 0;
};

class RPNFunNull : public RPNFunction {
public:
        RPNFunNull() {}
        virtual ~RPNFunNull() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAlloc : public RPNFunction {
public:
        RPNFunAlloc() {}
        virtual ~RPNFunAlloc() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunFree : public RPNFunction {
public:
        RPNFunFree() {}
        virtual ~RPNFunFree() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunLab : public RPNFunction {
public:
        RPNFunLab() {}
        virtual ~RPNFunLab() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunVar : public RPNFunction {
public:
        RPNFunVar() {}
        virtual ~RPNFunVar() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunInc : public RPNFunction {
public:
        RPNFunInc() {}
        virtual ~RPNFunInc() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunDec : public RPNFunction {
public:
        RPNFunDec() {}
        virtual ~RPNFunDec() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAssign : public RPNFunction {
public:
        RPNFunAssign() {}
        virtual ~RPNFunAssign() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunIndex : public RPNFunction {
public:
        RPNFunIndex() {}
        virtual ~RPNFunIndex() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunPlus : public RPNFunction {
public:
        RPNFunPlus() {}
        virtual ~RPNFunPlus() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunMinus : public RPNFunction {
public:
        RPNFunMinus() {}
        virtual ~RPNFunMinus() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunMul : public RPNFunction {
public:
        RPNFunMul() {}
        virtual ~RPNFunMul() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunDiv : public RPNFunction {
public:
        RPNFunDiv() {}
        virtual ~RPNFunDiv() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunMod : public RPNFunction {
public:
        RPNFunMod() {}
        virtual ~RPNFunMod() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunUMinus : public RPNFunction {
public:
        RPNFunUMinus() {}
        virtual ~RPNFunUMinus() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunEQ : public RPNFunction {
public:
        RPNFunEQ() {}
        virtual ~RPNFunEQ() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunXOR : public RPNFunction {
public:
        RPNFunXOR() {}
        virtual ~RPNFunXOR() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunOR : public RPNFunction {
public:
        RPNFunOR() {}
        virtual ~RPNFunOR() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAND : public RPNFunction {
public:
        RPNFunAND() {}
        virtual ~RPNFunAND() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunNOT : public RPNFunction {
public:
        RPNFunNOT() {}
        virtual ~RPNFunNOT() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunEQU : public RPNFunction {
public:
        RPNFunEQU() {}
        virtual ~RPNFunEQU() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunNEQ : public RPNFunction {
public:
        RPNFunNEQ() {}
        virtual ~RPNFunNEQ() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunGTR : public RPNFunction {
public:
        RPNFunGTR() {}
        virtual ~RPNFunGTR() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunLSS : public RPNFunction {
public:
        RPNFunLSS() {}
        virtual ~RPNFunLSS() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunGEQ : public RPNFunction {
public:
        RPNFunGEQ() {}
        virtual ~RPNFunGEQ() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunLEQ : public RPNFunction {
public:
        RPNFunLEQ() {}
        virtual ~RPNFunLEQ() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunPrint : public RPNFunction {
public:
        RPNFunPrint() {}
        virtual ~RPNFunPrint() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunScan : public RPNFunction {
public:
        RPNFunScan() {}
        virtual ~RPNFunScan() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCastBool : public RPNFunction {
public:
        RPNFunCastBool() {}
        virtual ~RPNFunCastBool() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCastInt : public RPNFunction {
public:
        RPNFunCastInt() {}
        virtual ~RPNFunCastInt() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCastDouble : public RPNFunction {
public:
        RPNFunCastDouble() {}
        virtual ~RPNFunCastDouble() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCastString : public RPNFunction {
public:
        RPNFunCastString() {}
        virtual ~RPNFunCastString() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunRand : public RPNFunction {
public:
        RPNFunRand() {}
        virtual ~RPNFunRand() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAbs : public RPNFunction {
public:
        RPNFunAbs() {}
        virtual ~RPNFunAbs() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunPow : public RPNFunction {
public:
        RPNFunPow() {}
        virtual ~RPNFunPow() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunSqrt : public RPNFunction {
public:
        RPNFunSqrt() {}
        virtual ~RPNFunSqrt() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunSin : public RPNFunction {
public:
        RPNFunSin() {}
        virtual ~RPNFunSin() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCos : public RPNFunction {
public:
        RPNFunCos() {}
        virtual ~RPNFunCos() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunTan : public RPNFunction {
public:
        RPNFunTan() {}
        virtual ~RPNFunTan() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAsin : public RPNFunction {
public:
        RPNFunAsin() {}
        virtual ~RPNFunAsin() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAcos : public RPNFunction {
public:
        RPNFunAcos() {}
        virtual ~RPNFunAcos() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAtan : public RPNFunction {
public:
        RPNFunAtan() {}
        virtual ~RPNFunAtan() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunAtan2 : public RPNFunction {
public:
        RPNFunAtan2() {}
        virtual ~RPNFunAtan2() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunExp : public RPNFunction {
public:
        RPNFunExp() {}
        virtual ~RPNFunExp() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunLog : public RPNFunction {
public:
        RPNFunLog() {}
        virtual ~RPNFunLog() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunCeil : public RPNFunction {
public:
        RPNFunCeil() {}
        virtual ~RPNFunCeil() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunFloor : public RPNFunction {
public:
        RPNFunFloor() {}
        virtual ~RPNFunFloor() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunTrunc : public RPNFunction {
public:
        RPNFunTrunc() {}
        virtual ~RPNFunTrunc() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunRound : public RPNFunction {
public:
        RPNFunRound() {}
        virtual ~RPNFunRound() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunMax : public RPNFunction {
public:
        RPNFunMax() {}
        virtual ~RPNFunMax() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

class RPNFunMin : public RPNFunction {
public:
        RPNFunMin() {}
        virtual ~RPNFunMin() {}
        virtual RPNElem *Call(RPNItem **stack, LabTable& L, VarTable& V) const;
};

#endif

