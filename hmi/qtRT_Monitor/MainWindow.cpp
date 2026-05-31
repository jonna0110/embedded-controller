#include "MainWindow.h"
#include "process_image.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_NAME "/process_image"

process_image_t *p;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), index(0)
{
    // Open shared memory
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    p = (process_image_t*)mmap(NULL, sizeof(process_image_t),
                              PROT_READ, MAP_SHARED, fd, 0);

    // Chart setup
    chart = new QChart();

    jitterSeries = new QLineSeries();
    jitterSeries->setName("Jitter (µs)");

    execSeries = new QLineSeries();
    execSeries->setName("Exec Time (µs)");

    pidSeries = new QLineSeries();
    pidSeries->setName("PID output");

    chart->addSeries(jitterSeries);
    chart->addSeries(execSeries);
    chart->addSeries(pidSeries);

    chart->createDefaultAxes();

    chart->axisX()->setRange(0, 100);
    chart->axisY()->setRange(0, 1000);

    QChartView *view = new QChartView(chart);
    setCentralWidget(view);

    // Timer (update every 50 ms)
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateData);
    timer.start(50);
}

void MainWindow::updateData()
{
    if (!p) return;

    // versioned read
    uint32_t v1, v2;
    process_image_t local;

    do {
        v1 = p->version;
        local = *p;
        v2 = p->version_end;
    } while (v1 != v2);

    if (local.metrics.jitter > 50) // 50µs
        jitterSeries->setColor(Qt::red);
        else jitterSeries->setColor(Qt::green);

    if (local.metrics.exec_time > 10000000) // 10ms
        execSeries->setColor(Qt::red);
        else execSeries->setColor(Qt::green);

    // Append points
    jitterSeries->append(index, local.metrics.jitter);
    execSeries->append(index, local.metrics.exec_time);
    pidSeries->append(index, local.app.pid_output);

    index++;

    // Keep last 100 samples
    if (jitterSeries->count() > 100) {
        jitterSeries->removePoints(0, 1);
        execSeries->removePoints(0, 1);
        pidSeries->removePoints(0, 1);

        chart->axisX()->setRange(index - 100, index);
    }
}