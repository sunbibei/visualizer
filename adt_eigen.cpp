#include "adt_eigen.h"
#include <iostream>
#include <QDateTime>

#include <opencv/cv.hpp>
#include <opencv2/core/eigen.hpp>

const unsigned int __INIT_TIMES = 16;

AdtEigen::AdtEigen(size_t r, size_t c)
    : ROWS(r), COLS(c), s_times_(0),
      N_times_(__INIT_TIMES), p_xmlfd_(nullptr)
{
    data_.resize(N_times_);
    for (auto& mat : data_) {
        mat.resize(ROWS, COLS);
        mat.fill(0.0);
    }

    max_val_ = 4;
    min_val_ = 0;

    n_times_    = 0;
    last_times_ = N_times_;
}

AdtEigen::~AdtEigen() {
    if (p_xmlfd_) {
        delete p_xmlfd_;
        p_xmlfd_ = nullptr;
    }
}

void AdtEigen::update(size_t _t, size_t _r, size_t _c, double _v) {
    if (N_times_ == last_times_) last_times_ = _t;
    // if (_t >= N_times_) return;
    if (last_times_ != _t) {
        ++n_times_;
        if (n_times_ == N_times_) {
            saveAll();
            ++s_times_;
            n_times_ = 0;
        }

        last_times_ = _t;
        data_[n_times_].fill(0.0);
    }

    auto& curr = data_[n_times_];
    curr(_r%ROWS, _c%COLS) = _v;
}

cv::Mat AdtEigen::cvtCvMat(size_t a, size_t b, size_t r) {
    const auto& _s = data_[n_times_];
    cv::Mat img = cv::Mat::zeros(a*ROWS, b*COLS, CV_8UC1);

    double scale = 1/(max_val_ - min_val_);

    for (size_t r = 0; r < ROWS; ++r) {
        for (size_t c = 0; c < COLS; ++c) {
           for (size_t ra = a*r; ra < a*r+a; ++ra) {
               for (size_t cb = b*c; cb < b*c+b; ++cb) {
                   img.at<unsigned char>(ra, cb) = _s(r, c)*scale*255;
               }
           }
        }
    }

    // cv::resize(img, img, cv::Size(r*b*COLS, r*a*ROWS));
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
    if (!_fd->LoadFile(out_file_)) {
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

void AdtEigen::save(size_t _t) {
    if (nullptr == p_xmlfd_) {
        p_xmlfd_ = new TiXmlDocument;
        if (!p_xmlfd_->LoadFile(out_file_)) {
            p_xmlfd_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
            p_xmlfd_->LinkEndChild(new TiXmlElement("history"));
        }
    }

    TiXmlElement* _new = new TiXmlElement("record");
    _new->SetAttribute("timestamp", QDateTime::currentDateTime().toTime_t());
    _new->SetAttribute("times",     _t);
    _new->SetAttribute("rows",      ROWS);
    _new->SetAttribute("cols",      COLS);

    const auto& _m = data_[_t%N_times_];
    const size_t BUF_SIZE   = 1024;
    const size_t VALUE_SIZE = 8;
    char* tmp = new char[BUF_SIZE];
    memset(tmp, 0x00, BUF_SIZE*sizeof(char));
    for (int r = 0; r < _m.rows(); ++r) {
        sprintf(tmp, "R%02d", r);
        TiXmlElement* _row = new TiXmlElement(tmp);
        for (int c = 0; c < _m.cols(); ++c)
            sprintf(tmp + c*VALUE_SIZE, "%01.05f ", _m(r, c));
        _row->LinkEndChild(new TiXmlText(tmp));
        _new->LinkEndChild(_row);
    }
    p_xmlfd_->RootElement()->LinkEndChild(_new);
    p_xmlfd_->SaveFile(out_file_);
    delete[] tmp;
}

void AdtEigen::saveAll() {
    if (nullptr == p_xmlfd_) {
        p_xmlfd_ = new TiXmlDocument;
        if (!p_xmlfd_->LoadFile(out_file_)) {
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
        _new->SetAttribute("timestamp", QDateTime::currentDateTime().toTime_t());
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

    p_xmlfd_->SaveFile(out_file_);
    delete[] tmp;
}

