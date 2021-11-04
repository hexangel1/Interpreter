#ifndef ARRAY_HPP_SENTRY
#define ARRAY_HPP_SENTRY

class Variable {
        friend class Array;
        enum {
                bool_type,
                int_type,
                double_type,
                string_type
        } type;
        union {
                bool boolean;
                long integer;
                double real;
                char *string;
        } value;
public:
        Variable();
        Variable(const Variable& var);
        ~Variable();
        Variable& operator=(const Variable& var);
        void Set(class RPNConst *val);
        class RPNConst *Get() const;
};

class Array {
        Variable *var;
        unsigned long allocated;
public:
        Array(unsigned long size);
        Array(const Array& arr);
        ~Array();
        void Allocate(unsigned long size);
        Variable& operator[](unsigned long index);
};

#endif

