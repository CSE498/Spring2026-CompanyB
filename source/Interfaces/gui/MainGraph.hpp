#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QVBoxLayout>

namespace cse498 {

class MainGraph : public QWidget {
    Q_OBJECT

private:
    QChartView* mChartView;
    QVBoxLayout* mLayout;

public:
    MainGraph(QWidget* parent = nullptr);

    void ShowLineGraph(const QString& title, const QString& seriesName, const std::vector<double>& data);
    void ShowPieChart(const QString& title, const std::vector<std::pair<QString, double>>& slices);

};

} // namespace cse498