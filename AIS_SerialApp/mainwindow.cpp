#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow *g_pMainwindow = NULL;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    g_pMainwindow = this;
    init_gui();
    connect_signals();

    m_NewDeviceTimer = new QTimer(this);
    connect(m_NewDeviceTimer, &QTimer::timeout, this, &MainWindow::load_available_com_ports_into_comboBox);
    m_NewDeviceTimer->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_serial_link_sts(bool sts)
{
    QString status = sts ? "Connected" : "Disconnected";
    ui->label_link_sts->setText(status);
}

void MainWindow::init_gui()
{
    mObj_SerialCommunication = nullptr;
    mObj_SerialCommunication = new SerialCore(this);

    load_available_com_ports_into_comboBox();
    enable_disable_uart_connection_button(false);
}

void MainWindow::load_available_com_ports_into_comboBox()
{
    ui->comboBox->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &port : ports)
    {
        ui->comboBox->addItem(port.portName());
    }
}

void MainWindow::enable_disable_uart_connection_button(bool sts)
{
    if(sts)
    {
        ui->pushButton_uart_open->setDisabled(true);
        ui->pushButton_uart_close->setDisabled(false);
        ui->comboBox->setDisabled(true);
        ui->pushButton_uart_open->setStyleSheet("background-color: gray; color : white;");
        ui->pushButton_uart_close->setStyleSheet("background-color: rgb(220, 20, 60);color:#fff");

    }
    else
    {
        ui->pushButton_uart_open->setDisabled(false);
        ui->comboBox->setDisabled(false);
        ui->pushButton_uart_close->setDisabled(true);

        ui->pushButton_uart_close->setStyleSheet("background-color: gray; color : white;");
        ui->pushButton_uart_open->setStyleSheet("color:#fff;background-color: rgb(34, 139, 34);");
    }
}

QString MainWindow::format_ais_type18(const S_AIS_TYPE18_FULL *in_pS)
{
    QString str;
    str += QString::asprintf("\n--- AIS TYPE 18 (CLASS B) REPORT ---\n");
    str += QString::asprintf("MMSI             : %u\n", in_pS->uiMMSI);
    str += QString::asprintf("SOG              : %.1f kn\n", in_pS->fSOG);
    str += QString::asprintf("COG              : %.1f deg\n", in_pS->fCOG);
    str += QString::asprintf("Heading          : %u\n", in_pS->usHeading);
    str += QString::asprintf("Longitude        : %.5f\n", in_pS->fLon);
    str += QString::asprintf("Latitude         : %.5f\n", in_pS->fLat);
    str += QString::asprintf("Timestamp        : %u\n", in_pS->ucTimestamp);

    if (in_pS->iValidateStatus != AIS_OK) {
        str += "VALIDATION: FAILED\n";
    } else {
        str += "VALIDATION: OK\n";
    }
    return str;
}

QString MainWindow::format_ais_type123(const S_AIS_TYPE123_FULL *in_pS)
{
    QString str;

    // Using asprintf allows you to reuse your existing printf format strings easily
    str += QString::asprintf("\n--- AIS TYPE %u POSITION REPORT ---\n", in_pS->ucMsgID);
    str += QString::asprintf("Repeat Indicator : %u\n", in_pS->ucRepeat);
    str += QString::asprintf("MMSI             : %u\n", in_pS->uiMMSI);
    str += QString::asprintf("Nav Status       : %u\n", in_pS->ucNav_Status);
    str += QString::asprintf("ROT              : %d\n", in_pS->cROT);
    str += QString::asprintf("SOG              : %.1f kn\n", in_pS->fSOG_kn);
    str += QString::asprintf("Position Acc     : %u\n", in_pS->ucPos_Acc);
    str += QString::asprintf("Longitude        : %.5f\n", in_pS->fLon);
    str += QString::asprintf("Latitude         : %.5f\n", in_pS->fLat);
    str += QString::asprintf("COG              : %.1f deg\n", in_pS->fCOG_deg);
    str += QString::asprintf("Heading          : %u\n", in_pS->usHeading);
    str += QString::asprintf("Timestamp        : %u\n", in_pS->ucTimestamp);
    str += QString::asprintf("Maneuver         : %u\n", in_pS->ucManeuver);
    str += QString::asprintf("RAIM             : %u\n", in_pS->ucRAIM);
    str += QString::asprintf("Sync State       : %u\n", in_pS->ucSync_State);
    str += QString::asprintf("Slot Timeout     : %u\n", in_pS->ucSlot_Timeout);
    str += QString::asprintf("Sub Message      : %u\n", in_pS->us_Sub_message);
    str += QString::asprintf("Spare            : %u\n", in_pS->ucOut_Spare);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        str += "VALIDATION: FAILED\n";

        if (in_pS->uiErrorMask & AIS_ERR_MSG_ID)     str += "  - Invalid MESSAGE ID\n";
        if (in_pS->uiErrorMask & AIS_ERR_REPEAT)     str += "  - Invalid REPEAT INDICATOR\n";
        if (in_pS->uiErrorMask & AIS_ERR_MMSI)       str += "  - Invalid MMSI\n";
        if (in_pS->uiErrorMask & AIS_ERR_NAV_STATUS) str += "  - Invalid NAVIGATION STATUS\n";
        if (in_pS->uiErrorMask & AIS_ERR_ROT)        str += "  - Invalid RATE OF TURN\n";
        if (in_pS->uiErrorMask & AIS_ERR_SOG)        str += "  - Invalid SPEED OVER GROUND\n";
        if (in_pS->uiErrorMask & AIS_ERR_POS_ACC)    str += "  - Invalid POSITION ACCURACY\n";
        if (in_pS->uiErrorMask & AIS_ERR_LON)        str += "  - Invalid LONGITUDE\n";
        if (in_pS->uiErrorMask & AIS_ERR_LAT)        str += "  - Invalid LATITUDE\n";
        if (in_pS->uiErrorMask & AIS_ERR_COG)        str += "  - Invalid COURSE OVER GROUND\n";
        if (in_pS->uiErrorMask & AIS_ERR_HEADING)    str += "  - Invalid TRUE HEADING\n";
        if (in_pS->uiErrorMask & AIS_ERR_TIMESTAMP)  str += "  - Invalid TIMESTAMP\n";
        if (in_pS->uiErrorMask & AIS_ERR_MANEUVER)   str += "  - Invalid MANEUVER INDICATOR\n";
        if (in_pS->uiErrorMask & AIS_ERR_RAIM)       str += "  - Invalid RAIM FLAG\n";
        if (in_pS->uiErrorMask & AIS_ERR_SYNC_STATE) str += "  - Invalid SYNC STATE\n";
        if (in_pS->uiErrorMask & AIS_ERR_SLOT_TIMEOUT) str += "  - Invalid SLOT TIMEOUT\n";
        if (in_pS->uiErrorMask & AIS_ERR_SUB_MESSAGE)  str += "  - Invalid SUB MESSAGE\n";
        if (in_pS->uiErrorMask & AIS_ERR_SPARE)        str += "  - Spare bits not ZERO\n";
    }
    else
    {
        str += "VALIDATION: OK\n";
    }

    return str;
}

void MainWindow::connect_signals()
{
    connect(mObj_SerialCommunication->m_serialPort, &QSerialPort::readyRead, mObj_SerialCommunication,&SerialCore::slot_read_data);
    connect(mObj_SerialCommunication, &SerialCore::signal_parse_rx_buffer, this,&MainWindow::slot_parse_rx_buffer);
}

void MainWindow::on_pushButton_uart_open_clicked()
{
    int iRet = mObj_SerialCommunication->connect_serial(ui->comboBox->currentText());
    if(iRet == 0)
    {
        enable_disable_uart_connection_button(true);
        ui->comboBox->setCurrentIndex(ui->comboBox->findText(ui->comboBox->currentText()));
        m_NewDeviceTimer->stop();
    }
    else if(iRet == -1)
    {
        QMessageBox::critical(this, "Error",
                              "The selected COM port is already open in another program.\n"
                              "Please close that program and try again.");
        m_NewDeviceTimer->start(5000);
    }
    else if(iRet == -2)
    {
        QMessageBox::critical(this, "Error", "Failed to open port");
        m_NewDeviceTimer->start(5000);
    }
}


void MainWindow::on_pushButton_uart_close_clicked()
{
    bool sts =  mObj_SerialCommunication->disconnet_serial();
    enable_disable_uart_connection_button(!sts);
}

void MainWindow::slot_parse_rx_buffer(const QByteArray &rx_byteArr)
{
    ui->plainTextEdit->insertPlainText("RAW : " + rx_byteArr.trimmed());
    ui->plainTextEdit->ensureCursorVisible();

    //decoding AIS
    int iMsgid = 0;
    U_AIS_MSG_FULL U_Msg;

    // .constData() converts Qt's QByteArray to the 'const char*' expected by C
    int status = ais_decode(rx_byteArr.constData(), &iMsgid, &U_Msg);

    if(status == AIS_SUCCESS)
    {
        QString decodedInfo;
        qDebug() << "MSG ID:" << iMsgid;
        if (iMsgid >= 1 && iMsgid <= 3) {
            decodedInfo = format_ais_type123(&U_Msg.mSt123);
        }

        else if(iMsgid == 18)
        {
            decodedInfo = format_ais_type18(&U_Msg.mSt18);
        }
        else if (iMsgid == 4) {
            // You can create format_ais_type4() later
            decodedInfo = "Received Type 4 (Base Station) - Formatting not implemented yet.";
        }
        else if (iMsgid == 5) {
            // You can create format_ais_type5() later
            decodedInfo = QString("Received Type 5 (Ship Name): %1").arg(U_Msg.mSt5.arr_cShip_name);
        }
        else {
            decodedInfo = QString("Received AIS Message Type: %1").arg(iMsgid);
        }
        // 4. Print the formatted string to the GUI
        ui->plainTextEdit_decoded->appendPlainText(decodedInfo);

        // Scroll to the bottom so you see the newest data
        ui->plainTextEdit_decoded->ensureCursorVisible();
    }
    else if (status == AIS_ERR_INCOMPLETE)
    {
        ui->plainTextEdit_decoded->appendPlainText("Waiting for second fragment...");
    }
    else
    {
        ui->plainTextEdit_decoded->appendPlainText("AIS Decode Error!");
    }




    //        if (iMsgid >= 1 && iMsgid <= 3)
    //            ais_print_type123(&U_Msg.mSt123);

    //        else if (iMsgid == 4)
    //            ais_print_type4(&U_Msg.mSt4);

    //        else if (iMsgid == 5)
    //            ais_print_type5(&U_Msg.mSt5);

    //        else if (iMsgid == 6)
    //            ais_print_type6(&U_Msg.mSt6);

    //        else if (iMsgid == 7)
    //            ais_print_type7(&U_Msg.mSt7);

    //        else if (iMsgid == 8)
    //            ais_print_type8(&U_Msg.mSt8);

    //        else if (iMsgid == 9)
    //            ais_print_type9(&U_Msg.mSt9);

    //        else if (iMsgid == 10)
    //            ais_print_type10(&U_Msg.mSt10);

    //        else if (iMsgid == 11)
    //            ais_print_type11(&U_Msg.mSt11);

    //        else if (iMsgid == 12) ais_print_type12(&U_Msg.mSt12);
    //        else if (iMsgid == 13) ais_print_type13(&U_Msg.mSt13);
    //        else if (iMsgid == 14) ais_print_type14(&U_Msg.mSt14);
    //        else if (iMsgid == 15) ais_print_type15(&U_Msg.mSt15);
    //        else if (iMsgid == 16) ais_print_type16(&U_Msg.mSt16);
    //        else if (iMsgid == 17) ais_print_type17(&U_Msg.mSt17);
    //        else if (iMsgid == 18) ais_print_type18(&U_Msg.mSt18);
    //        else if (iMsgid == 19) ais_print_type19(&U_Msg.mSt19);
    //        else if (iMsgid == 20) ais_print_type20(&U_Msg.mSt20);
    //        else if (iMsgid == 21) ais_print_type21(&U_Msg.mSt21);
    //        else if (iMsgid == 22) ais_print_type22(&U_Msg.mSt22);
    //        else if (iMsgid == 23) ais_print_type23(&U_Msg.mSt23);
    //        else if (iMsgid == 24) ais_print_type24(&U_Msg.mSt24);
    //        else if (iMsgid == 25) ais_print_type25(&U_Msg.mSt25);
    //        else if (iMsgid == 26) ais_print_type26(&U_Msg.mSt26);
    //        else if (iMsgid == 27) ais_print_type27(&U_Msg.mSt27);
}


