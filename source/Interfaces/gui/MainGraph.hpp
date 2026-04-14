#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace cse498 {

class MainGraph : public QWidget {
    Q_OBJECT

private:
    QChartView* mChartView;
    QLineSeries* mSeries;

public:
    MainGraph(QWidget* parent = nullptr);

    void AddDataPoint(double x, double y);

};

} 