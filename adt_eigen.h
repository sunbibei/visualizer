#ifndef ADT_EIGEN_H
#define ADT_EIGEN_H

#include <opencv/cv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <tinyxml.h>

//#ifndef size_t
//#define size_t unsigned int
//#endif

class AdtEigen
{
public:
    AdtEigen(size_t r, size_t c);

    static void setPath(const std::string& _p) { s_path_ = _p; }

    void print(size_t t);
    void save(size_t _t);
    bool load(size_t _t, Eigen::MatrixXd&);

    // for Debug
    Eigen::MatrixXd& data_ref(size_t t) {return data_[t];}

// private:
    void update(size_t _t, size_t _r, size_t _c, double _v);
    cv::Mat cvtCvMat(size_t _t, size_t a = 3, size_t b = 3, size_t r = 3);

public:
    const size_t    ROWS;
    const size_t    COLS;

private:
    std::vector<Eigen::MatrixXd> data_;
    size_t          n_times_;
    size_t          N_times_;

    double          min_val_;
    double          max_val_;

    TiXmlDocument*  p_xmlfd_;

    static std::string s_path_;
};

#endif // ADT_EIGEN_H
