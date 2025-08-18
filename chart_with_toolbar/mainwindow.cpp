#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QToolBar>
#include <QAction>
#include <QIcon>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Chart with Toolbar");

    m_chart = new QChart();
    m_chartView = new ChartView(m_chart);

    m_chart->setTheme(QChart::ChartThemeDark);

    // Series A
    QLineSeries *seriesA = new QLineSeries();
    seriesA->setName("A");
    seriesA->append(0, 0);
    seriesA->append(1, 5);
    seriesA->append(2, 3);
    seriesA->append(3, 10);
    seriesA->append(4, 7);

    // Series B
    QLineSeries *seriesB = new QLineSeries();
    seriesB->setName("B");
    seriesB->append(0, 10);
    seriesB->append(1, 15);
    seriesB->append(2, 13);
    seriesB->append(3, 25);
    seriesB->append(4, 10);

    m_chart->addSeries(seriesA);
    m_chart->addSeries(seriesB);
    m_chart->createDefaultAxes();
    m_chart->setTitle("Line Chart Example");

    m_chartView->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(m_chartView);

    m_chart->createDefaultAxes();

    auto axesX = m_chart->axes(Qt::Horizontal, seriesA);
    if (!axesX.isEmpty())
        m_axisX = qobject_cast<QValueAxis *>(axesX.first());

    auto axesY = m_chart->axes(Qt::Vertical, seriesA);
    if (!axesY.isEmpty())
        m_axisY = qobject_cast<QValueAxis *>(axesY.first());


    if (m_axisX && m_axisY) {
        m_initialXMin = m_axisX->min();
        m_initialXMax = m_axisX->max();
        m_initialYMin = m_axisY->min();
        m_initialYMax = m_axisY->max();
    }

    // Toolbar
    QToolBar *toolbar = addToolBar("Navigation");

    QAction *homeAction = toolbar->addAction(makeIcon(":/icons/icons/home.png"), "Home");
    connect(homeAction, &QAction::triggered, this, &MainWindow::resetView);

        QAction *leftAction = toolbar->addAction(makeIcon(":/icons/icons/arrow-left.png"), "Left");
    connect(leftAction, &QAction::triggered, this, [=]() {
        m_chart->scroll(-10, 0);
    });

    QAction *rightAction = toolbar->addAction(makeIcon(":/icons/icons/right-arrow.png"), "Right");
    connect(rightAction, &QAction::triggered, this, [=]() {
        m_chart->scroll(10, 0);
    });

    toolbar->addSeparator();

    QAction *panAction = new QAction(makeIcon(":/icons/icons/move.png"), "Pan Tool", this);
    panAction->setCheckable(true);
    toolbar->addAction(panAction);

    connect(panAction, &QAction::toggled, m_chartView, &ChartView::enablePanning);

    QAction *zoomInAction = toolbar->addAction(makeIcon(":/icons/icons/zoom-in.png"), "Zoom In");
    connect(zoomInAction, &QAction::triggered, this, [=]() {
        m_chart->zoom(1.2);
    });

    QAction *zoomOutAction = toolbar->addAction(makeIcon(":/icons/icons/zoom-out.png"), "Zoom Out");
    connect(zoomOutAction, &QAction::triggered, this, [=]() {
        m_chart->zoom(0.8);
    });


    QAction *saveAction = toolbar->addAction(makeIcon(":/icons/icons/save.png"), "Save");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetView()
{
    m_chart->zoomReset();
    if (m_axisX && m_axisY) {
        m_axisX->setRange(m_initialXMin, m_initialXMax);
        m_axisY->setRange(m_initialYMin, m_initialYMax);
    }
}

void MainWindow::saveChart()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Chart", "", "PNG Files (*.png)");
    if (!filename.isEmpty()) {
        QPixmap pixmap = m_chartView->grab();
        pixmap.save(filename, "PNG");
    }
}

QIcon MainWindow::makeIcon(const QString path)
{
    QPixmap pm(path);
    if (pm.isNull())
        return QIcon();
    return QIcon (QPixmap(path).scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

