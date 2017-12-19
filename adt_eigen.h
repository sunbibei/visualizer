#ifndef ADT_EIGEN_H
#define ADT_EIGEN_H

#include <opencv/cv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <fstream>
#include "tinyxml.h"

//#ifndef size_t
//#define size_t unsigned int
//#endif

class AdtEigen
{
public:
    AdtEigen(size_t r, size_t c, double min = 0, double max = 4.0);
    ~AdtEigen();

    void setFile(const std::string& _p) { out_path_ = _p; }

    void clear();

    void print(size_t t);
    void saveCenter();
    void saveCSV(bool save_center = true);
    void save(bool save_center = true);
    void saveOnceCSV();
    void saveOnce();
    void saveAll();
    bool load(size_t _t, Eigen::MatrixXd&);

    bool getCenter(size_t& _x, size_t& _y);
    bool whole_calc(cv::Mat&, size_t&, size_t&, size_t a = 3, size_t b = 3, size_t r = 3);
    // for Debug
    Eigen::MatrixXd& data_ref(size_t t) {return data_[t];}

// private:
    void update(size_t _t, size_t _r, size_t _c, double _v);
    cv::Mat cvtCvMat(size_t a = 3, size_t b = 3, size_t r = 3);
    cv::Mat cvtCvMat(size_t _t, size_t a = 3, size_t b = 3, size_t r = 3);

public:
    const size_t    ROWS;
    const size_t    COLS;

private:
    std::vector<Eigen::MatrixXd> data_;
    Eigen::VectorXd center_tmp_vecs_;

    std::vector<size_t> r_tmp_vec;
    std::vector<size_t> c_tmp_vec;

    size_t          s_times_;
    size_t          n_times_;
    size_t          N_times_;

    double          min_val_;
    double          max_val_;

    std::ofstream   center_ofd_;
    TiXmlDocument*  p_xmlfd_;
    std::ofstream   data_csv_;
    std::string     curr_file_name_;
    std::string     out_path_;
    size_t          last_times_;

    int             last_hour_;
};

#endif // ADT_EIGEN_H
