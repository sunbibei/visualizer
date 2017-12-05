#ifndef ADT_H
#define ADT_H

#ifndef size_t
#define size_t unsigned int
#endif

class ADT
{
public:
    ADT(size_t _r, size_t _c);
    virtual ~ADT();

public:
    void update(size_t _r, size_t _c, double _v);

    void print();
private:
    double***       data_mat_;
    const size_t    ROWS;
    const size_t    COLS;
    size_t          n_times_;
    size_t          N_times_;
};

#endif // ADT_H
