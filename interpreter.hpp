#ifndef INTERPRETER_HPP_SENTRY
#define INTERPRETER_HPP_SENTRY

class RPNItem;
class LabTable;

class Interpreter {
public:
        Interpreter() {}
        void RunScript(const char *script);
private:
        RPNItem *BuildProgram(const char *script, LabTable *L);
        void DeleteProgram(RPNItem *prog);
        static void ErrorLine(const char *script, unsigned int line);
};

#endif

