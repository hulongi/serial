#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSerialPort>
#include<qserialport.h>
#include <QTcpSocket>
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
//QT += serialport;


class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int compont_init();
    void timerEvent(QTimerEvent *event);
    int Str_to_hex(const QString &str,QByteArray &arr);
    int Hex_to_str(QString &str, const QByteArray &arr);
    bool isIP(QString ip);


private slots:
    void on_btn_serial_switch_clicked();
    int set_serial_bund(int index);
    int set_serial_databits(int index);
    int set_serial_stopbits(int index);
    int set_serial_port(QString text);
    int set_serial_check(int index);
    int read_data();

    void time_sender(int time);
    int on_btn_serial_send_clicked();



    void on_btn_clear_textedite_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_btn_tcp_conn_clicked();

    //TCP
    void tcp_connected();
    void tcp_disconnected();
    int tcp_read();

    //time
    void time_updada();

    void on_btn_exit_clicked();

private:
    Ui::MainWindow *ui;
    int m_serial_bund;
    int m_serial_send;
    int m_totla_send;
    int m_time;
    QTimer *timer;
    QTcpSocket m_sock;
    QSerialPort::StopBits m_serial_stopbit;
    QSerialPort::Parity m_serial_check;
    QSerialPort::DataBits m_serial_databits;
    QSerialPort m_serialport;
};
#endif // MAINWINDOW_H
