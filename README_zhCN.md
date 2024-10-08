# QControlBinder

QControlBinder 是一个简易的 QWidget 控件与 c++ POD 类型变量的 mvvm 绑定库，仅引入`QControlBinder.hpp` 文件，就可以在工程中引入双向绑定（变量与 widget 控件）功能。

### 引入头文件

```c++
#include "../QControlBinder.hpp"
```

### Bind / 绑定

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

### MultiBind / 多重绑定

### Watch / 监听
