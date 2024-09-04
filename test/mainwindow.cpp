#include "mainwindow.h"
#include <QDebug>
#include <QImage>
#include <QLineEdit>
#include <QLabel>
#include <QThread>
#include "../utils/log.hpp"
#include "../QControlBinder.hpp"

class MainWindow::Impl {
public:
    Impl(MainWindow *that)
        : that_(that) {
        binder_.setupBinder();
    }
    QControlBinderImpl binder_;
    MainWindow              *that_;
    std::string              editStr_;
    double                   editDouble{0.4f};
    std::vector<std::string> vecStr;
    std::vector<std::string> vecStrAuto;

    //  如何监控一个结构体?
    //  深层次的监听 deep Listen
    struct deepSt {
        int a;
        bool b;
        std::string c;

    };
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , impl_(new Impl(this)) {
    ui.setupUi(this);

    int num = 10000;
    int idx = 0;
    std::vector<QLineEdit*> lineEdits;
    std::vector<QLabel*> labels;
    impl_->vecStr.resize(num);
    lineEdits.resize(num);
    for (auto& edit : lineEdits) {
        auto label = new QLabel(this);
        ui.gridLayout->addWidget(label);
        edit = new QLineEdit(this);
        ui.gridLayout->addWidget(edit);
        impl_->binder_.bindWith(label, impl_->vecStr.at(idx));
        impl_->binder_.bindWith(edit, impl_->vecStr.at(idx));
        idx++;
    }

    impl_->binder_.bindWith(ui.lineEdit, impl_->editStr_);
    impl_->binder_(impl_->editStr_).watch([this]
        (const std::string& val) {
            LOG_INFO("editStr_.{}", val);
        });
	impl_->binder_.bindWith(ui.lineEdit_2, impl_->editDouble);
	impl_->binder_(impl_->editDouble).watch([this]
	(const QVariant& val) {
			LOG_INFO("editDouble.{}", val.toDouble());
		});


   
    //integer.watch([]() {});
}

MainWindow::~MainWindow() {
}

void MainWindow::on_btn_add_clicked()
{
    for (auto& str : impl_->vecStr) {
        str += 'f';
    }
}

std::string rand_str(const int length) {
    std::string str;
    return str;
}


//  EventLoop