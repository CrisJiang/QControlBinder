#ifndef _CONTROL_BINDER_
#define _CONTROL_BINDER_

#include <qbuttongroup.h>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QHash>
#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QVariant>
#include <stdexcept>
#include <QTimer>
#include <QApplication>
#include <QString>
#include <QtConcurrent>
#include <QList>
#include <set>
#include <chrono>

#include "utils/log.hpp"

static const int buttonGroupDefaultIndex = -2; // buttonGroup 如果不设置序号的话，默认开始为 -2
class ControlListenrWarrper {
    using changedFunc                = std::function<void(const QVariant &, const QVariant &)>;
    using stdstringChangedFunc       = std::function<void(const std::string &, const std::string &)>;
    using changedSingleFunc          = std::function<void(const QVariant &)>;
    using stdstringChangedSingleFunc = std::function<void(const std::string &)>;
    struct FuncComparator {
        bool operator()(const changedFunc &lhs, const changedFunc &rhs) const {
            //  比较函数指针, 防止同一函数重复触发
            return lhs.target<size_t()>() < rhs.target<size_t()>();
        }
    };
    struct StringFuncComparator {
        bool operator()(const stdstringChangedFunc &lhs, const stdstringChangedFunc &rhs) const {
            //  比较函数指针
            return lhs.target<size_t()>() < rhs.target<size_t()>();
        }
    };

public:
    explicit ControlListenrWarrper(){};
    explicit ControlListenrWarrper(QMetaObject::Connection &connection, QVariant &variant)
        : connection_(connection)
        , value_(variant){};
    void setVal(const QVariant &variant) {
        value_ = variant;
    };
    void setVal(const std::string &var) {
        value_ = QVariant::fromValue(QString::fromStdString(var));
    };
    void setVal(const char *var) {
        value_ = QVariant::fromValue(QString(var));
    };
    //  回调函数注册， emitOnce 是否立即执行一次
    void watch(const changedFunc &callback, bool emitOnce = false) {
        changedCallbacks_.insert(callback);
        if (emitOnce)
            callback(getVal(), getVal());
    };
    void watch(const stdstringChangedFunc &callback, bool emitOnce = false) {
        stringCallbacks_.insert(callback);
        if (emitOnce)
            callback(getVal().toString().toStdString(), getVal().toString().toStdString());
    };
    void watch(const changedSingleFunc &callback, bool emitOnce = false) {
        auto adaptedFunc = [callback](const QVariant &newVal, const QVariant &oldVal) { callback(newVal); };
        changedCallbacks_.insert(adaptedFunc);
        if (emitOnce)
            callback(getVal());
    };
    void watch(const stdstringChangedSingleFunc &callback, bool emitOnce = false) {
        auto adaptedFunc = [callback](const std::string &newVal, const std::string &oldVal) { callback(newVal); };
        stringCallbacks_.insert(adaptedFunc);
        if (emitOnce)
            callback(getVal().toString().toStdString());
    };
    void watch(const std::function<void()> &lambda, bool emitOnce = false) {
        auto adaptedFunc = [lambda](const QVariant &newVal, const QVariant &oldVal) { lambda(); };
        changedCallbacks_.insert(adaptedFunc);
        if (emitOnce)
            lambda();
    };
    void executeCallback(const QVariant &newVal, const QVariant &oldVal) {
        int i = 0;
        for (auto ite = changedCallbacks_.begin(); ite != changedCallbacks_.end() && i < changedCallbacks_.size();
             ite++) {
            if (*ite) {
                (*ite)(newVal, oldVal);
            }
            i++;
        }
    }
    void executeCallback(const std::string &newVal, const std::string &oldVal) {
        for (auto ite = stringCallbacks_.begin(); ite != stringCallbacks_.end(); ite++) {
            if (*ite) {
                (*ite)(newVal, oldVal);
            }
        }
    }

    QVariant &getVal() {
        return value_;
    };
    void setConnection(const QMetaObject::Connection &connection) {
        connection_ = connection;
    };
    QMetaObject::Connection &getConnection() {
        return connection_;
    };
    char format; //  格式化
    int  len{0}; //  长度
private:
    QMetaObject::Connection                              connection_;
    QVariant                                             value_;
    std::set<changedFunc, FuncComparator>                changedCallbacks_;
    std::set<stdstringChangedFunc, StringFuncComparator> stringCallbacks_;
};

using ControlValMap = QHash<QObject *, std::shared_ptr<ControlListenrWarrper>>;

class ControlBinder : public QObject {
    Q_OBJECT
public:
    ControlBinder(QObject *parent = nullptr)
        : QObject(parent){
              // binder_ = qobject_cast<QControlBinder*>(parent);
          };
    virtual ~ControlBinder(){};
    
    template <typename T>
    void updateVal(std::remove_reference<T>::type& val, std::shared_ptr<ControlListenrWarrper> warrper, T&& newVal) {
        val = newVal;
        warrper->executeCallback(std::forward<T>(newVal), warrper->getVal());
        warrper->setVal(std::forward<T>(newVal));
    };

	template <>
	void updateVal(std::string& val, std::shared_ptr<ControlListenrWarrper> warrper, std::string&& newVal) {
		val = newVal;
		warrper->executeCallback(newVal, warrper->getVal().toString().toStdString());
		warrper->setVal(newVal);
    };

    // 无法使用纯虚函数设计兼容不同的类型，退而求其次
    virtual bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                             QObject *object, int &val, QObject *receiver) {
        return false;
    };
    virtual bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                             QObject *object, double &val, QObject *receiver) {
        return false;
    };
    virtual bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                             QObject *object, bool &val, QObject *receiver) {
        return false;
    };
    virtual bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                             QObject *object, std::string &val, QObject *receiver) {
        return false;
    };
    virtual bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                             QObject *object, QString &val, QObject *receiver) {
        return false;
    };
    virtual void getVal(QObject *control, int &val){};
    virtual void getVal(QObject *control, double &val){};
    virtual void getVal(QObject *control, bool &val){};
    virtual void getVal(QObject *control, std::string &val){};
    virtual void getVal(QObject *control, QString &val){};
};

class TextEditBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, QString &val, QObject *receiver) override {
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(control);
        if (textEdit) {
            connection = QObject::connect(textEdit, &QTextEdit::textChanged, receiver, [&val, textEdit, this, warrper]() {
                updateVal(val, warrper, textEdit->toPlainText());
            });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
};

class TabWidgetBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QTabWidget *tab = qobject_cast<QTabWidget *>(control);
        if (tab) {
            connection =
                QObject::connect(tab, &QTabWidget::currentChanged, receiver, [&val, this, warrper](int value) {
                updateVal(val, warrper, value);
            });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
};

//  QDoubleSpinBox 绑定类
class DoubleSpinBoxBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, double &val, QObject *receiver) override {
        QDoubleSpinBox *spinbox = qobject_cast<QDoubleSpinBox *>(control);
        if (spinbox) {
            connection = QObject::connect(spinbox, &QDoubleSpinBox::valueChanged, receiver,
                                          [this, warrper, &val](double value) {
                    updateVal(val, warrper, value);
                });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
};

//  QSpinBox 绑定类
class SpinBoxBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QSpinBox *spinbox = qobject_cast<QSpinBox *>(control);
        if (spinbox) {
            connection = QObject::connect(spinbox, &QSpinBox::valueChanged, receiver,
                                          [this, warrper, &val](int value) {
                    updateVal(val, warrper, value);
                });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
};

//  QPushButton 绑定类
class PushButtonBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, bool &val, QObject *receiver) override {
        QPushButton *pushbutton = qobject_cast<QPushButton *>(control);
        if (pushbutton) {
            connection = QObject::connect(pushbutton, &QPushButton::clicked, receiver, [this, warrper, pushbutton, &val]() {
                updateVal(val, warrper, !val);
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
};

class ButtonGroupBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QButtonGroup *buttonGroup = qobject_cast<QButtonGroup *>(control);
        if (buttonGroup) {
            connection = QObject::connect(buttonGroup, &QButtonGroup::idClicked, receiver, [this, warrper, &val](int index) {
                updateVal(val, warrper, buttonGroupDefaultIndex - index);
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_INFO("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    void getVal(QObject *control, int &val) override {
        QButtonGroup *buttonGroup = qobject_cast<QButtonGroup *>(control);
        if (buttonGroup) {
            val = buttonGroupDefaultIndex - buttonGroup->checkedId();
        }
    }
};

class ComboBoxBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(control);
        if (comboBox) {
            connection = QObject::connect(comboBox, &QComboBox::currentIndexChanged, receiver, [this, warrper, &val](int index) {
                updateVal(val, warrper, index);
                LOG_INFO("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, bool &val, QObject *receiver) override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(control);
        if (comboBox) {
            connection = QObject::connect(comboBox, &QComboBox::currentIndexChanged, receiver, [this, warrper, &val](int index) {
                updateVal(val, warrper, index>0);
                LOG_INFO("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    void getVal(QObject *control, int &val) override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(control);
        if (comboBox) {
            val = comboBox->currentIndex();
        }
    }
    void getVal(QObject *control, bool &val) override {
        QComboBox *comboBox = qobject_cast<QComboBox *>(control);
        if (comboBox) {
            val = comboBox->currentIndex();
        }
    }
};

class LineEditBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            connection = QObject::connect(lineEdit, &QLineEdit::textChanged, receiver, [this, lineEdit, &val, warrper]() {
                updateVal(val, warrper, lineEdit->text().toInt());
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, double &val, QObject *receiver) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            connection = QObject::connect(lineEdit, &QLineEdit::textChanged, receiver, [this, lineEdit, &val, warrper]() {
                updateVal(val, warrper, lineEdit->text().toDouble());
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, std::string &val, QObject *receiver) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            connection = QObject::connect(lineEdit, &QLineEdit::textChanged, receiver, [this, lineEdit, &val, warrper]() {
                updateVal(val, warrper, lineEdit->text().toStdString());
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, QString &val, QObject *receiver) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            connection = QObject::connect(lineEdit, &QLineEdit::textChanged, receiver, [this, lineEdit, &val, warrper]() {
                updateVal(val, warrper, lineEdit->text());
                LOG_DEBUG("val={}", val.toStdString());
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    void getVal(QObject *control, int &val) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            val = lineEdit->text().toInt();
        }
    }
    void getVal(QObject *control, double &val) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            val = lineEdit->text().toDouble();
        }
    }
    void getVal(QObject *control, std::string &val) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            val = lineEdit->text().toStdString();
        }
    }
    void getVal(QObject *control, QString &val) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            val = lineEdit->text();
        }
    }
};

class CheckBoxBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, bool &val, QObject *receiver) override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(control);
        if (checkBox) {
            LOG_DEBUG("checkBox.bind{}", checkBox->objectName().toStdString());
            connection = QObject::connect(checkBox, &QCheckBox::stateChanged, receiver, [this, warrper, &val](bool state) {
                updateVal(val, warrper, state);
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    void getVal(QObject *control, bool &val) override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(control);
        if (checkBox) {
            val = checkBox->isChecked();
        }
    }
};

class SliderBinder : public ControlBinder {
public:
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, int &val, QObject *receiver) override {
        QSlider *slider = qobject_cast<QSlider *>(control);
        if (slider) {
            connection = QObject::connect(slider, &QSlider::valueChanged, receiver, [this, warrper, &val](int num) {
                updateVal(val, warrper, num);
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    bool makeConnect(QMetaObject::Connection &connection, std::shared_ptr<ControlListenrWarrper> &warrper,
                     QObject *control, double &val, QObject *receiver) override {
        QSlider *slider = qobject_cast<QSlider *>(control);
        if (slider) {
            connection = QObject::connect(slider, &QSlider::valueChanged, receiver, [this, warrper, &val](int num) {
                updateVal(val, warrper, (double)num / 100);
                LOG_DEBUG("val={}", val);
            });
            return connection;
        } else {
            LOG_DEBUG("Error connect control:{}) failed", control->objectName().toStdString());
        }
        return connection;
    }
    void getVal(QObject *control, int &val) override {
        QSlider *slider = qobject_cast<QSlider *>(control);
        if (slider) {
            val = slider->tickInterval();
        }
    }
};

// 控件UI更新类，可扩展
class ControlUpdater {
public:
    virtual ~ControlUpdater(){};
    virtual void update(QObject *object, ControlListenrWarrper *val) = 0;
};

class TextEditUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto       val      = &warper->getVal();
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(control);
        if (textEdit) {
            QString text;
            if (val->typeId() == QMetaType::QString) {
                if (val->toString() == textEdit->toPlainText())
                    return;
                text = val->value<QString>();
            } else {
                text = val->toString();
            }
            textEdit->setText(text);
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class TabWidgetUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto        val = &warper->getVal();
        QTabWidget *tab = qobject_cast<QTabWidget *>(control);
        if (tab) {
            if (val->toInt() == tab->currentIndex())
                return;
            tab->setCurrentIndex(val->toInt());
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class LabelUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto    val   = &warper->getVal();
        QLabel *label = qobject_cast<QLabel *>(control);
        if (label) {
            QString text;
            if (val->typeId() == QMetaType::Double) {
                text = QString::number(val->value<double>(), warper->format, warper->len);
            } else if (val->typeId() == QMetaType::Int) {
                text = QString::number(val->value<int>());
            } else if (val->typeId() == QMetaType::Bool) {
                text = QString::number(val->value<bool>());
            } else if (val->typeId() == QMetaType::QString) {
                text = val->value<QString>();
            } else {
                text = val->toString();
            }
            label->setText(text);
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class DoubleSpinBoxUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto            val     = &warper->getVal();
        QDoubleSpinBox *spinbox = qobject_cast<QDoubleSpinBox *>(control);
        if (spinbox) {
            spinbox->setValue(val->toDouble());
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class SpinBoxUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto      val     = &warper->getVal();
        QSpinBox *spinbox = qobject_cast<QSpinBox *>(control);
        if (spinbox) {
            spinbox->setValue(val->toInt());
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class PushButtonUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto         val        = &warper->getVal();
        QPushButton *pushbutton = qobject_cast<QPushButton *>(control);
        if (pushbutton) {
            bool isOpen = val->toBool();
            pushbutton->setText(isOpen ? "开" : "关");
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class ButtonGroupUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto          val         = &warper->getVal();
        QButtonGroup *buttonGroup = qobject_cast<QButtonGroup *>(control);
        if (buttonGroup) {
            int  id     = val->toInt();
            auto button = buttonGroup->button(buttonGroupDefaultIndex - id);
            if (button) {
                button->setChecked(true);
            } else {
                LOG_INFO("buttonGroup not find id.{}", id);
            }
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class ComboBoxUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto       val      = &warper->getVal();
        QComboBox *comboBox = qobject_cast<QComboBox *>(control);
        if (comboBox) {
            comboBox->setCurrentIndex(val->toInt());
            LOG_DEBUG("setCurrentIndex={}", val->toInt());
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class LineEditUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto       val      = &warper->getVal();
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(control);
        if (lineEdit) {
            QString text;
            if (val->typeId() == QMetaType::Double) {
                text = QString::number(val->value<double>(), warper->format, warper->len);
            } else if (val->typeId() == QMetaType::Int) {
                text = QString::number(val->value<int>());
            } else if (val->typeId() == QMetaType::Bool) {
                text = QString::number(val->value<bool>());
            } else if (val->typeId() == QMetaType::QString) {
                text = val->value<QString>();
            } else {
                text = val->toString();
            }
            lineEdit->setText(text);
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class CheckBoxUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto       val      = &warper->getVal();
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(control);
        if (checkBox) {
            checkBox->setChecked(val->toBool());
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class SliderUpdater : public ControlUpdater {
public:
    void update(QObject *control, ControlListenrWarrper *warper) override {
        auto val    = &warper->getVal();
        auto slider = qobject_cast<QSlider *>(control);
        if (slider) {
            if (val->typeId() == QMetaType::Int)
                slider->setValue(val->toInt());
            else if (val->typeId() == QMetaType::Double)
                slider->setValue(val->toDouble() * 100);
        } else {
            LOG_DEBUG("Error(control:{}, value:{}) failed", control->objectName().toStdString(), val->typeName());
        }
    }
};

class VData {
public:
    enum class DataType {
        INT,
        DOUBLE,
        BOOLEAN,
        STDSTRING,
        QSTRING,
    };
    /*template<typename T>
    VData(T* data) : typename_(typeid(*data).name()){
        p_data_ = reinterpret_cast<void*>(data);
    };*/
    VData(const VData &vdata)
        : p_data_(vdata.p_data_)
        , type_(vdata.type_){};
    // 模板无法记录变量类型，手动操作
    VData(int *data)
        : type_(DataType::INT) {
        if (data == nullptr) {
            throw std::invalid_argument("data pointer cannot be nullptr");
        }
        p_data_ = static_cast<void *>(data);
    };

    VData(double *data)
        : type_(DataType::DOUBLE) {
        if (data == nullptr) {
            throw std::invalid_argument("data pointer cannot be nullptr");
        }
        p_data_ = static_cast<void *>(data);
    };

    VData(bool *data)
        : type_(DataType::BOOLEAN) {
        if (data == nullptr) {
            throw std::invalid_argument("data pointer cannot be nullptr");
        }
        p_data_ = static_cast<void *>(data);
    };

    VData(std::string *data)
        : type_(DataType::STDSTRING) {
        if (data == nullptr) {
            throw std::invalid_argument("data pointer cannot be nullptr");
        }
        p_data_ = static_cast<void *>(data);
    };

    VData(QString *data)
        : type_(DataType::QSTRING) {
        if (data == nullptr) {
            throw std::invalid_argument("data pointer cannot be nullptr");
        }
        p_data_ = static_cast<void *>(data);
    };

    void *data() {
        return p_data_;
    }

    template <typename T> bool getVal(DataType type, T &transVal) const { // 获取VData监听的变量的值
        if (type == type_) {
            transVal = *static_cast<T *>(p_data_);
            return true;
        }
        return false;
    }

    DataType getType() const {
        return type_;
    }
    // 实现QHash
    bool operator==(const VData &other) const {
        return this->p_data_ == other.p_data_;
    };

    VData &operator=(const VData &other) {
        if (this == &other)
            return *this;
        p_data_ = other.p_data_;
        type_   = other.type_;
        return *this;
    };
    // 实现QHash
    uintptr_t getPtrAddr() const {
        return reinterpret_cast<uintptr_t>(p_data_);
    };

private:
    void    *p_data_{nullptr};
    DataType type_;
};

// 重载QHash函数
static size_t qHash(const VData &key, uint seed = 0) {
    return qHash(key.getPtrAddr(), seed);
}

// 一个简单的Qt控件双向绑定类，信号槽实现，线程不安全
class QControlBinder : public QObject {
    Q_OBJECT

    using VaribleControlMap = QHash<VData, QSet<QObject *>>;
private:
    class Impl {
    public:
        Impl(QControlBinder *thiz)
            : thiz_(thiz){};
        QControlBinder   *thiz_;
        QTimer           *eventLoop_{nullptr};
        bool              enableWatchCast{true}; //  默认开启watch
        ControlValMap     control_variant_map;   //	记录对象和对象应该赋的值
        VaribleControlMap varible_control_map;   //	记录变量和对应的QObject对象
        int               vInt_{0};              //  在类中创建
        double            vDouble_{0.0};
        bool              vBool_{false};
        std::string       vStr_{""};
        QString           vQStr_{""};

        template <typename T> void notifyUpdater(VData &vData, const T &value) {
            if (varible_control_map.contains(vData)) {
                auto controlSet = varible_control_map.value(vData);
                for (const auto control : controlSet) {
                    if (control_variant_map.contains(control)) {
                        auto ite = control_variant_map.find(control);
                        ite->setVal(value);
                        emit updateControl(control);
                    }
                }
            }
        };

        void emitUpdateControl(QObject *control) {
            emit thiz_->updateControl(control);
        };
    };
    std::unique_ptr<Impl> impl_;

public:
    explicit QControlBinder(QObject *parent = nullptr)
        : QObject(parent)
        , impl_(new Impl(this)) {
    }

    ~QControlBinder() {
        this->disconnect();
        impl_->control_variant_map.clear();
        impl_->varible_control_map.clear();
    }

    auto &getControl_variant_map() {
        return impl_->control_variant_map;
    }

    template <typename T> ControlListenrWarrper &operator()(T &variable) {
        auto vData      = VData(&variable);
        auto controlSet = impl_->varible_control_map[vData];           //  找到变量
        return *impl_->control_variant_map[*controlSet.begin()].get(); //  return vData;
    }

    void setupBinder() {
        impl_->eventLoop_ = new QTimer(qApp); //  采用单一的全局定时器
        //  确保在主线程中更新界面ui
        connect(this, &QControlBinder::updateControl, this, &QControlBinder::updateVar2Control);
        connect(impl_->eventLoop_, &QTimer::timeout, this, &QControlBinder::tryUpdateView);
        connect(this, &QControlBinder::variantChanged, this, &QControlBinder::watchVariantHolder);
        connect(this, &QControlBinder::stdstringChanged, this, &QControlBinder::watchStdStringHolder);
        impl_->eventLoop_->setSingleShot(true); //  每次qt事件循环时触发
        impl_->eventLoop_->start(20);
    };

    void setWatch(bool isWatch) {
        //  如果从 false -> true 先同步一遍 所有varint的值, 达到不响应这期间的值的变化
        if (!impl_->enableWatchCast && isWatch) {
            tryUpdateView();
        }
        impl_->enableWatchCast = isWatch;
    }
    virtual std::unique_ptr<ControlBinder> getBinderFromControl(QObject *control) = 0;

    //  单向绑定
    template <typename Obj, typename T> void bindModel(Obj *control, T &val) {
        if (impl_->control_variant_map.contains(control))
            return;
        //  绑定信号槽, 类型判断, 并信号槽绑定
        auto listenWarrper = std::make_shared<ControlListenrWarrper>();
        listenWarrper->setVal(val);
        impl_->control_variant_map.insert(control, listenWarrper); // 维护监听
        auto vData = VData(&val);
        if (impl_->varible_control_map.contains(vData)) {
            impl_->varible_control_map[vData].insert(control);
        } else {
            QSet<QObject *> controlSet;
            controlSet.insert(control);
            impl_->varible_control_map.insert(vData, controlSet);
        }
        emit updateControl(control);
    };

    //  绑定
    template <typename Obj, typename T> void bindWith(Obj *control, T &val, char format, int len) {
        if (impl_->control_variant_map.contains(control))
            return;
        // 绑定destoryed信号
        connect(control, &QObject::destroyed, this, [control, this]() { disBindWith(control); });
        //  绑定信号槽, 类型判断, 并信号槽绑定
        auto                    listenWarrper = std::make_shared<ControlListenrWarrper>();
        QMetaObject::Connection connection;
        auto                    binder = getBinderFromControl(control);
        if (binder) {
            if (binder->makeConnect(connection, listenWarrper, control, val, this)) {
                //  记录信号槽 记录控件监听
                listenWarrper->setConnection(connection);
            }
        }
        listenWarrper->format = format;
        listenWarrper->len    = len;
        listenWarrper->setVal(val);
        impl_->control_variant_map.insert(control, std::move(listenWarrper)); // 维护监听

        auto vData = VData(&val);
        if (impl_->varible_control_map.contains(vData)) {
            impl_->varible_control_map[vData].insert(control);
        } else {
            QSet<QObject *> controlSet;
            controlSet.insert(control);
            impl_->varible_control_map.insert(vData, controlSet);
        }
        emit updateControl(control);
    };

    template <typename Obj, typename T> void bindWith(Obj *obj, T &val) {
        bindWith(obj, val, ' ', 0);
    }

    template <> void bindWith(QLineEdit *lineEdit, double &val) {
        bindWith(lineEdit, val, 'f', 4);
    }

    template <> void bindWith(QLabel *label, double &val) {
        bindWith(label, val, 'f', 4);
    }

    template <typename Obj, typename T, typename U>
        requires std::is_same_v<Obj, QDoubleSpinBox> || std::is_same_v<Obj, QSpinBox>
    void bindWith(Obj *spinbox, T &val, U step, U min, U max) {
        spinbox->setSingleStep(step);
        spinbox->setMinimum(min);
        spinbox->setMaximum(max);
        bindWith(spinbox, val);
    }

    template <typename T> void bindWith(QSlider *slider, T &val, int min, int max) {
        slider->setMinimum(min);
        slider->setMaximum(max);
        bindWith(slider, val);
    }

    template <typename Obj, typename T> void rebindWith(Obj *control, T &val) {
        disBindWith(control, val);
        bindWith(control, val);
    }

    template <typename Obj> void disBindWith(Obj *control) {
        if (impl_->control_variant_map.contains(control)) {
            disconnect(impl_->control_variant_map[control]->getConnection());
            impl_->control_variant_map.remove(control);
            // 解绑是解除原先 QObjcet 对应绑定的对象, 暴力搜索, 暂时没想到更好的方式
            auto ite = impl_->varible_control_map.begin();
            while (ite != impl_->varible_control_map.end()) {
                auto &controlSet = ite.value();
                if (controlSet.contains(control)) {
                    controlSet.remove(control);
                    if (controlSet.size() == 0)
                        impl_->varible_control_map.erase(ite);
                    break;
                }
                ++ite;
            }
        }
    };

    template <typename Obj, typename T> void disBindWith(Obj *control, T &var) {
        if (impl_->control_variant_map.contains(control)) {
            disconnect(impl_->control_variant_map[control]->getConnection());
            impl_->control_variant_map.remove(control);
            // 解绑是解除原先 QObjcet 对应绑定的对象, 暴力搜索, 暂时没想到更好的方式
            auto ite = impl_->varible_control_map.begin();
            while (ite != impl_->varible_control_map.end()) {
                auto &controlSet = ite.value();
                if (controlSet.contains(control)) {
                    controlSet.remove(control);
                    if (controlSet.size() == 0)
                        impl_->varible_control_map.erase(ite);
                    break;
                }
                ++ite;
            }
        }

        auto vData = VData(&var);
        if (impl_->varible_control_map.contains(vData))
            impl_->varible_control_map.remove(vData);
    };

signals:
    void updateControl(QObject *);
    void variantChanged(QObject *, const QVariant &, const QVariant);
    void stdstringChanged(QObject *, const std::string &, const std::string);

private slots:
    void tryUpdateView() {
        //  遍历变量，观测变量的值是否变化
        //  如果变量的值变化，应该通知控件更新
        //  如果是控件流向变量的改变，则不应该再本事件中触发更新
        //  [1] 状态标记，如果是来自界面的操作，则不触发更新
        //  [2] 建立update表，防止update对象总是创建
        //  [3] 明确数据流向顺序 先-> 数据 > Ui 而后 Ui -> 数据
        for (auto &vData : impl_->varible_control_map.keys()) {
            auto &controlSet = impl_->varible_control_map[vData]; //  找到变量
            for (const auto control : controlSet) {
                auto &variant = impl_->control_variant_map[control];
                switch (vData.getType()) {
                case VData::DataType::INT:
                    if (vData.getVal(VData::DataType::INT, impl_->vInt_)) // 根据vData 拿到原先变量的值
                    {
                        if (impl_->vInt_ != variant->getVal().value<int>()) {
                            if (impl_->enableWatchCast)
                                emit variantChanged(control, impl_->vInt_, variant->getVal());
                            variant->getVal() = impl_->vInt_;
                            auto updater      = getUpdaterFromControl(control);
                            if (updater) {
                                updater->update(control, variant.get());
                            }
                            // notifyUpdater(vData, vInt_);
                        }
                    }

                    break;
                case VData::DataType::DOUBLE:
                    if (vData.getVal(VData::DataType::DOUBLE, impl_->vDouble_)) {
                        if (impl_->vDouble_ != variant->getVal().value<double>()) {
                            if (impl_->enableWatchCast)
                                emit variantChanged(control, impl_->vDouble_, variant->getVal());
                            variant->getVal() = impl_->vDouble_;
                            auto updater      = getUpdaterFromControl(control);
                            if (updater) {
                                updater->update(control, variant.get());
                            }
                            // notifyUpdater(vData, vDouble_);
                        }
                    }
                    break;
                case VData::DataType::BOOLEAN:
                    if (vData.getVal(VData::DataType::BOOLEAN, impl_->vBool_)) {
                        if (impl_->vBool_ != variant->getVal().value<bool>()) {
                            if (impl_->enableWatchCast)
                                emit variantChanged(control, impl_->vBool_, variant->getVal());
                            variant->getVal() = impl_->vBool_;
                            auto updater      = getUpdaterFromControl(control);
                            if (updater) {
                                updater->update(control, variant.get());
                            }
                            // notifyUpdater(vData, vBool_);
                        }
                    }
                    break;
                case VData::DataType::STDSTRING:
                    if (vData.getVal(VData::DataType::STDSTRING, impl_->vStr_)) {
                        std::string oldStr = variant->getVal().toString().toStdString();
                        if (impl_->vStr_.compare(oldStr)) {
                            if (impl_->enableWatchCast)
                                emit stdstringChanged(control, impl_->vStr_, oldStr);
                            variant->getVal() = QVariant::fromValue(QString::fromStdString(impl_->vStr_));
                            auto updater      = getUpdaterFromControl(control);
                            if (updater) {
                                updater->update(control, variant.get());
                            }
                            // notifyUpdater(vData, vStr_);
                        }
                    }
                    break;
                case VData::DataType::QSTRING:
                    if (vData.getVal(VData::DataType::QSTRING, impl_->vQStr_)) {
                        if (impl_->vQStr_ != variant->getVal().value<QString>()) {
                            if (impl_->enableWatchCast)
                                emit variantChanged(control, impl_->vQStr_, variant->getVal());
                            variant->getVal() = impl_->vQStr_;
                            auto updater      = getUpdaterFromControl(control);
                            if (updater) {
                                updater->update(control, variant.get());
                            }
                            // notifyUpdater(vData, vStr_);
                        }
                    }
                    break;
                }
            }
        }
        impl_->eventLoop_->start(2); //  一下一轮事件循环中再触发
    }
    void watchVariantHolder(QObject *control, const QVariant &newVal, const QVariant oldVal) {
        auto &warrper = impl_->control_variant_map[control];
        warrper->executeCallback(newVal, oldVal);
    }
    void watchStdStringHolder(QObject *control, const std::string &newVal, const std::string oldVal) {
        auto &warrper = impl_->control_variant_map[control];
        warrper->executeCallback(newVal, oldVal);
    }
    // 强制更新
    void updateVar2Control(QObject *control) {
        if (impl_->control_variant_map.contains(control)) {
            auto &variant = impl_->control_variant_map[control];
            auto  updater = getUpdaterFromControl(control);
            if (updater) {
                updater->update(control, variant.get());
            } else {
                LOG_DEBUG("control:{} not find updater", control->objectName().toStdString());
            }
        }
    };
    virtual std::unique_ptr<ControlUpdater> getUpdaterFromControl(QObject *control) = 0;
};

class QControlBinderImpl : public QControlBinder {
public:
    explicit QControlBinderImpl(QObject *parent = nullptr)
        : QControlBinder(parent){};
    void setupBinder() {
        return QControlBinder::setupBinder();
    };
    void setWatch(bool enable) {
        return QControlBinder::setWatch(enable);
    };
    virtual std::unique_ptr<ControlBinder> getCustomBinder(QObject *control) {
        return nullptr;
    };
    virtual std::unique_ptr<ControlUpdater> getCustomUpdater(QObject *control) {
        return nullptr;
    };

private:
    virtual std::unique_ptr<ControlBinder> getBinderFromControl(QObject *control) override final {
        if (qobject_cast<QComboBox *>(control)) {
            return std::make_unique<ComboBoxBinder>();
        } else if (qobject_cast<QLineEdit *>(control)) {
            return std::make_unique<LineEditBinder>();
        } else if (qobject_cast<QCheckBox *>(control)) {
            return std::make_unique<CheckBoxBinder>();
        } else if (qobject_cast<QSlider *>(control)) {
            return std::make_unique<SliderBinder>();
        } else if (qobject_cast<QButtonGroup *>(control)) {
            return std::make_unique<ButtonGroupBinder>();
        } else if (qobject_cast<QPushButton *>(control)) {
            return std::make_unique<PushButtonBinder>();
        } else if (qobject_cast<QSpinBox *>(control)) {
            return std::make_unique<SpinBoxBinder>();
        } else if (qobject_cast<QDoubleSpinBox *>(control)) {
            return std::make_unique<DoubleSpinBoxBinder>();
        } else if (qobject_cast<QTabWidget *>(control)) {
            return std::make_unique<TabWidgetBinder>();
        } else if (qobject_cast<QTextEdit *>(control)) {
            return std::make_unique<TextEditBinder>();
        }
        return getCustomBinder(control);
    };

    virtual std::unique_ptr<ControlUpdater> getUpdaterFromControl(QObject *control) override final {
        if (qobject_cast<QComboBox *>(control)) {
            return std::make_unique<ComboBoxUpdater>();
        } else if (qobject_cast<QLineEdit *>(control)) {
            return std::make_unique<LineEditUpdater>();
        } else if (qobject_cast<QCheckBox *>(control)) {
            return std::make_unique<CheckBoxUpdater>();
        } else if (qobject_cast<QSlider *>(control)) {
            return std::make_unique<SliderUpdater>();
        } else if (qobject_cast<QButtonGroup *>(control)) {
            return std::make_unique<ButtonGroupUpdater>();
        } else if (qobject_cast<QPushButton *>(control)) {
            return std::make_unique<PushButtonUpdater>();
        } else if (qobject_cast<QLabel *>(control)) {
            return std::make_unique<LabelUpdater>();
        } else if (qobject_cast<QSpinBox *>(control)) {
            return std::make_unique<SpinBoxUpdater>();
        } else if (qobject_cast<QDoubleSpinBox *>(control)) {
            return std::make_unique<DoubleSpinBoxUpdater>();
        } else if (qobject_cast<QTabWidget *>(control)) {
            return std::make_unique<TabWidgetUpdater>();
        } else if (qobject_cast<QTextEdit *>(control)) {
            return std::make_unique<TextEditUpdater>();
        }
        return getCustomUpdater(control);
    };
};

#endif // _CONTROL_BINDER_