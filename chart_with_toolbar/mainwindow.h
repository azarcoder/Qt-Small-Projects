#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QValueAxis>
#include "chartview.h"

QT_CHARTS_USE_NAMESPACE

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
    void resetView();
    void saveChart();

private:
    QChart *m_chart;
    ChartView *m_chartView;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    qreal m_initialXMin, m_initialXMax;
    qreal m_initialYMin, m_initialYMax;
    Ui::MainWindow *ui;

    QIcon makeIcon(const QString Path);
};
#endif // MAINWINDOW_H
