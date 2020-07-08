#include "mainwindow.h"
#include "mat2qimage.h"
#include "ui_mainwindow.h"
#include <QTimer>

#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv/cv.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/video/background_segm.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/objdetect.hpp>

#include<QtNetwork>
#include<QDebug>

using namespace cv;

QString nameBody = "../cara.xml";

CascadeClassifier Body;

QString IPCamera = "http://192.168.1.75:8080/video";

VideoCapture camera(0);

int counter = 0;
int counterConnect = 0;

int xMax;
int xMin;
int yMax;
int yMin;

double divx;
double divy;

bool setEnable;

Mat image;
Mat errorImage(288,512, CV_8UC3, Scalar(0,0,255));


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);                                          //Configure gui

    Body.load(nameBody.toUtf8().constData());                   //Load HAAR classifier

     QTimer *cronometro = new QTimer(this);                     //Timer

     connect(cronometro, SIGNAL(timeout()),this,SLOT(Timer())); //Connect signal

     cronometro->start(10);                                      //Start program
}

void MainWindow::Timer(){

    counter++;

    ui->lcdNumber->display(counter);

    capture();
}

void MainWindow::connectServer(QString url, QString message){

    QString serverAnswer = "";

    QNetworkInterface networkConnected = QNetworkInterface::interfaceFromName("wlo1");

    QList<QNetworkAddressEntry> list = networkConnected .addressEntries();

    if(!list.empty()){

        QNetworkAddressEntry IPasignada = list.first();
        qDebug() << "IP Asignada:" << IPasignada.ip() << endl;

        //Create web client
        QNetworkAccessManager *webClient = new QNetworkAccessManager();

        //Assign url to webClient
        QUrl server(url.toUtf8().constData());

        //Create HTML menssage
        QNetworkRequest request;
        QByteArray messageLength = QByteArray::number(message.size());

        if(server.isValid()){

              //Form request
              request.setUrl(server);
              request.setRawHeader(QByteArray("user-Agent"),QByteArray("botDAN"));
              request.setRawHeader(QByteArray("Connection"),QByteArray("closed"));
              request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
              request.setHeader(QNetworkRequest::ContentLengthHeader, messageLength);

              //Send post to server
              QNetworkReply *serverConnect =  webClient->post(request,message.toLatin1());


  }
 }
}

void MainWindow::capture()
{
    Point bodyCenter;
    Point SupIzq;
    Point SupDer;
    Point InfIzq;
    Point InfDer;

    camera >> image;             //Capture frame

    if(image.empty()){
        errorImage.copyTo(image);//Send error image if empty
    }

 //*****Change image size to ease processing****************************
        Mat smallImage;
        cv::resize(image, smallImage, Size(512,288),0,0,INTER_LINEAR);
//**********************************************************************
 //*****Start object recognition****************************
        if(!ui->checkBox->isChecked()){


            if(setEnable){ //Wait calibration

                divx = 512/(xMax - xMin);
                divy = 288/(yMax - yMin);

                setEnable = false;
            }
            //Disable all the inputs
            ui->horizontalSlider->setDisabled(true);
            ui->horizontalSlider_2->setDisabled(true);
            ui->pushButton->setDisabled(true);
            ui->pushButton_2->setDisabled(true);
            ui->pushButton_3->setDisabled(true);
            ui->pushButton_4->setDisabled(true);
            ui->pushButton_5->setDisabled(true);

            //Change image color to gray
            Mat grayImage;
            cvtColor(smallImage, grayImage, CV_BGR2GRAY);
            cv::equalizeHist(grayImage, grayImage);

            //Find the objects
            std::vector<Rect> foundObjects;
            Body.detectMultiScale(grayImage, foundObjects, 1.1, 3, 0|CASCADE_SCALE_IMAGE, Size(30,30));

            if(foundObjects.size() > 0){

                Mat uniqueBody = grayImage(foundObjects[0]);
                bodyCenter.x = foundObjects[0].x + foundObjects[0].width/2;
                bodyCenter.y = foundObjects[0].y + foundObjects[0].height/2;

                //Set corner points
                SupIzq.x = foundObjects[0].x;
                SupIzq.y = foundObjects[0].y;
                SupDer.x = foundObjects[0].x + foundObjects[0].width;
                SupDer.y = foundObjects[0].y;
                InfIzq.x = foundObjects[0].x;
                InfIzq.y = foundObjects[0].y + foundObjects[0].height;
                InfDer.x = foundObjects[0].x + foundObjects[0].width;
                InfDer.y = foundObjects[0].y + foundObjects[0].height;

                //Draw a square
                line(smallImage,  SupIzq, SupDer, Scalar(255,0,0,0),5,8,0);
                line(smallImage,  SupDer, InfDer, Scalar(255,0,0,0),5,8,0);
                line(smallImage,  InfDer, InfIzq, Scalar(255,0,0,0),5,8,0);
                line(smallImage,  InfIzq, SupIzq, Scalar(255,0,0,0),5,8,0);

                counterConnect ++;

                }


            if(counterConnect > 15){ //Delay to avoid damage the server

                counterConnect = 0;

                int ejeX = xMax - (bodyCenter.x / divx);
                int ejeY = yMax - (bodyCenter.y / divy);


                //Send message to server
                QString Angle = "{\"angle1\":\""+QString::number(ejeX)+"\",\"angle2\":\""+QString::number(ejeY)+"\"}";
                connectServer("http://robot.local",Angle);

               }


        }
        else{   //Enable input buttons

            ui->horizontalSlider->setDisabled(false);
            ui->horizontalSlider_2->setDisabled(false);
            ui->pushButton->setDisabled(false);
            ui->pushButton_2->setDisabled(false);
            ui->pushButton_3->setDisabled(false);
            ui->pushButton_4->setDisabled(false);
            ui->pushButton_5->setDisabled(false);

            setEnable = true;
        }

        //Send image to label
        QImage imagenQT = Mat2QImage(smallImage);
        QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);
        ui->label->clear();
        ui->label->setPixmap(mapaPixeles);
}



MainWindow::~MainWindow()
{
    delete ui;
}





void MainWindow::on_pushButton_clicked()
{
    int x = ui->horizontalSlider->value();
    int y = ui->horizontalSlider_2->value();

    QString Angle = "{\"angle1\":\""+QString::number(x)+"\",\"angle2\":\""+QString::number(y)+"\"}";
    connectServer("http://robot.local",Angle);
}

void MainWindow::on_pushButton_2_clicked()
{
    xMax = ui->horizontalSlider->value();
    qDebug() << xMax;
}

void MainWindow::on_pushButton_3_clicked()
{
    xMin = ui->horizontalSlider->value();
    qDebug() << xMin;
}



void MainWindow::on_pushButton_5_clicked()
{
    yMax = ui->horizontalSlider_2->value();
    qDebug() << yMax;
}



void MainWindow::on_pushButton_4_clicked()
{
    yMin = ui->horizontalSlider_2->value();
    qDebug() << yMin;
}
