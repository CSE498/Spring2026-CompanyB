#include "MainGraph.hpp"

#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QPieSlice>

namespace cse498 {

MainGraph::MainGraph(QWidget* parent) : QWidget(parent) {
    mLayout = new QVBoxLayout(this);
    mChartView = nullptr;
    setLayout(mLayout);
}

void MainGraph::ShowLineGraph(const QString& title, const QString& seriesName, const std::vector<double>& data, QColor color) {
    auto* series = new QLineSeries();
    series->setName(seriesName);
    series->setColor(color);

    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        double yValue = data[i];
        series->append(i, yValue);
    }

    auto* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->createDefaultAxes();

    auto* newView = new QChartView(chart, this);

    if (mChartView) {
        mLayout->removeWidget(mChartView);
        delete mChartView;
    }

    mChartView = newView;
    mLayout->addWidget(mChartView);
}

void MainGraph::ShowPieChart(const QString& title, const std::vector<std::pair<QString, double>>& slices, const std::vector<QColor>& colors) {
    auto* series = new QPieSeries();

    for (const auto& slice : slices) {
        QString label = slice.first;
        double value = slice.second;
        series->append(label, value);
    }

    auto* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);

    if (!colors.empty()) {
    QList<QPieSlice*> sliceList = series->slices();
    for (int i = 0; i < sliceList.size() && i < colors.size(); ++i) {
        sliceList[i]->setColor(colors[i]);
    }
    }

    auto* newView = new QChartView(chart, this);

    if (mChartView) {
        mLayout->removeWidget(mChartView);
        delete mChartView;
    }

    mChartView = newView;
    mLayout->addWidget(mChartView);
}

}