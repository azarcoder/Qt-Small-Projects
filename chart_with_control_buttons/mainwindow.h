#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include "chartview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onZoomInClicked();
    void onZoomOutClicked();
    void resetView();    // Home

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    ChartView*   m_chartView;
    QChart*       m_chart;
    QLineSeries*  m_series;
    QValueAxis*   m_axisX{nullptr};
    QValueAxis*   m_axisY{nullptr};

    // nav buttons
    QPushButton*  m_homeButton;
    QPushButton*  m_leftButton;
    QPushButton*  m_rightButton;
    QPushButton*  m_panButton;

    // zoom buttons
    QPushButton*  m_zoomInButton;
    QPushButton*  m_zoomOutButton;

    QWidget*      m_buttonOverlay;
    QHBoxLayout*  m_buttonLayout;

    // for resetView()
    qreal         m_initialXMin;
    qreal         m_initialXMax;
    qreal         m_initialYMin;
    qreal         m_initialYMax;

    QIcon makeIcon(const QString &path);
};

#endif // MAINWINDOW_H
