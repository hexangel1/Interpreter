
template <class T>
class Buffer {
        size_t buf_size;
        T *buffer;
public:
        Buffer() : buf_size(4) { buffer = new T[buf_size]; }
        ~Buffer() { delete []buffer; }
        T& operator[](size_t idx) {
                if (idx >= buf_size) {
                        size_t old_size = buf_size;
                        while (idx >= buf_size)
                                buf_size <<= 1;
                        T *tmp = new T[buf_size];
                        for (size_t i = 0; i < old_size; i++)
                                tmp[i] = buffer[i];
                        delete []buffer;
                        buffer = tmp;
                }
                return buffer[idx];
        }
};

