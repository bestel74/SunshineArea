#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>
#include <QXmlStreamReader>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString GoogleApiElevation(double nord, double est, QString apiKeyPath);
    double getElevation(double nord, double est);
    double XmlReadElevation(QString XML);

private slots:
    void on_pb_request_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
