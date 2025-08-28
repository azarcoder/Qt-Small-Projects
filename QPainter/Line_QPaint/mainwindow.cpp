#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QVector>
#include <QPoint>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("QPainter Line Drawing Example");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    /*
    // draw lines

    // Step 1: Create pixmap canvas
    QPixmap pixmap(200, 100);
    pixmap.fill(Qt::white);   // background

    // Step 2: Draw on pixmap
    QPainter pixPainter(&pixmap);
    pixPainter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(Qt::black);
    pixPainter.setPen(pen);
    pen.setStyle( Qt::DotLine );

    // 1. A single line
    //drawLine(x1,y1,x2,y2);
    //drawLine(start, end)
    pixPainter.drawLine(QPoint(10, 10), QPoint(30, 90));

    // 2. A polyline
    // Creates a zig-zag line through all those points in order:
    // Start at (60,10) → line to (80,90) → line to (75,10) → line to (110,90).
    QPolygon polyPoints;
    polyPoints << QPoint(40, 90)
               << QPoint(60, 10)
               << QPoint(80, 90)
               << QPoint(75, 10)
               << QPoint(110, 90);
    pixPainter.drawPolyline(polyPoints);

    // 3. Multiple independent lines
    QVector<QPoint> linePoints;
    for (const QPoint &point : polyPoints)
        linePoints << point + QPoint(80, 0);   // shifted right

    pixPainter.drawLines(linePoints);
*/
    /*
    // line styls

    QPixmap pixmap( 200, 100 );
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    QPen pen( Qt::black );
    pen.setStyle( Qt::SolidLine );
    painter.setPen( pen );
    painter.drawLine( QPoint( 10, 10 ), QPoint( 190, 10 ) );
    pen.setStyle( Qt::DashDotLine );
    painter.setPen( pen );
    painter.drawLine( QPoint( 10, 50 ), QPoint( 190, 50 ) );
    pen.setDashPattern( QVector<qreal>() << 1 << 1 << 1 << 1 << 2 << 2
                        << 2 << 2 << 4 << 4 << 4 << 4
                        << 8 << 8 << 8 << 8 );

    pen.setStyle( Qt::CustomDashLine );
    painter.setPen( pen );
    painter.drawLine( QPoint( 10, 90 ), QPoint( 190, 90 ) );
    */

    //drawint rectangle to pixmap
    /*
    QPixmap pixmap( 200, 100 );
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    painter.setPen( Qt::black );

    painter.drawRect( 10, 10, 85, 35 );
    painter.drawRoundRect( 10, 55, 85, 35 );

    QRect rect( 105, 10, 85, 35 );
    painter.drawRoundRect( rect );


//     rect starts at (105, 10) and size (85 × 35).
//        rect.translated(0,45) means:

//        Move 0 in x (no horizontal shift).

//        Move 45 in y (downward).

//        So new rect = (105, 55, 85, 35).

    painter.drawRect( rect.translated( 0, 45 ) );
*/
    /*
    //drawing ellipse
    QPixmap pixmap( 200, 190 );
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    painter.setPen( Qt::black );
    painter.drawEllipse( 10, 10, 10, 80 );
    painter.drawEllipse( 30, 10, 20, 80 );
    painter.drawEllipse( 60, 10, 40, 80 );
    painter.drawEllipse( QRect( 110, 10, 80, 80 ) );
    painter.drawArc( 10, 100, 10, 80, 30*16, 240*16 );
    painter.drawArc( 30, 100, 20, 80, 45*16, 200*16 );
    painter.drawArc( 60, 100, 40, 80, 60*16, 160*16 );
    painter.drawArc( QRect( 110, 100, 80, 80 ), 75*16, 120*16 );
    */

    //drawing text

    QPixmap pixmap( 200, 330 );
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    painter.setPen( Qt::black );
    painter.setRenderHints(QPainter::Antialiasing, true);

    QPoint point = QPoint( 10, 20 );
    painter.drawText( point, "You can draw text from a point..." );
    painter.drawLine( point+QPoint(-5, 0), point+QPoint(5, 0) );
    painter.drawLine( point+QPoint(0, -5), point+QPoint(0, 5) );

    //writing inside rectangle
    QRect rect = QRect(10, 30, 180, 20);
    painter.drawText( rect, Qt::AlignCenter,

                      "...or you can draw it inside a rectangle." );

    painter.drawRect( rect );

    //aligment
    rect.translate( 0, 30 );
    QFont font = QApplication::font();
    font.setPixelSize( rect.height() );
    painter.setFont( font );
    painter.drawText( rect, Qt::AlignRight, "Right." );
    painter.drawText( rect, Qt::AlignLeft, "Left." );
    painter.drawRect( rect );

    //To really be able to fit the text into a rectangle, use the QFontMetrics
    rect.translate( 0, rect.height()+10 );
    rect.setHeight( QFontMetrics( font ).height() );
    painter.drawText( rect, Qt::AlignRight, "Right." );
    painter.drawText( rect, Qt::AlignLeft, "Left." );
    painter.drawRect( rect );

    //write paragraph

    //Although it is fairly easy to draw text using the drawText method, you might want to use the
//    QTextDocument class to draw more complex texts. This class enables you to draw complex documents
//    with various formatting and alignments in a straightforward way.

    QTextDocument doc;
    doc.setHtml( "<p>A QTextDocument can be used to present formatted text "

                 "in a nice way.</p>"
                 "<p align=center>It can be <b>formatted</b> "
                 "<font size=+2>in</font> <i>different</i> ways.</p>"
                 "<p>The text can be really long and contain many "
                 "paragraphs. It is properly wrapped and such...</p>" );

    rect.translate( 0, rect.height()+10 );
    rect.setHeight( 160 );
    doc.setTextWidth( rect.width() );
    painter.save();
    painter.translate( rect.topLeft() );
    doc.drawContents( &painter, rect.translated( -rect.topLeft() ) );
    painter.restore();
    painter.drawRect( rect );

    //like footer
    rect.translate( 0, 160 );
    rect.setHeight( doc.size().height()-160 );
    painter.setBrush( Qt::gray );
    painter.drawRect( rect );


    // Step 3: Now paint pixmap onto the MainWindow
    QPainter winPainter(this);
    winPainter.drawPixmap(0, 0, pixmap);
}


