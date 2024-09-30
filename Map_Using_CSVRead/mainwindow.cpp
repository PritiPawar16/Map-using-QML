#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGeoRouteReply>
#include <QGeoRouteRequest>
#include <QQmlContext>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include<QQuickItem>
#include<QMessageBox>
#include <QtConcurrent/QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serviceProvider(nullptr)
    , routingManager(nullptr)
    , m_tableModel(new CustomTableModel(this))

{
    ui->setupUi(this);
    ui->mapQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->mapQuickWidget->setSource(QUrl(QStringLiteral("qrc:/MapView.qml")));
    ui->tableView->setModel(m_tableModel);
    ui->stackedWidget->setCurrentIndex(0);

    serviceProvider = new QGeoServiceProvider("osm");
    if (serviceProvider->error() == QGeoServiceProvider::NoError)
    {
        routingManager = serviceProvider->routingManager();
        if (!routingManager)
        {
            qDebug() << "Failed to get routing manager from service provider.";
        }
    }
    else
    {
        qDebug() << "Error initializing QGeoServiceProvider:" << serviceProvider->errorString();
    }

    connect(ui->openFileButton, &QPushButton::clicked, this, &MainWindow::openGpsFile);
    connect(ui->saveCsvButton, &QPushButton::clicked, this, &MainWindow::saveToCsv);


    QIntValidator *speedValidator1 = new QIntValidator(100, 180, this);
    QIntValidator *speedValidator2= new QIntValidator(0, 99, this);

    ui->RedLineEdit->setValidator(speedValidator1);
    ui->OrangelineEdit->setValidator(speedValidator2);
    ui->YellowlineEdit->setValidator(speedValidator2);
    ui->GreenlineEdit->setValidator(speedValidator2);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete serviceProvider;
}

void MainWindow::openGpsFile()
{
    bool hasExistingData = !ui->mapQuickWidget->rootObject()->property("routePath").toList().isEmpty();

    if (hasExistingData)
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Confirm"));
        msgBox.setText(tr("Existing map data will be cleared. Do you want to proceed?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            clearMapData();

            filePath = QFileDialog::getOpenFileName(this, tr("Open GPS Data File"), "", tr("Text Files (*.txt)"));
            if (!filePath.isEmpty())
            {
                readGpsFile(filePath);
                ui->stackedWidget->setCurrentIndex(0);
            }
        }
    }
    else
    {
        filePath = QFileDialog::getOpenFileName(this, tr("Open GPS Data File"), "", tr("Text Files (*.txt)"));
        if (!filePath.isEmpty())
        {
            readGpsFile(filePath);
            ui->stackedWidget->setCurrentIndex(0);
        }
    }
}
void MainWindow::readGpsFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return;
    }
    QList<QGeoCoordinate> coordinates;
    QList<double> speeds;
    QList<QDateTime> times;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("$GPGLL")) {
            QStringList parts = line.split(',');

            if (parts.size() >= 6) {
                QString latStr = parts[1];
                QString latDir = parts[2];
                QString lonStr = parts[3];
                QString lonDir = parts[4];
                QString speedStr = parts[5];
                QString timeStr = parts[6];

                double latitude = latStr.left(2).toDouble() + (latStr.mid(2).toDouble() / 60.0);
                double longitude = lonStr.left(3).toDouble() + (lonStr.mid(3).toDouble() / 60.0);
                double speed = speedStr.toDouble();

                if (latDir == "S") latitude = -latitude;
                if (lonDir == "W") longitude = -longitude;

                coordinates.append(QGeoCoordinate(latitude, longitude));
                speeds.append(speed);

                QTime time = QTime::fromString(timeStr.left(6), "hhmmss");
                QDate date = QDate::currentDate(); // or any other date if needed
                QDateTime dateTime(date, time, Qt::UTC); // Assuming the time is in UTC
                times.append(dateTime);
            }
        }
    }

    file.close();

    if (!coordinates.isEmpty())
    {
        plotRoute(coordinates, speeds, times);
        calculatedSpeeds = calculateSpeeds(coordinates, times);
        qDebug() << "Calculated speed size  " << calculatedSpeeds.size();


    }
    else
    {
        qDebug() << "No coordinates found in the file.";
    }
}

QList<double> MainWindow::calculateSpeeds(const QList<QGeoCoordinate> &coordinates, const QList<QDateTime> &times)
{
    QList<double> calculatedSpeeds;
    if (coordinates.size() < 2 || times.size() != coordinates.size()) {
        qDebug() << "Invalid data received.";
        return calculatedSpeeds;
    }

    for (int i = 0; i < coordinates.size() - 1; ++i) {
        double distance = haversineDistance(coordinates[i], coordinates[i + 1]); // in kilometers
        double timeDiff = times[i].secsTo(times[i + 1]) / 3600.0; // in hours

        double speed = (timeDiff > 0) ? (distance / timeDiff) : 0; // in km/h
        calculatedSpeeds.append(speed);

    }

    return calculatedSpeeds;
}

void MainWindow::plotRoute(const QList<QGeoCoordinate> &coordinates, const QList<double> &speeds, const QList<QDateTime> &times)
{
    if (coordinates.size() < 2 || speeds.size() != coordinates.size() || times.size() != coordinates.size()) {
        qDebug() << "Invalid data received.";
        return;
    }

    QVariantList path;
    QVariantList speedList;
    QVariantList timeList;
    for (int i = 0; i < coordinates.size(); ++i) {
        path.append(QVariant::fromValue(coordinates[i]));

        double speedKmph = speeds[i];
        speedList.append(QVariant::fromValue(speedKmph));

        QString timeString = times[i].toString("hh:mm");
        timeList.append(QVariant::fromValue(timeString));

        // qDebug()<<"timeList :"<<timeList;
    }

    QGeoCoordinate startCoordinate = coordinates.first();
    QGeoCoordinate endCoordinate = coordinates.last();

    qDebug() << "endCoordinate" << endCoordinate;
    qDebug() << "startCoordinate" << startCoordinate;

    QObject* rootObject = ui->mapQuickWidget->rootObject();

    QMetaObject::invokeMethod(rootObject, "setRouteData",
                              Q_ARG(QVariant, QVariant::fromValue(path)),
                              Q_ARG(QVariant, QVariant::fromValue(speedList)),
                              Q_ARG(QVariant, QVariant::fromValue(timeList)),
                              Q_ARG(QVariant, startCoordinate.latitude()),
                              Q_ARG(QVariant, startCoordinate.longitude()),
                              Q_ARG(QVariant, endCoordinate.latitude()),
                              Q_ARG(QVariant, endCoordinate.longitude()));


    qDebug() << "Path drawn with" << coordinates.size() << "coordinates.";
}

double MainWindow::roundToNDecimalPlaces(double value, int decimals)
{
    double factor = std::pow(10, decimals);
    return std::round(value * factor) / factor;
}

double MainWindow::haversineDistance(const QGeoCoordinate &coord1, const QGeoCoordinate &coord2)
{
    const double R = 6371.0; // Radius of Earth in kilometers

    double lat1 = qDegreesToRadians(coord1.latitude());
    double lon1 = qDegreesToRadians(coord1.longitude());
    double lat2 = qDegreesToRadians(coord2.latitude());
    double lon2 = qDegreesToRadians(coord2.longitude());

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1) * std::cos(lat2) *
                   std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double distance = R * c;

    return distance;
}

void MainWindow::on_openFileLog_clicked()
{
    if (!filePath.isEmpty())
    {
        ui->stackedWidget->setCurrentIndex(1);

        QtConcurrent::run([this]() {

            if (!m_tableModel->loadDataFromFile(filePath))
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to open file."));
            }
            else
            {
              m_tableModel->updateSpeeds(calculatedSpeeds);
            }
        });
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load data .file not open"));
    }

}
void MainWindow::saveToCsv()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save CSV File"), "", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file for writing."));
        return;
    }

    QTextStream out(&file);

    // Write data from the table model to the CSV file
    for (int row = 0; row < m_tableModel->rowCount(); ++row)
    {
        QStringList rowData;
        for (int column = 0; column < m_tableModel->columnCount(); ++column)
        {
            rowData << m_tableModel->data(m_tableModel->index(row, column)).toString();
        }
        out << rowData.join(",") << "\n";
    }

    file.close();

    QMessageBox::information(this, tr("Success"), tr("Data successfully saved to CSV file."));
}


void MainWindow::on_Backbutton_clicked()
{
   ui->stackedWidget->setCurrentIndex(0);
    qDebug()<<"filepath "<<filePath;
}
void MainWindow::applySpeedLimits()
{
    bool ok;
    double speed1 = ui->RedLineEdit->text().toDouble(&ok);
    if (!ok ||( speed1 < 100) || (speed1 > 180)) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid Red Range. It must be between 100 and 180."));
        return;
    }

    double speed2 = ui->OrangelineEdit->text().toDouble(&ok);
    if (!ok || (speed2 < 70 )|| (speed2 > 99)) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid Orange range. It must be between 70 and 99."));
        return;
    }

    double speed3 = ui->YellowlineEdit->text().toDouble(&ok);
    if (!ok || (speed3 < 61) || (speed3 > 69)) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid Yellow Range. It must be between 61 and 69."));
        return;
    }

    double speed4 = ui->GreenlineEdit->text().toDouble(&ok);
    if (!ok || (speed4 < 0) || (speed4 > 60)) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid speed 4 value. It must be between 0 and 60."));
        return;
    }

    // Pass the speed values to QML
    QObject* rootObject = ui->mapQuickWidget->rootObject();
    QMetaObject::invokeMethod(rootObject, "setSpeedValues",
                              Q_ARG(QVariant, speed1),
                              Q_ARG(QVariant, speed2),
                              Q_ARG(QVariant, speed3),
                              Q_ARG(QVariant, speed4));
}

void MainWindow::on_ApplypushButton_clicked()
{
    if(!filePath.isEmpty())
            applySpeedLimits();
    else
        QMessageBox::warning(this, tr("Error"), tr("File not Exist "));
}

void MainWindow::clearMapData()
{
    // Clear map data
    QObject* rootObject = ui->mapQuickWidget->rootObject();
    QMetaObject::invokeMethod(rootObject, "clearRouteData");


    // Optionally reset other UI elements or variables
    filePath.clear();
    calculatedSpeeds.clear();
    ui->stackedWidget->setCurrentIndex(0);

    qDebug() << "Map and data cleared.";
}


