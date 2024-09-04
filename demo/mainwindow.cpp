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
        : thiz_(that) {
        binder_.setupBinder();
    }
    void bind2View(Ui::MainWindowClass* ui);
    QControlBinderImpl      binder_;
    MainWindow              *thiz_; 
    int                     groupRadio_{ 0 };
    bool                    checkbox_{ false };
    int                     comboBox_{ 0 };
    QString                 lineEdit_;
    int                     lineEdit_int_{ 0 };
    double                  lineEdit_double_{ 0.0f };
    QString                 textEdit_;
    int                     spinBox_{ 0 };
    double                  doubleSpinBox_{ 0.0f };
};

void MainWindow::Impl::bind2View(Ui::MainWindowClass* ui)
{
    binder_.bindWith(ui->buttonGroup, groupRadio_);
    binder_.bindWith(ui->label_radio, groupRadio_);

    binder_.bindWith(ui->checkBox, checkbox_);
    binder_.bindWith(ui->label_checkbox, checkbox_);

    binder_.bindWith(ui->comboBox, comboBox_);
    binder_.bindWith(ui->label_comboBox, comboBox_);

    binder_.bindWith(ui->lineEdit, lineEdit_);
    binder_.bindWith(ui->label_lineEdit, lineEdit_);

	binder_.bindWith(ui->lineEdit_int, lineEdit_int_);
	binder_.bindWith(ui->label_lineEdit_int, lineEdit_int_);

	binder_.bindWith(ui->lineEdit_double, lineEdit_double_);
	binder_.bindWith(ui->label_lineEdit_double, lineEdit_double_);

    binder_.bindWith(ui->textEdit, textEdit_);
    binder_.bindWith(ui->label_textEdit, textEdit_);

	binder_.bindWith(ui->spinBox, spinBox_);
    binder_.bindWith(ui->label_spinBox, spinBox_);

	binder_.bindWith(ui->doubleSpinBox, doubleSpinBox_);
    binder_.bindWith(ui->label_doubleSpinBox, doubleSpinBox_);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , impl_(new Impl(this)) {
    ui.setupUi(this);
    impl_->bind2View(&ui);
}

MainWindow::~MainWindow() {
}



//  EventLoop