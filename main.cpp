#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    qputenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough");
    QApplication a(argc, argv);
    qDebug() << "Các style có sẵn:" << QStyleFactory::keys();
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();
    return a.exec();
}
