#ifndef PARSER_HPP_SENTRY
#define PARSER_HPP_SENTRY

#include "engine.hpp"
#include "scanner.hpp"
#include "labtable.hpp"

class Parser {
        LexItem *cur_lex;
        RPNItem *stack;
        RPNItem *last;
        RPNItem *prog;
        LabTable *tab;
public:
        Parser();
        RPNItem *Analyze(LexItem *tokens, LabTable *L);
private:
        void Next();
        void S();
        void A();
        void B();
        void B1();
        void B2();
        void B3();
        void B4();
        void B5();
        void B6();
        void B7();
        void B8();
        void B9();
        void B10();
        void C1();
        void C2();
        void C3();
        void C4();
        void C5();
        void C6();
        void C7();
        void C8();
        void D();
        void E();
        void Add(RPNElem *unit);
        void Push(RPNElem *unit);
        RPNElem *Pop();
        RPNItem *Blank();
        RPNElem *NewFunction() const;
        bool IsLex(const char *str) const;
        bool IsVariable() const;
        bool IsFunction() const;
        bool IsLabel() const;
        bool IsConstant() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsCast() const;
};

#endif

