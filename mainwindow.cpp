#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::GoogleApiElevation(double nord, double est, QString apiKeyPath)
{
    QString strReply;

    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QString request;
    request.append("https://maps.googleapis.com/maps/api/elevation/xml?locations=");
    request.append(QString::number(nord));
    request.append(",");
    request.append(QString::number(est));
    request.append("&key=");

    QFile apikey;
    apikey.setFileName(apiKeyPath);
    apikey.open(QFile::ReadOnly);
    request.append(apikey.readAll().trimmed());
    apikey.close();

    ui->statusBar->showMessage(QString("Requesting... (%1)").arg(request), 0);

    // the HTTP request
    QNetworkRequest req( QUrl(request.toUtf8()) );
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        //success
        strReply = reply->readAll();
    }
    else {
        //failure
        ui->statusBar->showMessage(QString("Request Failed: %1").arg(reply->errorString()), 0);
    }

    delete reply;
    return strReply;
}

double MainWindow::getElevation(double nord, double est)
{
    double elevation;

    QString apiKeyPath = QCoreApplication::applicationDirPath() + QDir::separator() + "API_KEY";
    QString reply = GoogleApiElevation(nord, est, apiKeyPath);
    elevation = XmlReadElevation(reply);

    return elevation;
}

double MainWindow::XmlReadElevation(QString XML)
{
    double elevation = -1;
    if(XML.isEmpty()) return elevation;

    ui->statusBar->showMessage(QString("Parsing XML..."), 0);

    //Reading from the file
    QXmlStreamReader xmlReader(XML);
    xmlReader.readNext();
    while (!xmlReader.isEndDocument())
    {
        QString name = xmlReader.name().toString();
        if (name.contains("elevation"))
        {
            elevation = xmlReader.readElementText().toDouble();
            break;
        }
        xmlReader.readNext();
    }

    ui->statusBar->showMessage(QString("Read XML End"), 0);


    if (xmlReader.hasError())
    {
        ui->statusBar->showMessage(QString("Read XML error: %1").arg(xmlReader.errorString()), 5000);
    }

    return elevation;
}

void MainWindow::on_pb_request_clicked()
{
    int reduceRequestPerSeconds = 0;
    double step = 0.001;
    double ymin = ui->sb_gps_nord->value() - 0.1;
    double ymax = ui->sb_gps_nord->value() + 0.1;
    double xmin = ui->sb_gps_est->value() - 0.1;
    double xmax = ui->sb_gps_est->value() + 0.1;
    int elevation_max = 1000;
    int elevation_min = 0;

    double max_elevation_seen = 0, min_elevation_seen = 100000000;

    ui->graphicsView->scene()->clear();

    for(double y=ymin ; y < ymax ; y += step)
    {
       for(double x=xmin ; x < xmax ; x += step)
       {
           double elevation = getElevation(x, y);
           if(elevation != -1)
           {
               int r = ((elevation - elevation_min) / elevation_max) * 255;
               int g = ((elevation - elevation_min) / elevation_max) * 255;
               int b = ((elevation - elevation_min) / elevation_max) * 255;
               int a = 255;

               ui->graphicsView->scene()->addRect(x*10000,
                                                  y*10000,
                                                  10,
                                                  10,
                                                  QPen(Qt::NoPen),
                                                  QBrush(QColor(r, g, b, a)));
           }
           else
           {
               reduceRequestPerSeconds = 100;
           }

           if(elevation < min_elevation_seen) min_elevation_seen = elevation;
           if(elevation > max_elevation_seen) max_elevation_seen = elevation;

           if(reduceRequestPerSeconds == 100)
           {
               // Spotted by Google, low profile
               reduceRequestPerSeconds = 0;
               QThread::sleep(3);
           }
           else if(++reduceRequestPerSeconds >= 10)
           {
               reduceRequestPerSeconds = 0;
               QThread::sleep(1);
           }
       }
    }

    ui->statusBar->showMessage(QString("Finish, min/max = %1/%2").arg(QString::number(min_elevation_seen))
                                                                 .arg(QString::number(max_elevation_seen)),
                                                                      0);
}
