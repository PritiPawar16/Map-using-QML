#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGeoServiceProvider>
#include <QGeoRoutingManager>
#include <QFileDialog>
#include <QQmlContext>
#include <QGeoCoordinate>
#include "customtablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openGpsFile();
    void readGpsFile(const QString &filePath);
    double roundToNDecimalPlaces(double value, int decimals);
    void plotRoute(const QList<QGeoCoordinate> &coordinates, const QList<double> &speeds, const QList<QDateTime> &times);
    double haversineDistance(const QGeoCoordinate &coord1, const QGeoCoordinate &coord2);
    void on_openFileLog_clicked();
    void on_Backbutton_clicked();
    QList<double> calculateSpeeds(const QList<QGeoCoordinate> &coordinates, const QList<QDateTime> &times);
    void saveToCsv();
    void applySpeedLimits();
    void on_ApplypushButton_clicked();
    void clearMapData();
private:
    Ui::MainWindow *ui;
    QGeoServiceProvider *serviceProvider;
    QGeoRoutingManager *routingManager;
    QGeoCoordinate startCoordinate;
    QGeoCoordinate endCoordinate;
    QList<QPair<QGeoCoordinate, int>> plottedCoordinates;
    QString filePath;
    CustomTableModel *m_tableModel;
    QList<double> calculatedSpeeds;


};

#endif // MAINWINDOW_H
