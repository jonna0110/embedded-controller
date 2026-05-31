#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.resize(800, 600);
    w.show();

    return app.exec();
}

/*
dependicies:
    sudo apt install qtcharts5-dev

*/

/* BUILD & RUN
qmake
make
./rt_monitor
*/
/*
zooming:
chartView->setRubberBand(QChartView::RectangleRubberBand);
*/
/*
logging to csv:
    log << time << jitter << exec << output;
*/
