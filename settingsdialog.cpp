/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QFileDialog>
#include <QLineEdit>
#include <qdir.h>
#include <fstream>

QT_USE_NAMESPACE

std::fstream ofd;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    config_doc_(nullptr)
{
    ui->setupUi(this);

    initUIs();
    initSettings();
    loadSettings();
    updateSettings(false);

    connect(ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;

    saveSettings();
    if (config_doc_) {
        delete config_doc_;
        config_doc_ = nullptr;
    }

    ofd.open("cfg", std::ios::out | std::ios::in | std::ios::trunc);
    ofd << settings_.cfg_file.toStdString();
    ofd.close();
}

void SettingsDialog::saveSettings() {
    if (!config_doc_) {
        config_doc_ = new TiXmlDocument;
        if (!config_doc_->LoadFile(settings_.cfg_file.toStdString())) {
            config_doc_->LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));
            config_doc_->LinkEndChild(new TiXmlElement("configures"));
        }
    }

    TiXmlElement* root = config_doc_->RootElement();
    root->SetAttribute("ip",     settings_.ip.toStdString());
    root->SetAttribute("width",  settings_.width);
    root->SetAttribute("height", settings_.height);
    root->SetAttribute("data_path", settings_.data_file.toStdString());
    root->SetAttribute("cfg_path",  settings_.cfg_file.toStdString());

    config_doc_->SaveFile(settings_.cfg_file.toStdString());
}

void SettingsDialog::loadSettings() {
    if (!config_doc_) {
        config_doc_ = new TiXmlDocument;
        if (!config_doc_->LoadFile(settings_.cfg_file.toStdString())) {
            delete config_doc_;
            config_doc_ = nullptr;
            initSettings();
            return;
        }
    }

    TiXmlElement* root = config_doc_->RootElement();
    settings_.ip        = QString(root->Attribute("ip"));

    settings_.data_file = QString(root->Attribute("data_path"));
    settings_.cfg_file  = QString(root->Attribute("cfg_path"));

    root->Attribute("width",    (int*)(&settings_.width));
    root->Attribute("height",   (int*)(&settings_.height));
}

void SettingsDialog::initSettings() {
    settings_.ip   = "10.10.100.254";
    settings_.port = 8899;
    settings_.width = 16;
    settings_.height = 88;
    settings_.data_file = QDir::currentPath() + "/data.xml";

    ofd.open("cfg", std::ios::in | std::ios::out);
    std::string cfg_file;
    ofd >> cfg_file;
    if (!cfg_file.empty()) {
        settings_.cfg_file = QString::fromStdString(cfg_file);
        ofd.close();
        return;
    }
    ofd.close();

    settings_.cfg_file  = QDir::currentPath() + "/cfg.xml";
}

void SettingsDialog::initUIs() {
    for (int i = 1024; i <= 10000; ++i)
        ui->portBox->addItem(QString::number(i));
    for (int i = 2; i <= 100; ++i)
        ui->widthBox->addItem(QString::number(i));
    for (int i = 2; i <= 100; ++i)
        ui->heightBox->addItem(QString::number(i));
}

void SettingsDialog::apply()
{
    updateSettings(true);
    hide();
}

void SettingsDialog::updateSettings(bool update)
{
    if (update) {
        settings_.ip     = ui->ipText->text();
        settings_.port   = ui->portBox->currentText().toInt();
        settings_.height = ui->heightBox->currentText().toInt();
        settings_.width  = ui->widthBox->currentText().toInt();
        settings_.data_file = ui->datapath->text();
        settings_.cfg_file  = ui->cfgpath->text();
    } else {
        ui->ipText->setText(settings_.ip);
        ui->portBox->setCurrentText(QString::number(settings_.port));
        ui->heightBox->setCurrentText(QString::number(settings_.height));
        ui->widthBox->setCurrentText(QString::number(settings_.width));
        ui->datapath->setText(settings_.data_file);
        ui->cfgpath->setText(settings_.cfg_file);
    }
}

void SettingsDialog::on_btnDataLoad_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    dir += "/data.xml";
    ui->datapath->setText(dir);
}

void SettingsDialog::on_btnCfgLoad_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    dir += "/cfg.xml";
    ui->cfgpath->setText(dir);
}
