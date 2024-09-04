#include <windows.h>

#include <QtWidgets/QApplication>
#include <iostream>
#include "../utils/log.hpp"
#include "mainwindow.h"

#if 0
// 自定义消息处理器函数
void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtInfoMsg:
		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		abort();
	}
}
#endif
// 自定义消息处理器函数
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        // 将 qDebug 的输出打印到控制台
        // OutputDebugStringA(localMsg.constData());
        fprintf(stdout, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
        // 其他类型的消息处理可以按需添加
    }
}

void installConsoleOutput()
{
    // 启动控制台
    AllocConsole();
    // 重定向 std::cout 到控制台
    freopen("CONOUT$", "w", stdout);
    // 安装自定义消息处理器
    qInstallMessageHandler(customMessageHandler);
    // 使用 qDebug 输出
    std::cout << "This will be printed to the console." << std::endl;
}


int main(int argc, char *argv[])
{
#if 0
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
	return a.exec();
#endif
    // 1.重定向cout => console
    installConsoleOutput();
    // 2.启动bind事件循环
    
    log_debug("hello {} {}", 42, "cur test");
    log_generic(log_level::debug,"hello {} {}", 42, "cur test");
#if 0
    BindData<int> num(0, "num");
    BindData<int> age(20, "age");

    View view1("View1", "num");
    View view2("View2", "num");
    View view3("View3", "age");

    DataManager &manager = DataManager::getInstance();
#endif
    //eventLoop(manager);
#if 1
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
#endif
}
