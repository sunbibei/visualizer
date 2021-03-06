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

    void print();
    void saveCenter();
    void saveCSV(bool save_center = true);
    void save(bool save_center = true);
    void saveOnceCSV();
    void saveOnce();
    // void saveAll();
    bool loadCSV(const std::string& file);
    bool load(size_t _t, Eigen::MatrixXd&);

    const std::string& getCurrentFileName() {return only_file_name_;}

    bool getCenter(double &_x, double &_y, double thres = 5);
    bool whole_calc(cv::Mat&, double&, double&, double max = -1, double thres = 5, size_t a = 6);
    // for Debug
    Eigen::MatrixXd& data_ref() {return data_;}

// private:
    void update(size_t _t, size_t _r, size_t _c, double _v);
    cv::Mat cvtCvMat(size_t a = 3, size_t b = 3, size_t r = 3);
    cv::Mat cvtCvMat(size_t _t, size_t a = 3, size_t b = 3, size_t r = 3);

public:
    const size_t    ROWS;
    const size_t    COLS;

private:
    Eigen::MatrixXd data_;
    int             s_times_;

    Eigen::VectorXd center_tmp_vecs_;

    std::vector<size_t> r_tmp_vec;
    std::vector<size_t> c_tmp_vec;

    double          min_val_;
    double          max_val_;

    std::ofstream   center_ofd_;
    TiXmlDocument*  p_xmlfd_;
    std::ofstream   data_csv_;
    std::string     only_file_name_;
    std::string     curr_file_name_;
    std::string     out_path_;
    size_t          last_times_;

    int             last_hour_;
};

#endif // ADT_EIGEN_H
