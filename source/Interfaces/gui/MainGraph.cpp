#include "MainGraph.hpp"

#include <QVBoxLayout>
#include <QtCharts/QChart>

namespace cse498 {

MainGraph::MainGraph(QWidget* parent) : QWidget(parent) {
    mSeries = new QLineSeries();
    mSeries->setName("[Placeholder Traffice data value]");

    // dummy data 
    mSeries->append(0, 0);
    mSeries->append(1, 4);
    mSeries->append(2, 6);
    mSeries->append(3, 2);
    mSeries->append(4, 10);

    QChart* chart = new QChart();
    chart->addSeries(mSeries);
    chart->setTitle("[Placeholder Traffice data title]");
    chart->createDefaultAxes();

    mChartView = new QChartView(chart, this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mChartView);
    setLayout(layout);
}

void MainGraph::AddDataPoint(double x, double y) {
    mSeries->append(x, y);
}

}