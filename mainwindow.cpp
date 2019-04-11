#include "mainwindow.h"
#include "ui_mainwindow.h"
#include    <QtNetwork>
#include <synchapi.h>
unsigned long  m_devtype=4;//USBCAN2类型号
unsigned long m_devind=0;
unsigned long m_cannum=0;
int m_connect=0;
uint16_t ucLifeSignal = 0;
QString MainWindow::getLocalIP()
{
    QString hostName=QHostInfo::localHostName();//本地主机名
    QHostInfo   hostInfo=QHostInfo::fromName(hostName);
    QString   localIP="";

    QList<QHostAddress> addList=hostInfo.addresses();//

    if (!addList.isEmpty())
    for (int i=0;i<addList.count();i++)
    {
        QHostAddress aHost=addList.at(i);
        if (QAbstractSocket::IPv4Protocol==aHost.protocol())
        {
            localIP=aHost.toString();
            break;
        }
    }
    return localIP;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    LabSocketState=new QLabel("Socket状态：");//
    LabSocketState->setMinimumWidth(200);
    ui->statusBar->addWidget(LabSocketState);

    QString localIP=getLocalIP();//本地主机名
    this->setWindowTitle(this->windowTitle()+"----本机IP："+localIP);
    ui->UcomboIP->addItem(localIP);

    MudpSocket=new QUdpSocket(this);//用于与连接的客户端通讯的QTcpSocket
//Multicast路由层次，1表示只在同一局域网内
    //组播TTL: 生存时间，每跨1个路由会减1，多播无法跨过大多数路由所以为1
    //默认值是1，表示数据包只能在本地的子网中传送。
    MudpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption,1);
    connect(MudpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    onSocketStateChange(MudpSocket->state());
    connect(MudpSocket,SIGNAL(readyRead()),this,SLOT(onMSocketReadyRead()));


    UudpSocket=new QUdpSocket(this);
    connect(UudpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    onSocketStateChange(UudpSocket->state());
    connect(UudpSocket,SIGNAL(readyRead()),this,SLOT(onUSocketReadyRead()));


    MudpSendSocket=new QUdpSocket(this);
    MudpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption,1);

    ui->stoptest->setEnabled(false);

    m_Timer1 = new QTimer(this);
    connect(m_Timer1,SIGNAL(timeout()),this,SLOT(onTimerOut()));  //绑定定时器的信号与槽
}

MainWindow::~MainWindow()
{
    MudpSocket->abort();
    delete MudpSocket;
    delete ui;
}

void MainWindow::onSocketStateChange(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState:
        LabSocketState->setText("scoket状态：UnconnectedState");
        break;
    case QAbstractSocket::HostLookupState:
        LabSocketState->setText("scoket状态：HostLookupState");
        break;
    case QAbstractSocket::ConnectingState:
        LabSocketState->setText("scoket状态：ConnectingState");
        break;

    case QAbstractSocket::ConnectedState:
        LabSocketState->setText("scoket状态：ConnectedState");
        break;

    case QAbstractSocket::BoundState:
        LabSocketState->setText("scoket状态：BoundState");
        break;

    case QAbstractSocket::ClosingState:
        LabSocketState->setText("scoket状态：ClosingState");
        break;

    case QAbstractSocket::ListeningState:
        LabSocketState->setText("scoket状态：ListeningState");
    }
}
    uint8_t buttontime = 0;
void MainWindow::DispData(PROCESS_DATA_STRUCT *ptProcessData,QHostAddress targetAddr)
{

    if (buttontime ==0)
    {
        //ui->display->setText("开始显示");
        return;
    }
    //显示包头
    QString ucStr;
    char cStr[] = {"00000000000"};
    uint8_t ucData = 0;
    ucData = ptProcessData->Package_header.Frame_header;
    ui->Frame_header0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    ucData = ptProcessData->Package_header.Frame_header >> 8;
    ui->Frame_header1->setText(ucStr.setNum(ucData,16));

    //显示帧长度
    ucData = ptProcessData->Package_header.Frame_length;
    ui->Frame_length0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
    ucData = ptProcessData->Package_header.Frame_length >> 8;
    ui->Frame_length1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
    //显示厂家代码
    ucData = ptProcessData->Package_header.Vender_code;  
    ui->Vender_code->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
    //显示设备代码
    ucData = ptProcessData->Package_header.Device_code;
    ui->Device_code->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
    //显示生命信号
    ucData = ptProcessData->Package_header.Life_signal;   
    ui->Life_signal0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    ucData = ptProcessData->Package_header.Life_signal >> 8;
    ui->Life_signal1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    //显示目标板地址
    ucData = ptProcessData->Package_header.Target_address;
    ui->Target_address0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    ucData = ptProcessData->Package_header.Target_address >> 8;
    ui->Target_address1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    //显示重发标志
    ucData = ptProcessData->Package_header.Repeat_flag;
    ui->Repeat_flag->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    //显示目标板应答标志
    ucData = ptProcessData->Package_header.Reply_flag;
    ui->Reply_flag->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
    //显示总包数
    ucData = ptProcessData->Package_header.Package_num;
    ui->Package_num->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    //显示软件版本
    ucData = ptProcessData->Software_version;
    ui->Software_version0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

    ucData = ptProcessData->Software_version >> 8;
    ui->Software_version1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));



    if (ptProcessData->Package_header.Frame_header == 0x10AA)
    {
       ui->TempSensorStatus->clear();
       for(uint8_t j = 26;j<66;j++)
       {
           ucData = ptProcessData->pro_data[j];
           if ((j == 36)||(j == 46)||(j == 56))
           {
               ui->TempSensorStatus->appendPlainText("  ");
           }
           ui->TempSensorStatus->appendPlainText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

       }
       ui->TempData->clear();
       for(uint8_t j = 66;j<106;j++)
       {
           ucData = ptProcessData->pro_data[j];
           if ((j == 76)||(j == 86)||(j == 96))
           {
               ui->TempData->appendPlainText("  ");
           }
           ui->TempData->appendPlainText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

       }
       ui->TempSensorDetail->clear();
       for(uint8_t j = 106;j<146;j++)
       {
           ucData = ptProcessData->pro_data[j];
           if ((j == 116)||(j == 126)||(j == 136))
           {
               ui->TempSensorDetail->appendPlainText("  ");
           }
           ui->TempSensorDetail->appendPlainText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

       }



        //控制板通信和输入电源状态
        ucData = ptProcessData->pro_data[146];
        ui->Talk_state->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示北京时间
        ucData = ptProcessData->pro_data[147];

        ui->Time_net0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        ucData = ptProcessData->pro_data[148];

        ui->Time_net1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[149];

        ui->Time_net2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[150];

        ui->Time_net3->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[151];

        ui->Time_net4->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[152];

        ui->Time_net5->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示车型
        ucData = ptProcessData->pro_data[153];

        ui->Train_type->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示车厢号
        ucData = ptProcessData->pro_data[154];

        ui->Car_number->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示动拖车
        ucData = ptProcessData->pro_data[155];

        ui->Motor_car->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //数字量输入
        ucData = 0;

            ui->Input_digit->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //数字量输出
        ucData = 0;

            ui->Output_dummy->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //编组编号
        ucData = ptProcessData->pro_data[168];
       ui->Marshalling_num1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[169];
        ui->Marshalling_num2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示速度
        ucData = ptProcessData->pro_data[170];

        ui->Speed_train0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[171];

        ui->Speed_train1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示温度
        ucData = ptProcessData->pro_data[172];

        ui->Temperature_outer->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示控制模式
        ucData = ptProcessData->pro_data[173];

        ui->Control_mode->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示GPS状态
        ucData = ptProcessData->pro_data[174];

        ui->Valid_bit->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示空气弹簧压力
        ucData = ptProcessData->Air_pressure[0];

        ui->Air_pressure1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Air_pressure[1];

        ui->Air_pressure2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示经度
        ucData = ptProcessData->pro_data[175];

        ui->Gps_data0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[176];

        ui->Gps_data1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[177];

        ui->Gps_data2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[178];

        ui->Gps_data3->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示经纬度方向

        ucData = ptProcessData->pro_data[179];

        ui->Gps_data4->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[180];

        ui->Gps_data5->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示纬度
        ucData = ptProcessData->pro_data[181];

        ui->Gps_data6->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[182];

        ui->Gps_data7->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[183];
        ui->Gps_data8->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[184];
        ui->Gps_data9->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //校验和
        ucData = ptProcessData->pro_data[254];
        ui->Check_sum0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->pro_data[255];
        ui->Check_sum1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        uint16_t TempData = 0;
        TempData = ptProcessData->Package_header.Obligate[0];
        TempData = TempData << 8 | ptProcessData->Package_header.Obligate[1];

        QString uString;
        uString = uString.setNum(TempData,10);
        ui->ReceivePacket->setText(uString);

        TempData = ptProcessData->Package_header.Obligate[2];
        TempData = TempData << 8 | ptProcessData->Package_header.Obligate[3];

        uString = uString.setNum(TempData,10);
        ui->RightPacket->setText(uString);
    }
    else
    {
        //加速度传感器和平稳采集卡故障
         ucData = ptProcessData->Sensor_fault >> 8;
         ui->Sensor_fault->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

         //1位横向指标
         ucData = ptProcessData->x1_value;
         ui->x1_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //2位横向指标
         ucData = ptProcessData->x2_value;
         ui->x2_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //1位垂向指标
         ucData = ptProcessData->z1_value;
         ui->z1_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //2位垂向指标
         ucData = ptProcessData->z2_value;
         ui->z2_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //1位纵向指标
         ucData = ptProcessData->y1_value;
         ui->y1_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //2位纵向指标
         ucData = ptProcessData->y2_value;
         ui->y2_value->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //总平稳报警状态
         ucData = ptProcessData->AllStatus;

             ui->AllStatus->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
         //1和2位平稳报警状态
         ucData = ptProcessData->AlarmStatus;
         ui->AlarmStatus->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //控制板通信和输入电源状态
        ucData = ptProcessData->Talk_state;
        ui->Talk_state->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示北京时间
        ucData = ptProcessData->Time_net.year;

        ui->Time_net0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        ucData = ptProcessData->Time_net.month;

        ui->Time_net1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Time_net.day;

        ui->Time_net2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Time_net.hour;

        ui->Time_net3->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Time_net.minute;

        ui->Time_net4->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Time_net.second;

        ui->Time_net5->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示车型
        ucData = ptProcessData->Train_type;

        ui->Train_type->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示车厢号
        ucData = ptProcessData->Car_number;

        ui->Car_number->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示动拖车
        ucData = ptProcessData->Motor_car;

        ui->Motor_car->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //数字量输入
        ucData = ptProcessData->Input_digit;

            ui->Input_digit->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //数字量输出
        ucData = ptProcessData->Output_dummy;

            ui->Output_dummy->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //编组编号
        ucData = ptProcessData->Marshalling_num1;
       ui->Marshalling_num1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Marshalling_num2;
        ui->Marshalling_num2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示速度
        ucData = ptProcessData->Speed_train;

        ui->Speed_train0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Speed_train >> 8;

        ui->Speed_train1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示温度
        ucData = ptProcessData->Temperature_outer;

        ui->Temperature_outer->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示控制模式
        ucData = ptProcessData->Control_mode;

        ui->Control_mode->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        //显示GPS状态
        ucData = ptProcessData->Valid_bit;

        ui->Valid_bit->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示空气弹簧压力
        ucData = ptProcessData->Air_pressure[0];

        ui->Air_pressure1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Air_pressure[1];

        ui->Air_pressure2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示经度
        ucData = ptProcessData->Gps_data[0];

        ui->Gps_data0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[1];

        ui->Gps_data1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[2];

        ui->Gps_data2->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[3];

        ui->Gps_data3->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示经纬度方向

        ucData = ptProcessData->Gps_data[4];

        ui->Gps_data4->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[5];

        ui->Gps_data5->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //显示纬度
        ucData = ptProcessData->Gps_data[6];

        ui->Gps_data6->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[7];

        ui->Gps_data7->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[8];
        ui->Gps_data8->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Gps_data[9];
        ui->Gps_data9->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        //校验和
        ucData = ptProcessData->Check_sum;
        ui->Check_sum0->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));
        ucData = ptProcessData->Check_sum >> 8;
        ui->Check_sum1->setText(QString("%1").arg(ucData,2,16,QLatin1Char('0')));

        uint16_t TempData = 0;
        TempData = ptProcessData->Package_header.Obligate[0];
        TempData = TempData << 8 | ptProcessData->Package_header.Obligate[1];

        QString uString;
        uString = uString.setNum(TempData,10);
        ui->ReceivePacket->setText(uString);

        TempData = ptProcessData->Package_header.Obligate[2];
        TempData = TempData << 8 | ptProcessData->Package_header.Obligate[3];

        uString = uString.setNum(TempData,10);
        ui->RightPacket->setText(uString);
    }




    //应答
    Reply_FRAME_STRUCT replay_data = {0};
    replay_data.Frame_header = ptProcessData->Package_header.Frame_header;
    replay_data.Frame_length = 0x1000;
    replay_data.Vender_code = ptProcessData->Package_header.Vender_code;
    replay_data.Device_code = ptProcessData->Package_header.Device_code;
    replay_data.Life_signal = ptProcessData->Package_header.Life_signal;
    replay_data.Right_flag = 0xA5;

    //QString     targetIP=ui->UcomboIP->currentText(); //目标IP
    //QHostAddress    targetAddr(targetIP);
    quint16     targetPort=ui->UspinPort->value();//目标port

    UudpSocket->writeDatagram((char *)&replay_data.Frame_header,16,targetAddr,targetPort); //发出数据报


}




void MainWindow::onMSocketReadyRead()
{//读取数据报
    while(MudpSocket->hasPendingDatagrams())
    {
        char receivedata[256] = {0};
        QHostAddress    peerAddr;
        quint16 peerPort;
        MudpSocket->readDatagram(receivedata,256,&peerAddr,&peerPort);
        DispData((PROCESS_DATA_STRUCT*)receivedata,peerAddr);
    }
}
void MainWindow::DispDataRaw(RAW_DATA_STRUCT *ptProcessData)
{




}
void MainWindow::onUSocketReadyRead()
{//读取数据报
    while(UudpSocket->hasPendingDatagrams())
    {
        char receivedata[1048] = {0};
        QHostAddress    peerAddr;
        quint16 peerPort;
        UudpSocket->readDatagram(receivedata,256,&peerAddr,&peerPort);
        DispDataRaw((RAW_DATA_STRUCT*)receivedata);
    }
}



char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}


QByteArray StringToHex(QString str)
{
    QByteArray senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
    return senddata;

}



void MainWindow::on_display_clicked()
{
    buttontime = ~buttontime;
    if (buttontime == 0)
    {
        ui->display->setText("开始显示");
    }
    else
    {
        ui->display->setText("停止显示");
    }

}
PROCESS_DATA_STRUCT ptProcessData = {0};
    uint16_t ucTempLife = 0;
void MainWindow::on_set_clicked()
{

    ui->set->setEnabled(false);
    ui->stoptest->setEnabled(true);
    QByteArray byteARR;
    uint8_t ucDatal = 0;
    uint8_t ucDatah = 0;
    QString ucStr;

    //显示包头
    ucStr = ui->Frame_header0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Frame_header1->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ptProcessData.Package_header.Frame_header = (ucDatah << 8)|ucDatal;

    //显示帧长度
    ucStr = ui->Frame_length0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Frame_length1->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ptProcessData.Package_header.Frame_length = (ucDatah << 8)|ucDatal;


    //显示厂家代码
    ucStr = ui->Vender_code->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Package_header.Vender_code = ucDatal;

    //显示设备代码
    ucStr = ui->Device_code->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Package_header.Device_code = ucDatal;

    //显示生命信号
    ucStr = ui->Life_signal0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Life_signal1->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ucTempLife =  (ucDatal << 8)|ucDatah;




    //显示目标板地址
    ucStr = ui->Target_address0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Target_address1->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ptProcessData.Package_header.Target_address = (ucDatah << 8)|ucDatal;

    //显示重发标志
    ucStr = ui->Repeat_flag->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Package_header.Repeat_flag = ucDatal;

    //显示目标板应答标志
    ucStr = ui->Reply_flag->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Package_header.Reply_flag = ucDatal;

    //显示总包数
    ucStr = ui->Package_num->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Package_header.Package_num = ucDatal;

    //显示软件版本
    ucStr = ui->Software_version0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Software_version0->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ptProcessData.Software_version = (ucDatah << 8)|ucDatal;

    if(ui->TestItem->currentIndex() == 0)
    {
        ptProcessData.Package_header.Obligate[4] = 0;
        ptProcessData.Package_header.Obligate[5] = 0;
    }
    else if(ui->TestItem->currentIndex() == 1)
    {
        ptProcessData.Package_header.Obligate[4] = 0;
        ptProcessData.Package_header.Obligate[5] = 0;
    }
    //可靠性测试数据
    else
    {
        ptProcessData.Package_header.Obligate[4] = 0x55;
        ptProcessData.Package_header.Obligate[5] = 0xAA;
    }




   //加速度传感器和平稳采集卡故障
    ucStr = ui->Sensor_fault->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Sensor_fault = ucDatal;

    //1位横向指标
    ucStr = ui->x1_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.x1_value = ucDatal;

    //2位横向指标
    ucStr = ui->x2_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.x2_value = ucDatal;

    //1位垂向指标
    ucStr = ui->z1_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.z1_value = ucDatal;

    //2位垂向指标
    ucStr = ui->z2_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.z2_value = ucDatal;

    //1位纵向指标
    ucStr = ui->y1_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.y1_value = ucDatal;

    //2位纵向指标
    ucStr = ui->y2_value->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.y2_value = ucDatal;

    //总平稳报警状态
    ucStr = ui->AllStatus->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.AllStatus = ucDatal;

    //1和2位平稳报警状态
    ucStr = ui->AlarmStatus->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.AlarmStatus = ucDatal;

    //控制板通信和输入电源状态
    ucStr = ui->Talk_state->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Talk_state = ucDatal;
    //显示北京时间
    ucStr = ui->Time_net0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.year = ucDatal;

    ucStr = ui->Time_net1->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.month = ucDatal;

    ucStr = ui->Time_net2->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.day = ucDatal;

    ucStr = ui->Time_net3->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.hour = ucDatal;

    ucStr = ui->Time_net4->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.minute = ucDatal;

    ucStr = ui->Time_net5->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Time_net.second = ucDatal;


    //显示车型
    ucStr = ui->Train_type->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Train_type = ucDatal;

    //显示车厢号
    ucStr = ui->Car_number->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Car_number = ucDatal;

    //显示动拖车
    ucStr = ui->Motor_car->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Motor_car = ucDatal;

    //数字量输入
    ucStr = ui->Input_digit->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Input_digit = ucDatal;

    //数字量输出
    ucStr = ui->Output_dummy->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Output_dummy = ucDatal;

    //编组编号
    ucStr = ui->Marshalling_num1->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Marshalling_num1 = ucDatal;

    ucStr = ui->Marshalling_num2->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Marshalling_num2 = ucDatal;

    //显示速度
    ucStr = ui->Speed_train0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ucStr = ui->Speed_train1->text();
    byteARR = StringToHex(ucStr);
    ucDatah = byteARR[0];
    ptProcessData.Speed_train = (ucDatah << 8)|ucDatal;


    //显示温度
    ucStr = ui->Temperature_outer->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Temperature_outer = ucDatal;

    //显示控制模式
    ucStr = ui->Control_mode->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Control_mode = ucDatal;

    //显示GPS状态

    ucStr = ui->Valid_bit->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Valid_bit = ucDatal;

    //显示空气弹簧压力
    ucStr = ui->Air_pressure1->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Air_pressure[0] = ucDatal;

    ucStr = ui->Air_pressure2->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Air_pressure[1] = ucDatal;



    //显示经度
    ucStr = ui->Gps_data0->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[0] = ucDatal;

    ucStr = ui->Gps_data1->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[1] = ucDatal;

    ucStr = ui->Gps_data2->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[2] = ucDatal;

    ucStr = ui->Gps_data3->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[3] = ucDatal;

    //显示经纬度方向
    ucStr = ui->Gps_data4->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[4] = ucDatal;

    ucStr = ui->Gps_data5->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[5] = ucDatal;


    //显示纬度
    ucStr = ui->Gps_data6->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[6] = ucDatal;

    ucStr = ui->Gps_data7->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[7] = ucDatal;

    ucStr = ui->Gps_data8->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[8] = ucDatal;

    ucStr = ui->Gps_data9->text();
    byteARR = StringToHex(ucStr);
    ucDatal = byteARR[0];
    ptProcessData.Gps_data[9] = ucDatal;



    m_Timer1->start(1000);

}
    uint16_t sendNum = 0;
void MainWindow::onTimerOut()
{

    QString ucStr;
    uint8_t ucDatal = 0;
    uint8_t ucDatah = 0;



    sendNum++;
     ucLifeSignal = ucTempLife >> 8;
     ucLifeSignal = ucLifeSignal|(ucTempLife << 8);
     ptProcessData.Package_header.Life_signal = ucLifeSignal;
     ucTempLife ++;

    uint16_t Addsum = 0;
    for(uint8_t i = 0;i<254;i++)
    {
        Addsum += ptProcessData.pro_data[i];
    }
    //校验和

    ucDatal = Addsum >> 8;
    ucDatah = Addsum;
    ptProcessData.Check_sum = (ucDatah << 8)|ucDatal;


    ucStr = ucStr.setNum(sendNum,10);
    ui->SendNum->setText(ucStr);

    quint16   groupPort=ui->MSendPort->value();//端口
    groupAddress=QHostAddress(ui->McomboSendIP->currentText());//多播组地址
    MudpSendSocket->writeDatagram((char *)&ptProcessData.Package_header.Frame_header,256,groupAddress,groupPort);
}

void MainWindow::on_TestItem_currentIndexChanged(int index)
{

    if (index == 0)
    {
        ClearAllData();
         ui->Frame_header0->setText("AA");
        ui->Frame_header1->setText("50");  //待确定
        ui->Frame_length0->setText("01");
       ui->Frame_length1->setText("00");  //待确定
        ui->Vender_code->setText("01");
        ui->Device_code->setText("11");
        ui->Target_address0->setText("01");
        ui->Target_address1->setText("FC");
        ui->Repeat_flag->setText("55");
        ui->Package_num->setText("01");
    }
    else if (index == 1)
    {
        ClearAllData();
         ui->Frame_header0->setText("AA");
        ui->Frame_header1->setText("50");  //待确定
        ui->Frame_length0->setText("01");
       ui->Frame_length1->setText("00");  //待确定
        ui->Vender_code->setText("02");
        ui->Device_code->setText("66");
        ui->Target_address0->setText("00");
        ui->Target_address1->setText("07");
        ui->Repeat_flag->setText("55");
        ui->Package_num->setText("07");
    }
    else
    {
        ClearAllData();
        ui->Frame_header0->setText("AA");  //待确定
        ui->Frame_header1->setText("50");  //待确定
        ui->Frame_length0->setText("01");
       ui->Frame_length1->setText("00");  //待确定
        ui->Vender_code->setText("01");
        ui->Device_code->setText("11");
        ui->Target_address0->setText("01");
        ui->Target_address1->setText("FC");
        ui->Repeat_flag->setText("55");
        ui->Package_num->setText("01");
    }
}

void MainWindow::ClearAllData(void)
{

    ui->Frame_header0->setText("00");
    ui->Frame_header1->setText("00");
    ui->Frame_length0->setText("00");
    ui->Frame_length1->setText("00");
    ui->Vender_code->setText("00");
    ui->Device_code->setText("00");
   ui->Life_signal0->setText("00");
    ui->Life_signal1->setText("00");
    ui->Target_address0->setText("00");
    ui->Target_address1->setText("00");
    ui->Repeat_flag->setText("00");
    ui->Reply_flag->setText("00");
    ui->Package_num->setText("00");
    ui->Software_version0->setText("00");
    ui->Software_version1->setText("00");
    ui->Sensor_fault->setText("00");
    ui->x1_value->setText("00");
    ui->x2_value->setText("00");
    ui->z1_value->setText("00");
    ui->z2_value->setText("00");
    ui->y1_value->setText("00");
    ui->y2_value->setText("00");
    ui->AllStatus->setText("00");
    ui->AlarmStatus->setText("00");
    ui->Talk_state->setText("00");
    ui->Time_net0->setText("00");
    ui->Time_net1->setText("00");
    ui->Time_net2->setText("00");
    ui->Time_net3->setText("00");
    ui->Time_net4->setText("00");
    ui->Time_net5->setText("00");
    ui->Train_type->setText("00");
    ui->Car_number->setText("00");
    ui->Motor_car->setText("00");
    ui->Input_digit->setText("00");
    ui->Output_dummy->setText("00");
   ui->Marshalling_num1->setText("00");
    ui->Marshalling_num2->setText("00");
    ui->Speed_train0->setText("00");
    ui->Speed_train1->setText("00");
    ui->Temperature_outer->setText("00");
    ui->Control_mode->setText("00");
    ui->Valid_bit->setText("00");
    ui->Air_pressure1->setText("00");
    ui->Air_pressure2->setText("00");
    ui->Gps_data0->setText("00");
    ui->Gps_data1->setText("00");
    ui->Gps_data2->setText("00");
    ui->Gps_data3->setText("00");
    ui->Gps_data4->setText("00");
    ui->Gps_data5->setText("00");
    ui->Gps_data6->setText("00");
    ui->Gps_data7->setText("00");
    ui->Gps_data8->setText("00");
    ui->Gps_data9->setText("00");
    ui->Check_sum0->setText("00");
    ui->Check_sum1->setText("00");
}


void MainWindow::on_stoptest_clicked()
{
    m_Timer1->stop();
    ui->set->setEnabled(true);
    sendNum = 0;
    ui->SendNum->setText("0");
    ui->ReceivePacket->setText("0");
    ui->RightPacket->setText("0");
}
void MainWindow::on_actStart_triggered()
{//加入组播
    QString     IP;
    quint16     groupPort;

    IP =ui->McomboRecvIP->currentText();
    groupAddress=QHostAddress(IP);//多播组地址
    groupPort=ui->MRecvPort->value();//端口


    if (MudpSocket->bind(QHostAddress::AnyIPv4, groupPort, QUdpSocket::ShareAddress))//先绑定端口
    {
        MudpSocket->joinMulticastGroup(groupAddress); //加入多播组
        ui->actStart->setEnabled(false);
        ui->actStop->setEnabled(true);
        ui->McomboRecvIP->setEnabled(false);
        ui->MRecvPort->setEnabled(false);
    }
}

void MainWindow::on_actStop_triggered()
{//退出组播

    groupAddress=QHostAddress(ui->McomboRecvIP->currentText());//多播组地址
    MudpSocket->leaveMulticastGroup(groupAddress);//退出组播
    MudpSocket->abort(); //解除绑定
    ui->actStart->setEnabled(true);
    ui->actStop->setEnabled(false);
    ui->McomboRecvIP->setEnabled(true);
    ui->MRecvPort->setEnabled(true);
}

void MainWindow::on_actStartUdp_triggered()
{//绑定端口

    quint16     port=ui->LspinPort->value(); //本机UDP端口
    if (UudpSocket->bind(port))//绑定端口成功
    {
        ui->actStartUdp->setEnabled(false);
        ui->actStopUdp->setEnabled(true);
    }
}

void MainWindow::on_actStopUdp_triggered()
{//解除绑定
    UudpSocket->abort(); //不能解除绑定
    ui->actStartUdp->setEnabled(true);
    ui->actStopUdp->setEnabled(false);
}

void MainWindow::on_actStartSend_triggered()
{
    QString  IP =ui->McomboSendIP->currentText();
    groupAddress=QHostAddress(IP);//多播组地址
    quint16     port = ui->LspinPort->value();
    if (MudpSendSocket->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress))//先绑定端口
    {
        MudpSendSocket->joinMulticastGroup(groupAddress); //加入多播组
        ui->actStartSend->setEnabled(false);
        ui->actStopSend->setEnabled(true);
        ui->McomboSendIP->setEnabled(false);
        ui->MSendPort->setEnabled(false);

    }
}

void MainWindow::on_actStopSend_triggered()
{
    groupAddress=QHostAddress(ui->McomboSendIP->currentText());//多播组地址
    MudpSendSocket->leaveMulticastGroup(groupAddress);//退出组播
    MudpSendSocket->abort(); //解除绑定
    ui->actStartSend->setEnabled(true);
    ui->actStopSend->setEnabled(false);
    ui->McomboSendIP->setEnabled(true);
    ui->MSendPort->setEnabled(true);
}

void MainWindow::on_BoardType_currentIndexChanged(int index)
{
    if (index == 0)
    {
        ui->SensorStatusLabel->setText("26:加速度传感器和平稳采集卡故障");
        ui->RealayLabel->setText("27:1位横向指标");
        ui->BogieFaultLabel->setText("28:2位横向指标");
        ui->BogieWarnLabel->setText("29:1位垂向指标");
        ui->BogieFaultNumLabel->setText("30:2位垂向指标");
        ui->ParaLevelLabel->setText("31:1位纵向指标");
        ui->DetectParaLabel1->setText("32:2位纵向指标");
        ui->DetectParaLabel2->setText("33:总平稳报警状态");
        ui->DetectParaLabel3->setText("34:1和2位平稳报警状态");
        ui->ModleFaultLabel->setText("35:预留");
    }
    else if (index == 1)
    {

        ui->SensorStatusLabel->setText("26:转向架加速度传感器");
        ui->RealayLabel->setText("27:继电器故障");
        ui->BogieFaultLabel->setText("28:转向架故障");
        ui->BogieWarnLabel->setText("29:转向架预判");
        ui->BogieFaultNumLabel->setText("30:转向架异常累计发生件数");
        ui->ParaLevelLabel->setText("31:检测参数等级");
        ui->DetectParaLabel1->setText("32:异常检测参数1");
        ui->DetectParaLabel2->setText("33:异常检测参数2");
        ui->DetectParaLabel3->setText("34:异常检测参数3");
        ui->ModleFaultLabel->setText("35:模块故障等");
    }
    else
    {

    }
}
