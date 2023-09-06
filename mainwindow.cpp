#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include<QString>
#include<QMessageBox>
#include<QScrollBar>
#include<QDateTime>
#include<string>
#include<QFileDialog>
#include <QTimer>
#include <QLCDNumber>
#include <regex>
//#include <QTcpSocket>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    m_serial_bund=0;
    m_totla_send=0;
    connect(ui->com_serial_checked,SIGNAL(currentIndexChanged(int)),this,SLOT(set_serial_check(int)));
    connect(ui->com_serial_databits,SIGNAL(currentIndexChanged(int)),this,SLOT(set_serial_databits(int)));
    connect(ui->com_serial_stop,SIGNAL(currentIndexChanged(int)),this,SLOT(set_serial_stopbits(int)));
    connect(ui->com_serial_port,SIGNAL(currentTextChanged(QString)),this,SLOT(set_serial_port(QString)));
    connect(ui->com_serial_rate,SIGNAL(currentIndexChanged(int)),this,SLOT(set_serial_bund(int)));
    connect(&m_serialport,SIGNAL(readyRead()),this,SLOT(read_data()));
    connect(ui->checkBox_time,SIGNAL(stateChanged(int)),this,SLOT(time_sender(int)));
    connect(&m_sock,SIGNAL(connected()),this,SLOT(tcp_connected()));
    connect(&m_sock,SIGNAL(disconnected()),this,SLOT(tcp_disconnected()));
    connect(&m_sock,SIGNAL(readyRead()),this,SLOT(tcp_read()));
    connect(timer,SIGNAL(timeout()),this,SLOT(time_updada()));
    timer->start(1000);

    m_serial_send=0;
    compont_init();
}

MainWindow::~MainWindow()
{
    delete timer;
    delete ui;
}

int MainWindow::compont_init()
{
    //获取可用串口
    QList<QSerialPortInfo> serialinfo;
    serialinfo=QSerialPortInfo::availablePorts();
    for(auto i:serialinfo)
    {
       // qDebug()<<i.portName();
        ui->com_serial_port->addItem(i.portName());
    }
    ui->btn_serial_switch->setIcon(QIcon(":/icon/icons/lights.png"));
    QString style="background-color:#000000;color:#00FF00";
    ui->label_recv_data->setStyleSheet(style);
    //初始化波特率
    ui->com_serial_rate->addItem("9600");
    ui->com_serial_rate->addItem("38400");
    ui->com_serial_rate->addItem("57600");
    ui->com_serial_rate->addItem("115200");

    //初始化停止位
    ui->com_serial_stop->addItem("1");
    ui->com_serial_stop->addItem("1.5");
    ui->com_serial_stop->addItem("2");

    //初始化数据位
    ui->com_serial_databits->addItem("5");
    ui->com_serial_databits->addItem("6");
    ui->com_serial_databits->addItem("7");
    ui->com_serial_databits->addItem("8");
    ui->com_serial_databits->setCurrentIndex(3);

    //初始化奇偶校验
    ui->com_serial_checked->addItem("无校验");
    ui->com_serial_checked->addItem("奇校验");
    ui->com_serial_checked->addItem("偶校验");

    ui->btn_serial_switch->setText("打开串口");

    ui->radioButton_asc->setChecked(true);
    ui->radiobtn_recv_asc->setChecked(true);

    ui->lineEdit_ip->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px");
    ui->lineEdit_port->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px");

    ui->btn_tcp_conn->setText("连接");
    ui->btn_tcp_conn->setStyleSheet("background-color:#BEBEBE; color:#A52A2A");

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setStyleSheet("background-color:#BCD2EE");

    ui->groupBox_send->setStyleSheet("background-color:#D1EEEE");
    ui->groupBox_recv->setStyleSheet("background-color:#E8E8E8");

    MainWindow::setStyleSheet("background-color:#E0FFFF");

    return 0;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(on_btn_serial_send_clicked()==-1)
    {
        killTimer(m_time);
        ui->checkBox_time->setChecked(false);
    }

}


void MainWindow::on_btn_serial_switch_clicked()
{
    if(m_serialport.isOpen())
    {
        m_serialport.close();
        ui->btn_serial_switch->setText("打开串口");
        ui->btn_serial_switch->setIcon(QIcon(":/icon/icons/lights.png"));
        qDebug()<<"close";
    }
    else
    {
        m_serialport.setFlowControl(QSerialPort::NoFlowControl);
        m_serialport.open(QIODeviceBase::ReadWrite);
        ui->btn_serial_switch->setText("关闭串口");
        ui->btn_serial_switch->setIcon(QIcon(":/icon/icons/lighting.png"));
        qDebug()<<"open";
    }


}

int MainWindow::set_serial_bund(int index)
{
    switch(index)
    {
    case 0: m_serial_bund =9600;break;
    case 1: m_serial_bund =38400;break;
    case 2: m_serial_bund =57600;break;
    case 3: m_serial_bund =115200;break;
    }
    qDebug()<<" bund set to"<<m_serial_bund;
    return m_serialport.setBaudRate(m_serial_bund);
}

int MainWindow::set_serial_databits(int index)
{
    switch(index)
    {
    case 0: m_serial_databits =QSerialPort::Data5;break;
    case 1: m_serial_databits =QSerialPort::Data6;break;
    case 2: m_serial_databits =QSerialPort::Data7;break;
    case 3: m_serial_databits =QSerialPort::Data8;break;
    }
    qDebug()<<" databits set to"<<m_serial_databits;
    return m_serialport.setDataBits(m_serial_databits);
}

int MainWindow::set_serial_stopbits(int index)
{
    switch(index)
    {
    case 0: m_serial_stopbit =QSerialPort::OneStop;break;
    case 1: m_serial_stopbit =QSerialPort::OneAndHalfStop;break;
    case 2: m_serial_stopbit =QSerialPort::TwoStop;break;
    }
    qDebug()<<" spotbits set to"<<m_serial_stopbit;
    return m_serialport.setStopBits(m_serial_stopbit);
}

int MainWindow::set_serial_port(QString text)
{
    m_serialport.setPortName(text);
    qDebug()<<" port set to"<<text;
    return 0;
}

int MainWindow::set_serial_check(int index)
{
    switch(index)
    {
    case 0: m_serial_check =QSerialPort::NoParity;break;
    case 1: m_serial_check =QSerialPort::OddParity;break;
    case 2: m_serial_check =QSerialPort::EvenParity;break;
    }
    qDebug()<<" check set to"<<m_serial_check;
    return m_serialport.setParity(m_serial_check);
}

int MainWindow::read_data()
{

    QByteArray arr=m_serialport.readAll();
    qDebug()<<arr.data();
    QString str=ui->label_recv_data->text();
    QString str1;
    qDebug()<<"recv"<<str;
    if(ui->radiobtn_recv_hex->isChecked())
    {
        //hex to str
        Hex_to_str(str1,arr);
    }
    else
    {
        str1=arr.data();
    }
    if(ui->checkBox_stamp->isChecked())
    {
        QDateTime current=QDateTime::currentDateTime();
        QString datettime=current.toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString newstr;
        newstr="[ "+datettime+" ] "+str1;
        str+=newstr;
    }
    else
    {
        str+=str1;
    }
    ui->label_recv_data->setText(str);
    QScrollBar* scroll_bar=ui->scrollArea->verticalScrollBar();
    if(scroll_bar == nullptr)
    {
        qDebug()<<"scroll_bar is null";
    }
    else
    {
        scroll_bar->setValue(scroll_bar->maximum());
        qDebug()<<"scroll_bar value is"<<scroll_bar->value();
    }
    return arr.length();
}

int MainWindow::Str_to_hex(const QString &str,QByteArray &arr)
{

    arr.resize(str.length());
    int j=0;
    for(int i=0;i<str.length()-1;)
    {
        char ch1=str.at(i).toLatin1(),ch2=str.at(i+1).toLatin1();
        if((ch1>='0'&&ch1<='9')||(ch1>='a'&&ch1<='f')||(ch1>='A'&&ch1<='F'))
        {
            if((ch2>='0'&&ch2<='9')||(ch2>='a'&&ch2<='f')||(ch2>='A'&&ch2<='F'))
            {
                if(ch1>='0'&&ch1<='9') ch1-='0';
                if(ch1>='a'&&ch1<='f') ch1=ch1-'a'+10;
                if(ch1>='A'&&ch1<='F')  ch1=ch1-'A'+10;
                if(ch2>='0'&&ch2<='9') ch2-='0';
                if(ch2>='a'&&ch2<='f')  ch2=ch2-'a'+10;
                if(ch2>='A'&&ch2<='F')  ch2=ch2-'A'+10;

                arr[j++]=ch1*16+ch2;
                i+=2;
            }
            else
                return -1;
        }
        else if(ch1!=' ')
        {
            return -1;
        }
        else
        {
            i++;
        }
    }
    arr.resize(j);
    return 0;
}

int MainWindow::Hex_to_str(QString &str, const QByteArray &arr)
{
    int count=0;
    QString newbyte;
    foreach (int byte, arr)
    {
        if((unsigned char)byte=='\r' || (unsigned char)byte=='\n')
            {
            if(count ==1)
                continue;
            else
            {
                str+="\r\n";
                count=1;
            }

            }

        else
        {
            newbyte=QString::number((unsigned char)byte,16).toUpper();
            if(newbyte.length()<2)
            {
                newbyte="0"+newbyte;
            }
            str+=newbyte;
            str+=" ";
        }

    }
    return 0;
}

bool MainWindow::isIP(QString ip)
{

    if (ip.length()<0) {
        return false;
           }
    std::string str=ip.toStdString();
    std::regex pattern("((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)");
           std::smatch res;
    if(std::regex_match(str, res, pattern)){
        return true;
    }
    return false;

}

void MainWindow::time_sender(int time)
{
    if(ui->checkBox_time->isChecked())
    {
        int time=ui->spinBox_time->value();
        m_time=startTimer(time);
        ui->spinBox_time->setEnabled(false);
        ui->btn_clear_textedite->setEnabled(false);
    }
    else
    {
        killTimer(m_time);
        ui->spinBox_time->setEnabled(true);
        ui->btn_clear_textedite->setEnabled(true);
    }
}


int MainWindow::on_btn_serial_send_clicked()
{
    int send_num=0;
    if(m_serialport.isOpen() || m_sock.isOpen())
    {
        QString str=ui->textEdit->toPlainText().toLatin1();
        if(str.length()<=0)
        {
            QMessageBox::warning(this,"提示","请输入要发送的数据！");
            return -1;
        }
        else
        {
            //判断十六进制还是ASCII
            if(ui->radioButton_hex->isChecked())
            {
                QByteArray arr;
                if(Str_to_hex(str,arr)==-1)
                {
                    QMessageBox::warning(this,"错误","请检查数据位");
                    return -1;
                }
                if(ui->checkBox_newline->isChecked())
                    arr.append("\r\n");
                if(m_serialport.isOpen())
                {
                    send_num=m_serialport.write(arr);
                    m_totla_send+=send_num;
                }

                if(m_sock.isOpen())
                {
                    send_num=m_sock.write(arr);
                    m_totla_send+=send_num;
                }

                qDebug()<<"send_num"<<send_num;
            }
            else
            {
                if(ui->checkBox_newline->isChecked())
                    str+="\r\n";
                if(m_serialport.isOpen())
                {
                     send_num=m_serialport.write(str.toLatin1());
                     m_totla_send+=send_num;
                }

                if(m_sock.isOpen())
                {
                     send_num=m_sock.write(str.toLatin1());
                      m_totla_send+=send_num;
                }

            }

            qDebug()<<ui->textEdit->toPlainText();
            str=str.number(send_num);
            ui->label_signal_count->setText(str);

            str=str.number(m_totla_send);
            qDebug()<<"total"<<str;
            ui->label_total_send_count->setText(str);
        }

    }
    else
    {
        QMessageBox::warning(this,"错误","请先打开串口！");
        return -1;
    }
    return 0;
}





void MainWindow::on_btn_clear_textedite_clicked()
{
    ui->textEdit->clear();
}


void MainWindow::on_pushButton_clicked()
{
    ui->label_recv_data->clear();
}


void MainWindow::on_pushButton_2_clicked()
{
    QFileDialog::saveFileContent(ui->label_recv_data->text().toLatin1(),"recv.txt");
}


void MainWindow::on_btn_tcp_conn_clicked()
{
    if(m_sock.isOpen())
    {
        m_sock.close();
    }
    else
    {

        QString ip=ui->lineEdit_ip->text();
        QString port_str=ui->lineEdit_port->text();

        if(port_str.length()==0 ||!isIP(ip))
        {
            QMessageBox::warning(this,"错误","请输入有效ip或端口");
            return;
        }
        unsigned short port=port_str.toUShort();
        qDebug()<<"ip"<<ip<<" "<<"port"<<port;
        m_sock.connectToHost(ip,port);
    }

}

void MainWindow::tcp_connected()
{
    ui->btn_tcp_conn->setText("断开连接");
    ui->btn_tcp_conn->setStyleSheet("background-color:#00CD00; color:#EEEE00");
    qDebug()<<"tcp connected";
}

void MainWindow::tcp_disconnected()
{
    ui->btn_tcp_conn->setText("连接");
    ui->btn_tcp_conn->setStyleSheet("background-color:#BEBEBE; color:#A52A2A");
    qDebug()<<"tcp disconnected";
}

int MainWindow::tcp_read()
{
    QByteArray arr=m_sock.readAll();
    qDebug()<<arr.data();
    QString str=ui->label_recv_data->text();
    QString str1;
    qDebug()<<"recv"<<str;
    if(ui->radiobtn_recv_hex->isChecked())
    {
        //hex to str
        Hex_to_str(str1,arr);
    }
    else
    {
        str1=arr.data();
    }
    if(ui->checkBox_stamp->isChecked())
    {
        QDateTime current=QDateTime::currentDateTime();
        QString datettime=current.toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString newstr;
        newstr="[ "+datettime+" ] "+str1;
        str+=newstr;
    }
    else
    {
        str+=str1;
    }
    ui->label_recv_data->setText(str);
    QScrollBar* scroll_bar=ui->scrollArea->verticalScrollBar();
    if(scroll_bar == nullptr)
    {
        qDebug()<<"scroll_bar is null";
    }
    else
    {
        scroll_bar->setValue(scroll_bar->maximum());
        qDebug()<<"scroll_bar value is"<<scroll_bar->value();
    }
    return arr.length();
}

void MainWindow::time_updada()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str1 = time.toString("yyyy-MM-dd hh:mm:sss");

}


void MainWindow::on_btn_exit_clicked()
{
    exit(1);
}

