#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QEvent>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    static const char *buttonStyle = R"(
    QPushButton {
        border: none;
        background-color: transparent;
        border-radius : 8px;
        padding : 5px;
    }
    QPushButton:hover {
        border-radius : 25px;
        background-color: rgba(0, 0, 0, 0.2);    /* darker hover */
        border-radius: 8px;
    }
    QPushButton:checked {
        background-color: rgba(0, 0, 0, 0.3);
        border-radius: 8px;
    }
    QPushButton:pressed {
        background-color: rgba(0, 0, 0, 0.4);
        border-radius: 8px;
    }
    )";



    // –– Chart + axes setup (same as before) ––
    m_chart    = new QChart();
    m_series   = new QLineSeries();
    m_chartView= new ChartView(m_chart, this);
    m_chart->setTheme(QChart::ChartThemeDark);

    // sample data
    m_series->append(0,0);
    m_series->append(1,1);
    m_series->append(2,4);
    m_chart->addSeries(m_series);
    m_chart->createDefaultAxes();
    m_chart->setTitle("Line Chart with Nav & Zoom");

    // capture initial axis ranges for “Home”
    auto axesX = m_chart->axes(Qt::Horizontal, m_series);
    if (!axesX.isEmpty())
        m_axisX = qobject_cast<QValueAxis*>(axesX.first());
    auto axesY = m_chart->axes(Qt::Vertical,   m_series);
    if (!axesY.isEmpty())
        m_axisY = qobject_cast<QValueAxis*>(axesY.first());
    if (m_axisX && m_axisY) {
        m_initialXMin = m_axisX->min();
        m_initialXMax = m_axisX->max();
        m_initialYMin = m_axisY->min();
        m_initialYMax = m_axisY->max();
    }

    m_chartView->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(m_chartView);

    // –– Create all buttons ––
    m_homeButton    = new QPushButton(this);
    m_leftButton    = new QPushButton(this);
    m_rightButton   = new QPushButton(this);
    m_panButton     = new QPushButton(this);
    m_zoomInButton  = new QPushButton(this);
    m_zoomOutButton = new QPushButton(this);

    for (auto btn : { m_homeButton,
         m_leftButton,
         m_rightButton,
         m_panButton,
         m_zoomInButton,
         m_zoomOutButton })
    {
        btn->setStyleSheet(buttonStyle);
    }

    m_panButton->setCheckable(true);

    // –– Style + icon-ify ––
    auto setupBtn = [&](QPushButton* btn, const QString &iconPath, const QString &tip, bool checkable=false){
        btn->setText("");
        btn->setIcon(makeIcon(iconPath));
        btn->setToolTip(tip);
        btn->setCheckable(checkable);
    };

    setupBtn(m_homeButton,    ":/images/icons/home_white.png",       "Home");
    setupBtn(m_leftButton,    ":/images/icons/arrow-left_white.png", "Scroll Left");
    setupBtn(m_rightButton,   ":/images/icons/right-arrow_white.png","Scroll Right");
    setupBtn(m_panButton,     ":/images/icons/move_white.png",       "Pan Tool", true);
    setupBtn(m_zoomInButton,  ":/images/icons/zoom-in_white.png",    "Zoom In");
    setupBtn(m_zoomOutButton, ":/images/icons/zoom-out_white.png",   "Zoom Out");


    // –– Build overlay & layout ––
    m_buttonOverlay = new QWidget(m_chartView->viewport());
    m_buttonOverlay->setCursor(Qt::ArrowCursor);
    m_buttonLayout  = new QHBoxLayout(m_buttonOverlay);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    m_buttonLayout->setSpacing(4);

    // order: Home | Left | Right | Pan | Zoom In | Zoom Out
    m_buttonLayout->addWidget(m_homeButton);
    m_buttonLayout->addWidget(m_leftButton);
    m_buttonLayout->addWidget(m_rightButton);
    m_buttonLayout->addWidget(m_panButton);
    m_buttonLayout->addWidget(m_zoomInButton);
    m_buttonLayout->addWidget(m_zoomOutButton);

    m_buttonOverlay->setStyleSheet("background: transparent;");
    m_buttonOverlay->setFixedSize(6*30 + 5*4, 40); // 6 buttons + spacing
    m_buttonOverlay->installEventFilter(this);
    m_chartView->viewport()->installEventFilter(this);

    // initial overlay placement
    m_buttonOverlay->move(
                m_chartView->viewport()->width() - m_buttonOverlay->width() - 10,
                10
                );
    m_buttonOverlay->raise();

    // –– Signal / slot wiring ––
    connect(m_homeButton,    &QPushButton::clicked, this, &MainWindow::resetView);
    connect(m_leftButton,    &QPushButton::clicked, [=](){ m_chart->scroll(-10, 0); });
    connect(m_rightButton,   &QPushButton::clicked, [=](){ m_chart->scroll( 10, 0); });
    connect(m_panButton,     &QPushButton::toggled,  m_chartView, &ChartView::enablePanning);
    connect(m_zoomInButton,  &QPushButton::clicked, this, &MainWindow::onZoomInClicked);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &MainWindow::onZoomOutClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onZoomInClicked()
{
    m_chart->zoom(1.2);
}

void MainWindow::onZoomOutClicked()
{
    m_chart->zoom(0.8);
}

void MainWindow::resetView()
{
    m_chart->zoomReset();
    if (m_axisX && m_axisY) {
        m_axisX->setRange(m_initialXMin, m_initialXMax);
        m_axisY->setRange(m_initialYMin, m_initialYMax);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == m_chartView->viewport() || watched == m_buttonOverlay)
            && event->type() == QEvent::Resize)
    {
        // keep overlay top-right
        m_buttonOverlay->move(
                    m_chartView->viewport()->width()  - m_buttonOverlay->width()  - 20,
                    20
                    );
    }
    return QMainWindow::eventFilter(watched, event);
}

QIcon MainWindow::makeIcon(const QString &path)
{
    QPixmap pm(path);
    if (pm.isNull())
        return {};
    return QIcon(pm.scaled(18,18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
