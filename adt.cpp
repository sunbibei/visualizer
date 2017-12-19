#include "adt.h"

const unsigned int INIT_TIMES = 4;

bool __malloc(double*** _ptr, size_t _r, size_t _c) {
    if (nullptr == _ptr) return false;
    *_ptr = new double*[_r];
    for (size_t r = 0; r < _r; ++r) {
        (*_ptr)[r] = new double[_c];
        for (size_t c = 0; c < _c; ++c)
            (*_ptr)[r][c] = 0;
    }
    return true;
}

void __free(double*** _ptr, size_t _r) {
    if (nullptr == _ptr) return;

    for (size_t r = 0; r < _r; ++r)
        delete[] (*_ptr)[r];

    delete[] *_ptr;
    *_ptr = nullptr;
}

ADT::ADT(size_t _r, size_t _c)
    : data_mat_(nullptr),
      ROWS(_r), COLS(_c),
      n_times_(0), N_times_(INIT_TIMES)
{
    data_mat_ = new double**[N_times_];
    __malloc(data_mat_, ROWS, COLS);
}

ADT::~ADT() {
    for (size_t t = 0; t < n_times_; ++t)
        __free(data_mat_, ROWS);
    delete[] data_mat_;
    data_mat_ = nullptr;
}

void ADT::update(size_t _r, size_t _c, double _v) {
    if ((_r < ROWS) && (_c < COLS))
        data_mat_[n_times_][_r][_c] = _v;
}

void ADT::print() {
//    std::cout << "The current values: " << std::endl;
//    printf("-- ");
//    for (int i = 0; i < data_mat_[0].size(); ++i)
//        printf("%02d ", i);

//    for (size_t _r = 0; _r < data_mat_.size(); ++_r) {
//        printf("%02d ", _r);
//        const auto& _rs = data_mat_[_r];
//        for (const auto& _v : _rs)
//            printf("%01.04f ", _v);
//        printf("\n");
//    }
}
