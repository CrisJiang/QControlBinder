#pragma once

#include "ui_MainWindow.h"
#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindowClass ui;
    class Impl;
    std::unique_ptr<Impl>    impl_;
};