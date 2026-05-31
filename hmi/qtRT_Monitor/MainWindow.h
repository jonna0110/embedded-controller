#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    QChart *chart;
    QLineSeries *jitterSeries;
    QLineSeries *execSeries;
    QLineSeries *pidSeries;

    QTimer timer;

    int index;

    void updateData();
};

#endif