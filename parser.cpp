#include <cstdlib>
#include <cstring>
#include "parser.hpp"
#include "error.hpp"

Parser::Parser()
{
        cur_lex = 0;
        stack = 0;
        last = 0;
        prog = 0;
}

RPNItem *Parser::Analyze(LexItem *tokens, LabTable *L)
{
        cur_lex = tokens;
        tab = L;
        if (cur_lex) {
                S();
                return prog;
        }
        throw SyntaxError("no input tokens", cur_lex);
}

void Parser::Next()
{
        cur_lex = cur_lex->next ? cur_lex->next : cur_lex;
}

void Parser::S()
{
        if (!IsLex("program"))
                throw SyntaxError("expected keyword 'program'", cur_lex);
        Next();
        if (!IsString())
                throw SyntaxError("expected program name", cur_lex);
        Next();
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
        if (!IsLex("begin"))
                throw SyntaxError("expected keyword 'begin'", cur_lex);
        Next();
        A();
        if (!IsLex("end"))
                throw SyntaxError("expected keyword 'end'", cur_lex);
        if (cur_lex->next)
                throw SyntaxError("expected end of file", cur_lex);
}

void Parser::A()
{
        if (IsLex("{")){
                Next();
                while (!IsLex("}"))
                        B();
                Next();
        } else {
                throw SyntaxError("expected '{'", cur_lex);
        }
}

void Parser::B()
{
        if (IsLex("if")) {
                Next();
                B1();
        } else if (IsLex("while")) {
                Next();
                B2();
        } else if (IsLex("repeat")) {
                Next();
                B3();
        } else if (IsLex("goto")) {
                Next();
                B4();
        } else if (IsLex("alloc")) {
                Next();
                B5();
        } else if (IsLex("free")) {
                Next();
                B6();
        } else if (IsLex("print")) {
                Next();
                B7();
        } else if (IsLex("scan")) {
                Next();
                B8();
                Add(new RPNFunScan);
        } else if (IsLex("inc")) {
                Next();
                B8();
                Add(new RPNFunInc);
        } else if (IsLex("dec")) {
                Next();
                B8();
                Add(new RPNFunDec);
        } else if (IsVariable()) {
                Add(new RPNString(cur_lex->token));
                Next();
                D();
                B9();
        } else if (IsLabel()) {
                B10();
        } else {
                throw SyntaxError("expected operator or label", cur_lex);
        }
}

void Parser::B1()
{
        RPNItem *tmp = Blank();
        C1();
        Add(new RPNJumpFalse);
        A();
        if (IsLex("else")) {
                Next();
                RPNItem *tmp2 = Blank();
                Add(new RPNJump);
                Add(new RPNFunNull);
                tmp->elem = new RPNLabel(last);
                A();
                Add(new RPNFunNull);
                tmp2->elem = new RPNLabel(last);
        } else {
                Add(new RPNFunNull);
                tmp->elem = new RPNLabel(last);
        }
}

void Parser::B2()
{
        Add(new RPNFunNull);
        RPNItem *tmp2 = last;
        RPNItem *tmp = Blank();
        C1();
        Add(new RPNJumpFalse);
        A();
        Add(new RPNLabel(tmp2));
        Add(new RPNJump);
        Add(new RPNFunNull);
        tmp->elem = new RPNLabel(last);
}

void Parser::B3()
{
        Add(new RPNFunNull);
        RPNItem *tmp = last;
        A();
        Add(new RPNLabel(tmp));
        if (!IsLex("until"))
                throw SyntaxError("expected keyword 'until'", cur_lex);
        Next();
        C1();
        Add(new RPNJumpFalse);
}

void Parser::B4()
{
        if (!IsLabel())
                throw SyntaxError("expected label", cur_lex);
        Add(new RPNString(cur_lex->token));
        Next();
        Add(new RPNFunLab);
        Add(new RPNJump);
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B5()
{
        if (IsVariable()) {
                Add(new RPNString(cur_lex->token));
                Next();
                C1();
        } else {
                throw SyntaxError("expected variable", cur_lex);
        }
        Add(new RPNFunAlloc);
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B6()
{
        if (IsVariable()) {
                Add(new RPNString(cur_lex->token));
                Next();
        } else {
                throw SyntaxError("expected variable", cur_lex);
        }
        Add(new RPNFunFree);
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B7()
{
again:
        if (IsLex("endl")) {
                Add(new RPNString("\n"));
                Next();
        } else {
                C1();
                Add(new RPNFunCastString);
        }
        if (IsLex(",")) {
                Next();
                goto again;
        }
        Add(new RPNFunPrint);
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B8()
{
        if (!IsVariable())
                throw SyntaxError("expected variable", cur_lex);
        Add(new RPNString(cur_lex->token));
        Next();
        D();
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B9()
{
        if (!IsLex("="))
                throw SyntaxError("expected operator '='", cur_lex);
        Next();
        C1();
        Add(new RPNFunAssign);
        if (!IsLex(";"))
                throw SyntaxError("expected ';'", cur_lex);
        Next();
}

void Parser::B10()
{
        Add(new RPNFunNull);
        bool res = tab->AddLabel(last, cur_lex->token);
        if (!res)
                throw SyntaxError("duplicate label", cur_lex);
        Next();
        if (!IsLex(":"))
                throw SyntaxError("expected ':' after label", cur_lex);
        Next();
}

void Parser::C1()
{
        C2();
        while (IsLex("~") || IsLex("equ")) {
                Push(new RPNFunEQ);
                Next();
                C2();
                Add(Pop());
        }
}

void Parser::C2()
{
        C3();
        while (IsLex("|") || IsLex("^") || IsLex("or") || IsLex("xor")) {
                if (IsLex("|") || IsLex("or"))
                        Push(new RPNFunOR);
                else
                        Push(new RPNFunXOR);
                Next();
                C3();
                Add(Pop());
        }
}

void Parser::C3()
{
        C4();
        while (IsLex("&") || IsLex("and")) {
                Push(new RPNFunAND);
                Next();
                C4();
                Add(Pop());
        }
}

void Parser::C4()
{
        C5();
        if (IsLex("==")||IsLex(">=")||IsLex(">")||
            IsLex("!=")||IsLex("<=")||IsLex("<")){
                if (IsLex("=="))
                        Push(new RPNFunEQU);
                else if (IsLex("!="))
                        Push(new RPNFunNEQ);
                else if (IsLex(">="))
                        Push(new RPNFunGEQ);
                else if (IsLex("<="))
                        Push(new RPNFunLEQ);
                else if (IsLex(">"))
                        Push(new RPNFunGTR);
                else
                        Push(new RPNFunLSS);
                Next();
                C5();
                Add(Pop());
        }
}

void Parser::C5()
{
        C6();
        while (IsLex("+") || IsLex("-")) {
                if (IsLex("+"))
                        Push(new RPNFunPlus);
                else
                        Push(new RPNFunMinus);
                Next();
                C6();
                Add(Pop());
        }
}

void Parser::C6()
{
        C7();
        while (IsLex("*") || IsLex("/") || IsLex("%")) {
                if (IsLex("*"))
                        Push(new RPNFunMul);
                else if (IsLex("/"))
                        Push(new RPNFunDiv);
                else
                        Push(new RPNFunMod);
                Next();
                C7();
                Add(Pop());
        }
}

void Parser::C7()
{
        if (IsLex("-") || IsLex("!") || IsLex("not")) {
                if (IsLex("-"))
                        Push(new RPNFunUMinus);
                else
                        Push(new RPNFunNOT);
                Next();
                C8();
                Add(Pop());
        } else {
                C8();
        }
}

void Parser::C8()
{
        if (IsVariable()) {
                Add(new RPNString(cur_lex->token));
                Next();
                D();
                Add(new RPNFunVar);
        } else if (IsFunction() || IsCast()) {
                Push(NewFunction());
                Next();
                E();
                Add(Pop());
        } else if (IsString()) {
                Add(new RPNString(cur_lex->token));
                Next();
        } else if (IsConstant()) {
                if (strchr(cur_lex->token, '.'))
                        Add(new RPNDouble(atof(cur_lex->token)));
                else
                        Add(new RPNInt(atol(cur_lex->token)));
                Next();
        } else if (IsBool()) {
                Add(new RPNBool(IsLex("true")));
                Next();
        } else if (IsLex("(")) {
                Next();
                C1();
                if (!IsLex(")"))
                        throw SyntaxError("expected closing bracket", cur_lex);
                Next();
        } else {
                throw SyntaxError("expected operand", cur_lex);
        }
}

void Parser::D()
{
        if (IsLex("[")) {
                Next();
                C1();
                if (!IsLex("]"))
                        throw SyntaxError("expected ']'", cur_lex);
                Next();
        } else {
                Add(new RPNInt(0));
        }
        Add(new RPNFunIndex);
}

void Parser::E()
{
        if (!IsLex("("))
                throw SyntaxError("expected '(' before arguments", cur_lex);
        Next();
        if (!IsLex(")")) {
                C1();
                while (IsLex(",")) {
                        Next();
                        C1();
                }
        }
        if (!IsLex(")"))
                throw SyntaxError("expected ')' after arguments", cur_lex);
        Next();
}

void Parser::Add(RPNElem *unit)
{
        RPNItem *tmp = Blank();
        tmp->elem = unit;
}

void Parser::Push(RPNElem *unit)
{
        RPNItem *tmp = new RPNItem;
        tmp->elem = unit;
        tmp->next = stack;
        stack = tmp;
}

RPNElem *Parser::Pop()
{
        RPNElem *unit = stack->elem;
        RPNItem *tmp = stack;
        stack = stack->next;
        delete tmp;
        return unit;
}

RPNItem *Parser::Blank()
{
        if (last) {
                last->next = new RPNItem;
                last = last->next;
        } else {
                prog = new RPNItem;
                last = prog;
        }
        last->next = 0;
        return last;
}

RPNElem *Parser::NewFunction() const
{
        if (IsLex("bool"))
                return new RPNFunCastBool;
        if (IsLex("int"))
                return new RPNFunCastInt;
        if (IsLex("double"))
                return new RPNFunCastDouble;
        if (IsLex("string"))
                return new RPNFunCastString;
        if (IsLex("?rand"))
                return new RPNFunRand;
        if (IsLex("?abs"))
                return new RPNFunAbs;
        if (IsLex("?pow"))
                return new RPNFunPow;
        if (IsLex("?sqrt"))
                return new RPNFunSqrt;
        if (IsLex("?sin"))
                return new RPNFunSin;
        if (IsLex("?cos"))
                return new RPNFunCos;
        if (IsLex("?tan"))
                return new RPNFunTan;
        if (IsLex("?asin"))
                return new RPNFunAsin;
        if (IsLex("?acos"))
                return new RPNFunAcos;
        if (IsLex("?atan"))
                return new RPNFunAtan;
        if (IsLex("?atan2"))
                return new RPNFunAtan2;
        if (IsLex("?exp"))
                return new RPNFunExp;
        if (IsLex("?log"))
                return new RPNFunLog;
        if (IsLex("?ceil"))
                return new RPNFunCeil;
        if (IsLex("?floor"))
                return new RPNFunFloor;
        if (IsLex("?trunc"))
                return new RPNFunTrunc;
        if (IsLex("?round"))
                return new RPNFunRound;
        if (IsLex("?max"))
                return new RPNFunMax;
        if (IsLex("?min"))
                return new RPNFunMin;
        throw SyntaxError("unknown function", cur_lex);
}

bool Parser::IsLex(const char *str) const
{
        return !strcmp(cur_lex->token, str);
}

bool Parser::IsVariable() const
{
        return cur_lex->type == identifier && cur_lex->token[0] == '$';
}

bool Parser::IsFunction() const
{
        return cur_lex->type == identifier && cur_lex->token[0] == '?';
}

bool Parser::IsLabel() const
{
        return cur_lex->type == identifier && cur_lex->token[0] == '@';
}

bool Parser::IsConstant() const
{
        return cur_lex->type == constant;
}

bool Parser::IsString() const
{
        return cur_lex->type == string_literal;
}

bool Parser::IsBool() const
{
        return IsLex("true") || IsLex("false");
}

bool Parser::IsCast() const
{
        return IsLex("bool") || IsLex("int") || IsLex("double") ||
               IsLex("string");
}

