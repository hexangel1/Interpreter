#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "interpreter.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include "vartable.hpp"
#include "labtable.hpp"
#include "engine.hpp"
#include "error.hpp"

void Interpreter::WriteLine(const char *script, unsigned int line)
{
        FILE *fp = fopen(script, "r");
        if (!fp) {
                perror(script);
                return;
        }
        unsigned int current = 1;
        while (current < line) {
                int c = fgetc(fp);
                if (c == EOF)
                        break;
                if (c == '\n')
                        current++;
        }
        char buff[128];
        fgets(buff, 128, fp);
        fputs(buff, stderr);
        fclose(fp);
}

RPNItem *Interpreter::BuildProgram(const char *script, LabTable *L)
{
        Scanner B;
        Parser C;
        FILE *fp = fopen(script, "r");
        if (!fp) {
                perror(script);
                return 0;
        }
        int c;
        while ((c = fgetc(fp)) != EOF)
                B.Feed(c);
        fclose(fp);
        if (!B.IsSuccess()) {
                B.PrintError();
                WriteLine(script, B.ErrLine());
                return 0;
        }
        LexItem *token = B.GetTokenList();
        RPNItem *prog = 0;
        try {
                prog = C.Analyze(token, L);
        }
        catch (const SyntaxError& err) {
                err.Report();
                WriteLine(script, err.Get()->line);
                fputs("Exception: parsing error\n", stderr);
        }
        return prog;
}

void Interpreter::DeleteProgram(RPNItem *prog)
{
        RPNItem *tmp;
        while (prog) {
                tmp = prog;
                prog = prog->next;
                delete tmp->elem;
                delete tmp;
        }
}

void Interpreter::RunScript(const char *script)
{
        VarTable V;
        LabTable L;
        RPNItem *prog, *cur_cmd, *stack = 0;
        prog = BuildProgram(script, &L);
        if (!prog)
                return;
        srand(time(0));
        cur_cmd = prog;
        try {
                while (cur_cmd)
                        cur_cmd->elem->Evaluate(&cur_cmd, &stack, L, V);
        }
        catch (const RuntimeError& err) {
                err.Report();
                fputs("Exception: runtime error\n", stderr);
        }
        DeleteProgram(stack);
        DeleteProgram(prog);
}

