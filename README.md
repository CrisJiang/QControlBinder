# QControlBinder

QControlBinder is a simple binding library for QWidget controls and C++ POD (Plain Old Data) types, enabling MVVM (Model-View-ViewModel) binding functionality. By including the QControlBinder.hpp file, you can introduce two-way binding (between variables and widget controls) in your project.
![image](https://github.com/CrisJiang/QControlBinder/blob/main/demo.gif)

### First

```c++
#include "../QControlBinder.hpp"
```

### Bind

```c++
class MainWindow::Impl {
public:
    Impl(MainWindow *that)
        : thiz_(that) {
        binder_.setupBinder();          //  初始化binder事件循环
    }
    void bind2View(Ui::MainWindowClass* ui);
    QControlBinderImpl      binder_;    //  创建binder实体
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

//  绑定变量和Widget控间
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
```

### MultiBind

### Watch

use `watch` function to listen value change event

```c++
void MainWindow::Impl::bindWatch(Ui::MainWindowClass* ui)
{
	binder_.bindWith(ui->edit_add1, edit_add1_);
    binder_(edit_add1_).watch([this]() {
        label_addresult_ = edit_add1_ + edit_add2_;
    });
	binder_.bindWith(ui->edit_add2, edit_add2_);
    binder_(edit_add2_).watch([this]() {
        label_addresult_ = edit_add1_ + edit_add2_;
    });
    binder_.bindWith(ui->label_addresult, label_addresult_);

	binder_.bindWith(ui->slider_watch, slider_watch_);
    binder_(slider_watch_).watch([this]() {
        label_percent_ = std::format("{}", (double)slider_watch_/100);
        if (slider_watch_ > 10 && slider_watch_ < 20)
            label_key_ = "red";
        else if (slider_watch_ > 20 && slider_watch_ < 30)
            label_key_ = "yellow";
        else if (slider_watch_ > 30 && slider_watch_ < 40)
            label_key_ = "green";
        else if (slider_watch_ > 40 && slider_watch_ < 50)
            label_key_ = "blue";
        else if (slider_watch_ > 50 && slider_watch_ < 60)
            label_key_ = "white";
    });
	binder_.bindWith(ui->label_percent, label_percent_);
	binder_.bindWith(ui->label_key, label_key_);
}
```
