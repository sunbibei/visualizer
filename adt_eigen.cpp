#include "adt_eigen.h"
#include <iostream>
#include <QDateTime>

#include <opencv/cv.hpp>
#include <opencv2/core/eigen.hpp>

const unsigned int __INIT_TIMES = 16;

inline void __set_value(cv::Mat& _mat, const cv::Range& _rr, const cv::Range& _cr, uchar _val) {
    cv::Mat part = _mat(_rr, _cr);
    for (int r = 0; r < part.rows; ++r) {
        uchar* pR = part.ptr(r);
        for (int c = 0; c < part.cols; ++c) {
            pR[c] = _val;
        }
    }
}

AdtEigen::AdtEigen(size_t r, size_t c, double min, double max)
    : ROWS(r), COLS(c), p_xmlfd_(nullptr),
      max_val_(max), min_val_(min)
{
    data_.resize(ROWS, COLS);
    center_tmp_vecs_.resize(ROWS*COLS);

    s_times_ = 0;
}

AdtEigen::~AdtEigen() {
    if (p_xmlfd_) {
        delete p_xmlfd_;
        p_xmlfd_ = nullptr;
    }

    // if (center_ofd_.is_open()) center_ofd_.close();
}

void AdtEigen::clear() {
    data_.fill(0.0);

    s_times_ = 0;
}

bool AdtEigen::whole_calc(cv::Mat& img, double& _r, double& _c, double max, double thres, size_t a) {
    const auto& _s = data_;
    // img.resize(a*ROWS, b*4*COLS);
    if (max <= 0) max = max_val_;

    double scale = (max == min_val_) ? 0 : 255.0/(max - min_val_);
    // double scale = 0.111;
    // img = cv::Mat::zeros(b*4*COLS, a*ROWS, CV_8UC1);
    // return img; 16 * 88
    for (int r = 0; r < ROWS; r += 2) {
        for (int i = 0; i < 2; ++i) {
            auto data_r = _s.row(r+i);
            for (int c = 0; c < COLS; ++c) {
                __set_value(img, cv::Range(a*r, 2*a+r*a),
                            cv::Range(2*a*(i*COLS+c), 2*a*(i*COLS+c) + 2*a), data_r(c)*scale);
            }
        }
    }

    // plot coordinate system
    cv::line(img, cv::Point(a, a), cv::Point(img.cols - a, a), CV_RGB(255, 255, 255));
    cv::line(img, cv::Point(img.cols-3*a, 0),   cv::Point(img.cols - a, a), CV_RGB(255, 255, 255));
    cv::line(img, cv::Point(img.cols-3*a, 2*a), cv::Point(img.cols - a, a), CV_RGB(255, 255, 255));

    cv::line(img, cv::Point(a, a), cv::Point(a, img.rows - a), CV_RGB(255, 255, 255));
    cv::line(img, cv::Point(0, img.rows-3*a), cv::Point(a, img.rows - a), CV_RGB(255, 255, 255));
    cv::line(img, cv::Point(2*a, img.rows-3*a), cv::Point(a, img.rows - a), CV_RGB(255, 255, 255));

    if (!getCenter(_r, _c, thres)) return false;
    size_t _x = a*2*_r+a;
    size_t _y = a*2*_c+a;
    //绘制横线
    cv::line(img, cv::Point(_y, a), cv::Point(_y, _x), CV_RGB(255, 255, 255));
    //绘制竖线
    cv::line(img, cv::Point(a, _x), cv::Point(_y, _x), CV_RGB(255, 255, 255));
    // cv::putText(img, std::to_string(_r), cv::Point(_y, a+20), 0.5, 0.5, cv::Scalar(255, 255, 255), 1);
    // cv::putText(img, std::to_string(_c), cv::Point(a+10, _x-5), 0.5, 0.5, cv::Scalar(255, 255, 255), 1);
    // cv::resize(img, img, cv::Size(r*b*COLS, r*a*ROWS));
    return true;
}

bool AdtEigen::getCenter(double& _x, double& _y, double thres) {
    const auto& curr = data_;

//    Eigen::Matrix3d guass;
//    guass << 0.0947416, 0.118318, 0.0947416,
//             0.118318,  0.147761, 0.118318,
//             0.0947416, 0.118318, 0.0947416;
//    Eigen::MatrixXd blur(ROWS, COLS);
//    for (size_t _r = 0; _r < ROWS; ++_r) {
//        const auto& rows = curr.row(_r);
//        for (size_t _c = 0; _c < COLS; ++_c) {
//            if ((0 == _r) || (ROWS - 1 == _r)
//                    || (0 == _c) || (COLS - 1 == _c)) {
//                blur(_r, _c) = rows(_c);
//                continue;
//            }
//            blur(_r, _c) = curr.block(_r-1, _c-1, 3, 3).cwiseProduct(guass).sum();
//        }
//    }

    double mean = curr.mean();
    double sqMean = curr.array().square().mean();
    double var  = sqMean - mean*mean;
    // std::cout << "mean: " << mean << std::endl;
    // std::cout << "var:  " << var  << std::endl;
    if (thres < 0) thres = 5;
    double thr  = mean + thres*sqrt(var);
    // std::cout << thr << std::endl;
    _x = 0;
    _y = 0;
    r_tmp_vec.clear();
    c_tmp_vec.clear();
    // std::cout << "ready into loop, thr=" << thr << std::endl;
    for (size_t _r = 0; _r < ROWS; ++_r) {
        const auto& rows = curr.row(_r);
        for (size_t _c = 0; _c < COLS; ++_c) {
            // std::cout << rows(_c) << " ";
            if (rows(_c) > thr) {
                r_tmp_vec.push_back(_r/2);
                _x += _r/2;
                if (1 == _r%2) {
                    c_tmp_vec.push_back(COLS+_c);
                    _y += COLS+_c;
                } else {
                    c_tmp_vec.push_back(_c);
                    _y += _c;
                }
            }
        }
        // std::cout << std::endl;
    }
    // std::cout << "out the loop, " << r_corrs.size() << ", " << c_corrs.size() << std::endl;
    if ((r_tmp_vec.empty()) || (c_tmp_vec.empty())) return false;
    _x /= r_tmp_vec.size();
    _y /= c_tmp_vec.size();
    // std::cout << "center: (" << _x << ", " << _y << ")" << std::endl;
    return true;
    // std::cout << "center: (" << mean_r << ", " << mean_c << ")" << std::endl;


//    Eigen::HouseholderQR<Eigen::MatrixXd> qr;
//    qr.compute(data_[n_times_]);
//    Eigen::MatrixXd R = qr.matrixQR().triangularView<Eigen::Upper>();
//    Eigen::MatrixXd Q = qr.householderQ();

//    //块操作，获取向量或矩阵的局部
//    Eigen::VectorXd S;
//    S = (Q.transpose()* center_tmp_vecs_).head(5);
//    Eigen::MatrixXd R1;
//    R1 = R.block(0,0,5,5);

//    Eigen::VectorXd C;
//    C = R1.inverse() * S;

//    _x = -0.5 * C[1] / C[3];
//    _y = -0.5 * C[2] / C[4];
}

void AdtEigen::update(size_t _t, size_t _r, size_t _c, double _v) {
    data_(_r%ROWS, _c%COLS) = _v;
}

cv::Mat AdtEigen::cvtCvMat(size_t a, size_t b, size_t r) {
    const auto& _s = data_;
    // qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    // auto _s = Eigen::MatrixXd(ROWS, COLS);
//    for (int r = 0; r < ROWS; ++r) {
//        auto data_r = _s.row(r);
//        for (int c = 0; c < COLS; ++c) {
//            data_r(c) = qrand()%255;
//        }
//    }
    // std::cout << _s << std::endl;
    double scale = (max_val_ == min_val_) ? 0 : 255.0/(max_val_ - min_val_);
    // double scale = 0.111;
    cv::Mat img = cv::Mat::zeros(a*ROWS, b*4*COLS, CV_8UC1);
    // return img;
    for (int r = 0; r < ROWS; r += 2) {
        for (int i = 0; i < 2; ++i) {
            auto data_r = _s.row(r+i);
            for (int c = 0; c < COLS; ++c) {
                __set_value(img, cv::Range(a*r, 2*a+r*a),
                            cv::Range(2*b*(i*COLS+c), 2*b*(i*COLS+c) + 2*b), data_r(c)*scale);
            }
        }
    }
    // __set_value(img, cv::Range(40, 90), cv::Range(80,90), 0xff);
    return img;
}


cv::Mat AdtEigen::cvtCvMat(size_t _t, size_t a, size_t b, size_t r) {
    const auto& _s = data_;
    cv::Mat img = cv::Mat::zeros(b*COLS, a*ROWS, CV_8UC1);

    double scale = 1/(max_val_ - min_val_);

    for (size_t r = 0; r < ROWS; ++r) {
        for (size_t c = 0; c < COLS; ++c) {
           for (size_t ra = a*r; ra < a*r+a; ++ra) {
               for (size_t cb = b*c; cb < b*c+b; ++cb) {
                   img.at<unsigned char>(cb, ra) = _s(r, c)*scale*255;
               }
           }
        }
    }

    // cv::resize(img, img, cv::Size(r*b*COLS, r*a*ROWS));
    return img;
}

void AdtEigen::print() {
    const auto& _m = data_;
    std::cout << _m << std::endl;
}

bool AdtEigen::load(size_t _t, Eigen::MatrixXd& _out) {
    auto _fd = new TiXmlDocument;
    if (!_fd->LoadFile(curr_file_name_)) {
        return false;
    }

    auto ele = _fd->RootElement()->FirstChildElement();
    for (; nullptr != ele; ele = ele->NextSiblingElement()) {
        int times = 0;
        if (ele->Attribute("times", &times) && _t == times) {
            int rows = 0;
            int cols = 0;
            if (!ele->Attribute("rows", &rows) || !ele->Attribute("cols", &cols))
                return false;
            _out.resize(rows, cols);
            _out.fill(0.0);
            break;
        }
    }
    if (nullptr == ele) return false;

    const int BUF_SIZE = 32;
    char buf[BUF_SIZE] = {0};
    for (int r = 0; r < _out.rows(); ++r) {
        sprintf(buf, "R%02d", r);
        auto pr = ele->FirstChildElement(buf);
        if (nullptr == pr) continue;
        std::stringstream ss;
        ss << pr->GetText();
        int c = 0;
        while ((c < _out.cols()) && (ss >> _out(r, c))) ++c;
    }

    delete _fd;
    return true;
}

void AdtEigen::saveOnceCSV() {
    std::ofstream tmp_ofd;
    QDateTime curr_date =QDateTime::currentDateTime();
    only_file_name_ = "once_" + curr_date.toString("yyyy_MM_dd_hh_mm_ss").toStdString() + ".csv";
    curr_file_name_ = out_path_ + "/" + only_file_name_;
    tmp_ofd.open(curr_file_name_);


    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    tmp_ofd << "timestamp, " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()
            << ",times, " << s_times_
              << ",rows, " << ROWS << ",cols, " << COLS << std::endl;

    const auto& _m = data_;
    for (int r = 0; r < _m.rows(); ++r) {
        for (int c = 0; c < _m.cols(); ++c)
            sprintf(tmp + c*VALUE_SIZE, "%01.05f,", _m(r, c));
        tmp_ofd.write(tmp, _m.cols()*VALUE_SIZE);
        tmp_ofd << std::endl;
    }

    delete[] tmp;
    tmp_ofd.close();
}

void AdtEigen::saveOnce() {
    auto tmp_xmlfd_ = new TiXmlDocument;
    QDateTime curr_date = QDateTime::currentDateTime();
    only_file_name_ = "once_" + curr_date.toString("yyyy_MM_dd_hh_mm_ss").toStdString() + ".xml";
    curr_file_name_ = out_path_ + "/" + only_file_name_;
    if (!tmp_xmlfd_->LoadFile(curr_file_name_)) {
        tmp_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
        tmp_xmlfd_->LinkEndChild(new TiXmlElement("history"));
    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    TiXmlElement* _new = new TiXmlElement("record");
    _new->SetAttribute("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
    _new->SetAttribute("times",     s_times_);
    _new->SetAttribute("rows",      ROWS);
    _new->SetAttribute("cols",      COLS);

    const auto& _m = data_;
    for (int r = 0; r < _m.rows(); ++r) {
        sprintf(tmp, "R%02d", r);
        TiXmlElement* _row = new TiXmlElement(tmp);
        for (int c = 0; c < _m.cols(); ++c)
            sprintf(tmp + c*VALUE_SIZE, "%01.05f ", _m(r, c));
        _row->LinkEndChild(new TiXmlText(tmp));
        _new->LinkEndChild(_row);
    }
    tmp_xmlfd_->RootElement()->LinkEndChild(_new);
    tmp_xmlfd_->SaveFile(curr_file_name_);

    delete[] tmp;
    delete tmp_xmlfd_;

    clear();
}

bool AdtEigen::loadCSV(const std::string& file) {
    std::ifstream ifd(file);
    if (!ifd.is_open()) return false;

    double tmp = 0;
    auto& _m = data_;
    for (int r = 0; r < _m.rows(); ++r) {
        for (int c = 0; c < _m.cols(); ++c) {
            ifd >> tmp;
            _m(r, c) = tmp;
        }
    }

//    Eigen::Matrix3d guass;
//    guass << 0.0947416, 0.118318, 0.0947416,
//             0.118318,  0.147761, 0.118318,
//             0.0947416, 0.118318, 0.0947416;
//    // Eigen::MatrixXd blur(ROWS, COLS);
//    for (size_t _r = 0; _r < ROWS; ++_r) {
//        const auto& rows = _m.row(_r);
//        for (size_t _c = 0; _c < COLS; ++_c) {
//            if ((0 == _r) || (ROWS - 1 == _r)
//                    || (0 == _c) || (COLS - 1 == _c)) {
//                _m(_r, _c) = rows(_c);
//                continue;
//            }
//            _m(_r, _c) = _m.block(_r-1, _c-1, 3, 3).cwiseProduct(guass).sum();
//        }
//    }
}

void AdtEigen::saveCSV(bool save_center) {
    QTime curr = QTime::currentTime();
    if (last_hour_ < curr.hour()) {
        if (data_csv_.is_open()) data_csv_.close();
        last_hour_= curr.hour();
    }

    if (!data_csv_.is_open()) {
        QDateTime curr_date =QDateTime::currentDateTime();
        only_file_name_ = "data_" + curr_date.toString("yyyy_MM_dd_hh_mm").toStdString() + ".csv";
        curr_file_name_ = out_path_ + "/" + only_file_name_;
        data_csv_.open(curr_file_name_);
    }
//    if (!center_ofd_.is_open()) {
//        center_ofd_.open(out_path_ + "/center.csv");
//    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
//    if (save_center) {
//        size_t x, y;
//        getCenter(x, y);
//        center_ofd_ << time << "," << x << "," << y << std::endl;
//    }
    data_csv_ << "timestamp, " << time << ",times, " << s_times_++
              << ",rows, " << ROWS << ",cols, " << COLS << std::endl;

    const auto& _m = data_;
    for (int r = 0; r < _m.rows(); ++r) {
        for (int c = 0; c < _m.cols(); ++c)
            sprintf(tmp + c*VALUE_SIZE, "%01.05f,", _m(r, c));
        data_csv_.write(tmp, _m.cols()*VALUE_SIZE);
        data_csv_ << std::endl;
    }

    delete[] tmp;
}

void AdtEigen::save(bool save_center) {
    QTime curr = QTime::currentTime();
    if (last_hour_ < curr.hour()) {
        if (p_xmlfd_) delete p_xmlfd_;
        p_xmlfd_ = nullptr;
        last_hour_= curr.hour();
    }

    if (nullptr == p_xmlfd_) {
        p_xmlfd_ = new TiXmlDocument;
        QDateTime curr_date =QDateTime::currentDateTime();
        only_file_name_ = "data_" + curr_date.toString("yyyy_MM_dd_hh_mm").toStdString() + ".xml";
        curr_file_name_ = out_path_ + "/" + only_file_name_;
        if (!p_xmlfd_->LoadFile(curr_file_name_)) {
            p_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
            p_xmlfd_->LinkEndChild(new TiXmlElement("history"));
        }
    }
//    if (!center_ofd_.is_open()) {
//        center_ofd_.open(out_path_ + "/center.csv");
//    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    TiXmlElement* _new = new TiXmlElement("record");
    std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
//    if (save_center) {
//        size_t x, y;
//        getCenter(x, y);
//        center_ofd_ << time << "," << x << "," << y << std::endl;
//    }

    _new->SetAttribute("timestamp", time);
    _new->SetAttribute("times",     s_times_++);
    _new->SetAttribute("rows",      ROWS);
    _new->SetAttribute("cols",      COLS);

    const auto& _m = data_;
    for (int r = 0; r < _m.rows(); ++r) {
        sprintf(tmp, "R%02d", r);
        TiXmlElement* _row = new TiXmlElement(tmp);
        for (int c = 0; c < _m.cols(); ++c)
            sprintf(tmp + c*VALUE_SIZE, "%01.05f ", _m(r, c));
        _row->LinkEndChild(new TiXmlText(tmp));
        _new->LinkEndChild(_row);
    }
    p_xmlfd_->RootElement()->LinkEndChild(_new);
    p_xmlfd_->SaveFile(curr_file_name_);

    delete[] tmp;
}

void AdtEigen::saveCenter() {
//    if (center_ofd_.is_open()) {
//        size_t x, y;
//        getCenter(x, y);

//        std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
//        center_ofd_ << time << "," << x << "," << y << std::endl;
//    }
}

//void AdtEigen::saveAll() {
//    QTime curr = QTime::currentTime();
//    if (last_hour_ < curr.hour()) {
//        if (p_xmlfd_) delete p_xmlfd_;
//        p_xmlfd_ = nullptr;
//        last_hour_= curr.hour();
//    }

//    if (nullptr == p_xmlfd_) {
//        p_xmlfd_ = new TiXmlDocument;
//        QDateTime curr_date =QDateTime::currentDateTime();
//        curr_file_name_ = out_path_ + curr_date.toString("_yyyy_MM_dd_hh_mm").toStdString();
//        if (!p_xmlfd_->LoadFile(curr_file_name_)) {
//            p_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
//            p_xmlfd_->LinkEndChild(new TiXmlElement("history"));
//        }
//    }

//    const size_t BUF_SIZE   = 1024;
//    const size_t VALUE_SIZE = 8;
//    char* tmp = new char[BUF_SIZE];
//    memset(tmp, 0x00, BUF_SIZE*sizeof(char));
//    for (size_t i = 0; i < N_times_; ++i) {
//        TiXmlElement* _new = new TiXmlElement("record");
//        _new->SetAttribute("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
//        _new->SetAttribute("times",     s_times_*N_times_ + i);
//        _new->SetAttribute("rows",      ROWS);
//        _new->SetAttribute("cols",      COLS);

//        const auto& _m = data_[i];
//        for (int r = 0; r < _m.rows(); ++r) {
//            sprintf(tmp, "R%02d", r);
//            TiXmlElement* _row = new TiXmlElement(tmp);
//            for (int c = 0; c < _m.cols(); ++c)
//                sprintf(tmp + c*VALUE_SIZE, "%01.05f ", _m(r, c));
//            _row->LinkEndChild(new TiXmlText(tmp));
//            _new->LinkEndChild(_row);
//        }
//        p_xmlfd_->RootElement()->LinkEndChild(_new);
//    }

//    p_xmlfd_->SaveFile(curr_file_name_);
//    delete[] tmp;
//}

