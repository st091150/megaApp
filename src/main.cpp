#include "mainwindow.h"

#include <QApplication>
#include <QStyle>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow w;
  w.setWindowIcon(app.style()->standardIcon(QStyle::SP_ComputerIcon));
  w.show();

  return app.exec();
}
