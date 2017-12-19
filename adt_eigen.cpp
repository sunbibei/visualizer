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
    : ROWS(r), COLS(c), s_times_(0),
      N_times_(__INIT_TIMES), p_xmlfd_(nullptr),
      max_val_(max), min_val_(min)
{
    data_.resize(N_times_);
    for (auto& mat : data_) {
        mat.resize(ROWS, COLS);
        mat.fill(0.0);
    }
    center_tmp_vecs_.resize(ROWS*COLS);

    n_times_    = 0;
    last_times_ = N_times_;
}

AdtEigen::~AdtEigen() {
    if (p_xmlfd_) {
        delete p_xmlfd_;
        p_xmlfd_ = nullptr;
    }

    if (center_ofd_.is_open()) center_ofd_.close();
}

void AdtEigen::clear() {
    for (auto& mat : data_)
        mat.fill(0.0);

    n_times_ = 0;
    last_times_ = N_times_;
}

bool AdtEigen::whole_calc(cv::Mat& img, size_t& _x, size_t& _y, size_t a, size_t b, size_t r) {
    const auto& _s = data_[n_times_];
    // img.resize(a*ROWS, b*4*COLS);
    double scale = (max_val_ == min_val_) ? 0 : 255.0/(max_val_ - min_val_);
    // double scale = 0.111;
    // img = cv::Mat::zeros(a*ROWS, b*4*COLS, CV_8UC1);
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


    if (!getCenter(_x, _y)) return false;
    // _x = 30;
    // _y = 16;
    //绘制横线
    cv::line(img, cv::Point(_x - 8, _y), cv::Point(_x + 8, _y), CV_RGB(255, 255, 255));
    //绘制竖线
    cv::line(img, cv::Point(_x, _y - 10), cv::Point(_x, _y + 10), CV_RGB(255, 255, 255));
    // cv::resize(img, img, cv::Size(r*b*COLS, r*a*ROWS));
    return true;
}

bool AdtEigen::getCenter(size_t& _x, size_t& _y) {
    const auto& curr = data_[n_times_];

    double mean = curr.mean();
    double sqMean = curr.array().square().mean();
    double var  = sqMean - mean*mean;
    // std::cout << "mean: " << mean << std::endl;
    // std::cout << "var:  " << var  << std::endl;

    double thr  = mean + 5*sqrt(var);

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
                r_tmp_vec.push_back(_r);
                c_tmp_vec.push_back(_c);
                _x += _r;
                _y += _c;
            }
        }
        // std::cout << std::endl;
    }
    // std::cout << "out the loop, " << r_corrs.size() << ", " << c_corrs.size() << std::endl;
    if ((r_tmp_vec.empty()) || (c_tmp_vec.empty())) return false;
    _x /= r_tmp_vec.size();
    _y /= c_tmp_vec.size();
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
    if (N_times_ == last_times_) last_times_ = _t;
    // if (_t >= N_times_) return;
    if (last_times_ != _t) {
        ++n_times_;
        if (n_times_ == N_times_) {
            // saveAll();
            ++s_times_;
            n_times_ = 0;
        }

        last_times_ = _t;
        data_[n_times_].fill(0.0);
    }

    auto& curr = data_[n_times_];
    curr(_r%ROWS, _c%COLS) = _v;
    if (0 == _v) _v = 0.00001;
    center_tmp_vecs_(_r*COLS + _c) = _v * log(_v);
}

cv::Mat AdtEigen::cvtCvMat(size_t a, size_t b, size_t r) {
    const auto& _s = data_[n_times_];
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
    const auto& _s = data_[_t%N_times_];
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

void AdtEigen::print(size_t t) {
    const auto& _m = data_[t%N_times_];
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
    curr_file_name_ = out_path_ + "/once_" + curr_date.toString("yyyy_MM_dd_hh_mm_ss").toStdString() + ".csv";
    tmp_ofd.open(curr_file_name_);


    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    tmp_ofd << "timestamp, " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()
            << ",times, " << s_times_*N_times_ + n_times_
              << ",rows, " << ROWS << ",cols, " << COLS << std::endl;

    const auto& _m = data_[n_times_];
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
    curr_file_name_ = out_path_ + "/once_" + curr_date.toString("yyyy_MM_dd_hh_mm_ss").toStdString() + ".xml";
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
    _new->SetAttribute("times",     s_times_*N_times_ + n_times_);
    _new->SetAttribute("rows",      ROWS);
    _new->SetAttribute("cols",      COLS);

    const auto& _m = data_[n_times_];
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

void AdtEigen::saveCSV(bool save_center) {
    QTime curr = QTime::currentTime();
    if (last_hour_ < curr.hour()) {
        if (data_csv_.is_open()) data_csv_.close();
        last_hour_= curr.hour();
    }

    if (!data_csv_.is_open()) {
        QDateTime curr_date =QDateTime::currentDateTime();
        curr_file_name_ = out_path_ + "/data_" + curr_date.toString("yyyy_MM_dd_hh_mm").toStdString() + ".csv";
        data_csv_.open(curr_file_name_);
    }
    if (!center_ofd_.is_open()) {
        center_ofd_.open(out_path_ + "/center.csv");
    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    if (save_center) {
        size_t x, y;
        getCenter(x, y);
        center_ofd_ << time << "," << x << "," << y << std::endl;
    }
    data_csv_ << "timestamp, " << time << ",times, " << s_times_*N_times_ + n_times_
              << ",rows, " << ROWS << ",cols, " << COLS << std::endl;

    const auto& _m = data_[n_times_];
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
        curr_file_name_ = out_path_ + "/data_" + curr_date.toString("yyyy_MM_dd_hh_mm").toStdString() + ".xml";
        if (!p_xmlfd_->LoadFile(curr_file_name_)) {
            p_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
            p_xmlfd_->LinkEndChild(new TiXmlElement("history"));
        }
    }
    if (!center_ofd_.is_open()) {
        center_ofd_.open(out_path_ + "/center.csv");
    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));

    TiXmlElement* _new = new TiXmlElement("record");
    std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    if (save_center) {
        size_t x, y;
        getCenter(x, y);
        center_ofd_ << time << "," << x << "," << y << std::endl;
    }

    _new->SetAttribute("timestamp", time);
    _new->SetAttribute("times",     s_times_*N_times_ + n_times_);
    _new->SetAttribute("rows",      ROWS);
    _new->SetAttribute("cols",      COLS);

    const auto& _m = data_[n_times_];
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
    if (center_ofd_.is_open()) {
        size_t x, y;
        getCenter(x, y);

        std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
        center_ofd_ << time << "," << x << "," << y << std::endl;
    }
}

void AdtEigen::saveAll() {
    QTime curr = QTime::currentTime();
    if (last_hour_ < curr.hour()) {
        if (p_xmlfd_) delete p_xmlfd_;
        p_xmlfd_ = nullptr;
        last_hour_= curr.hour();
    }

    if (nullptr == p_xmlfd_) {
        p_xmlfd_ = new TiXmlDocument;
        QDateTime curr_date =QDateTime::currentDateTime();
        curr_file_name_ = out_path_ + curr_date.toString("_yyyy_MM_dd_hh_mm").toStdString();
        if (!p_xmlfd_->LoadFile(curr_file_name_)) {
            p_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
            p_xmlfd_->LinkEndChild(new TiXmlElement("history"));
        }
    }

    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));
    for (size_t i = 0; i < N_times_; ++i) {
        TiXmlElement* _new = new TiXmlElement("record");
        _new->SetAttribute("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
        _new->SetAttribute("times",     s_times_*N_times_ + i);
        _new->SetAttribute("rows",      ROWS);
        _new->SetAttribute("cols",      COLS);

        const auto& _m = data_[i];
        for (int r = 0; r < _m.rows(); ++r) {
            sprintf(tmp, "R%02d", r);
            TiXmlElement* _row = new TiXmlElement(tmp);
            for (int c = 0; c < _m.cols(); ++c)
                sprintf(tmp + c*VALUE_SIZE, "%01.05f ", _m(r, c));
            _row->LinkEndChild(new TiXmlText(tmp));
            _new->LinkEndChild(_row);
        }
        p_xmlfd_->RootElement()->LinkEndChild(_new);
    }

    p_xmlfd_->SaveFile(curr_file_name_);
    delete[] tmp;
}

