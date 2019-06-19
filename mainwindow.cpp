#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction* actionSelectDir = new QAction(tr("选择路径"),this);

    this->connect(actionSelectDir,SIGNAL(triggered()),this,SLOT(selectDir()));

    this->ui->mainToolBar->addAction(actionSelectDir);

    //this->setWindowState(Qt::WindowState::WindowMaximized);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMat(cv::Mat mat)
{
        this->ui->tabWidget->clear();
        QImage image = ImageUtilities::Mat2QImage(mat,QImage::Format_RGB888);

        //构建Tab页面
        QWidget* tabOriginal = this->generateImageLabel(mat,QImage::Format_RGB888);
        this->ui->tabWidget->addTab(tabOriginal,tr("SOURCE"));

        //将BGR图片转化为HSV图片
        cv::Mat matHSV;
        cv::cvtColor(mat,matHSV,cv::COLOR_BGR2HSV);
        QWidget* tabHSV = this->generateImageLabel(matHSV,QImage::Format_RGB888);
        this->ui->tabWidget->addTab(tabHSV,tr("HSV"));

        //分通道
        cv::Mat* matHSVs = new cv::Mat[3];
        cv::split(matHSV,matHSVs);
        QWidget* tabH = this->generateImageLabel(matHSVs[0],QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabH,tr("H"));
        QWidget* tabS = this->generateImageLabel(matHSVs[1],QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabS,tr("S"));
        QWidget* tabV = this->generateImageLabel(matHSVs[2],QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabV,tr("V"));

        //matV的均衡
        cv::Mat matV_EqualizeHis;
        cv::equalizeHist(matHSVs[2],matV_EqualizeHis);
        QWidget* tabV_EqualizeHis = this->generateImageLabel(matV_EqualizeHis,QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabV_EqualizeHis,tr("V_Eq"));

        //合并
        matHSVs[2] = matV_EqualizeHis;
        cv::Mat matHSV_Eq;
        cv::merge(matHSVs,3, matHSV_Eq);
        QWidget* tabHSV_Eq = this->generateImageLabel(matHSV_Eq,QImage::Format_RGB888);
        this->ui->tabWidget->addTab(tabHSV_Eq,tr("HSV_Eq"));

        //在均衡化后的HSV颜色空间中寻找黄色区域
        cv::Scalar yellow_Low(15,95,95);
        cv::Scalar yellow_Upper(40,255,255);
        cv::Mat matYellow;
        cv::inRange(matHSV_Eq,yellow_Low,yellow_Upper,matYellow);
        QWidget* tabYellow = this->generateImageLabel(matYellow,QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabYellow,tr("Yellow"));

        //在均衡化后的HSV颜色空间寻找蓝色区域
        cv::Scalar blue_low(100,95,95);
        cv::Scalar blue_upper(140,255,255);
        cv::Mat matBlue;
        cv::inRange(matHSV_Eq,blue_low,blue_upper,matBlue);
        QWidget* tabBlue = this->generateImageLabel(matBlue,QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabBlue,tr("Blue"));

        //使用形态学操作来对选定的颜色区域进行处理，矩形块
        //膨胀
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(7,3));
        cv::Mat matYellow_Dilate;
        cv::dilate(matYellow,matYellow_Dilate,element);

        QWidget* tabYellow_Dilate = this->generateImageLabel(matYellow_Dilate,QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabYellow_Dilate,tr("Yellow_D"));
        //腐蚀
        cv::Mat matYellow_Erode;
        cv::erode(matYellow_Dilate,matYellow_Erode,element);
        element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3));

        QWidget* tabYellow_Erode = this->generateImageLabel(matYellow_Erode,QImage::Format_Grayscale8);
        this->ui->tabWidget->addTab(tabYellow_Erode,tr("Yellow_E"));

        //对蓝色进行膨胀操作
        cv::Mat elementBlue = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(7,3));
        cv::Mat matBlue_Dilate;
        cv::dilate(matBlue,matBlue_Dilate,elementBlue);

        //寻找车牌

        std::vector<std::vector<cv::Point>> contoursYellow;
        std::vector<std::vector<cv::Point>> contoursBlue;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchyYellow;
        std::vector<cv::Vec4i> hierarchyBlue;
        cv::findContours(matYellow_Dilate,contoursYellow,hierarchyYellow,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(matBlue_Dilate,contoursBlue,hierarchyBlue,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        contours.insert(contours.end(),contoursBlue.begin(),contoursBlue.end());
        contours.insert(contours.end(),contoursYellow.begin(),contoursYellow.end());
        cv::Mat matContours = mat.clone();
        cv::drawContours(matContours,contours,-1,cv::Scalar(0,0,255),2);
        QWidget* tabContours = this->generateImageLabel(matContours,QImage::Format_RGB888);
        this->ui->tabWidget->addTab(tabContours,tr("Contours"));

        //求轮廓的最小外接矩形
        cv::Mat matRects = mat.clone();
        std::vector<cv::Rect> rects;
        for (int index = 0; index < contours.size();index++)
        {
           cv::Rect rect = cv::boundingRect(contours[index]);
           double radio=rect.width/rect.height;
           if(0.95<radio<2.5)
           {
               rects.push_back(rect);
               cv::rectangle(matRects,rect,cv::Scalar(255,0,0),1);
           }

         }

         QWidget* tabRects = this->generateImageLabel(matRects,QImage::Format_RGB888);
         this->ui->tabWidget->addTab(tabRects,tr("Rects"));
         this->showMatSplitResult(mat,rects);




      mat.release();
      matHSV.release();
      delete[] matHSVs;
      matRects.release();
      matYellow.release();
      matContours.release();
      matHSV_Eq.release();
      matYellow_Erode.release();
      matYellow_Dilate.release();
      matBlue.release();
      matV_EqualizeHis.release();

}


void MainWindow::showProcessBySobel(cv::Mat mat)
{
    if(mat.empty()) return;

    this->ui->tabWidget->clear();

    //构建Tab页面
    QWidget* tabOriginal = this->generateImageLabel(mat,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabOriginal,tr("原图"));

    //高斯模糊
    cv::Mat matBlur;
    cv::GaussianBlur(mat,matBlur,cv::Size(3,3),0);

    QWidget* tabBlur = this->generateImageLabel(matBlur,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabBlur,tr("模糊"));

    //灰度处理
    cv::Mat matGray;
    cv::cvtColor(matBlur,matGray,cv::COLOR_BGR2GRAY);

    QWidget* tabGray = this->generateImageLabel(matGray,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabGray,tr("灰度"));

    cv::Mat grad_X;
    cv::Sobel(matGray,grad_X,CV_16S,1,0);
    cv::Mat grad_X_abs;
    cv::convertScaleAbs(grad_X,grad_X_abs);

    cv::Mat grad_Y;
    cv::Sobel(matGray,grad_Y,CV_16S,0,1);
    cv::Mat grad_Y_abs;
    cv::convertScaleAbs(grad_Y,grad_Y_abs);

    cv::Mat matGrad;
    cv::addWeighted(grad_X_abs,1,grad_Y_abs,0,0,matGrad);

    QWidget* tabGrad = this->generateImageLabel(matGrad,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabGrad,tr("梯度"));

    cv::Mat matThreshold;
    cv::threshold(matGrad,matThreshold,100,255,cv::THRESH_BINARY);
    QWidget* tabThreshold = this->generateImageLabel(matThreshold,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabThreshold,tr("二值化"));

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(11,3));
    cv::Mat matClose;
    cv::morphologyEx(matThreshold,matClose,cv::MORPH_CLOSE,element);


    QWidget* tabClose = this->generateImageLabel(matClose,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabClose,tr("闭操作"));
    //腐蚀操作
    element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3));
    cv::Mat matErode;
    cv::erode(matClose,matErode,element);

    QWidget* tabErode = this->generateImageLabel(matErode,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabErode,tr("腐蚀"));
    //求轮廓操作
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(matErode,contours,hierarchy,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);

    cv::Mat matContours = mat.clone();
    cv::drawContours(matContours,contours,-1,cv::Scalar(0,0,255),2);

    QWidget* tabContours = this->generateImageLabel(matContours,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabContours,tr("轮廓"));

    //求轮廓的最小外接矩形
    cv::Mat matRects = mat.clone();
    std::vector<cv::Rect> rects;
    for (int index = 0; index < contours.size();index++)
    {
        cv::Rect rect = cv::boundingRect(contours[index]);
        if(rect.width>rect.height)
        {
            rects.push_back(rect);
            cv::rectangle(matRects,rect,cv::Scalar(255,0,0),1);
        }
     }

    QWidget* tabRects = this->generateImageLabel(matRects,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabRects,tr("Rects"));

    this->showMatSplitResult(mat,rects);

}


QLabel* MainWindow::generateImageLabel(cv::Mat mat,QImage::Format format)
{
    QLabel* lblImage = new QLabel();
    QImage image = ImageUtilities::Mat2QImage(mat,format);
    QPixmap pixmap = QPixmap::fromImage(image);

    lblImage->setPixmap(pixmap);
    lblImage->resize(pixmap.size());

    return lblImage;
}

void MainWindow::showMatSplitResult(cv::Mat mat, std::vector<cv::Rect> rects)
{
    this->ui->lstImages->clear();
    //this->ui->lstImages->setIconSize(QSize(1000,1000));
    if(mat.empty()) return;
    for (int index = 0; index < rects.size(); index++)
    {
        cv::Mat roi = mat(rects[index]);
        QPixmap pixmap = QPixmap::fromImage(ImageUtilities::Mat2QImage(roi,QImage::Format_RGB888));
        QIcon iconImage(pixmap);
        QListWidgetItem *listWidgetItem = new QListWidgetItem(this->ui->lstImages,0);
        listWidgetItem->setSizeHint(QSize(150,150));
        QWidget *w = new QWidget(this->ui->lstImages);
        QHBoxLayout *layout=new QHBoxLayout(w);
        QLabel *imgLabel=new QLabel(w);

        int width =imgLabel->width();
        //int height =imgLabel->height();
        QPixmap fitpixmap = pixmap.scaled(width, pixmap.height()*(width/pixmap.width()),Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        imgLabel->setPixmap(fitpixmap);
        layout->addWidget(imgLabel);
        w->setLayout(layout);
        w->show();
        this->ui->lstImages->setItemWidget(listWidgetItem,w);
        this->ui->lstImages->show();

    }
}

void MainWindow::selectDir()
{
    this->pathSelected = QFileDialog::getExistingDirectory(this,
                                                           tr("选择图片所在文件夹"),
                                                           this->pathSelected.isEmpty()?tr("C:/Users/27505/Desktop/car"):this->pathSelected);
    if(this->pathSelected.isEmpty()) return;

    QDir* dir = new QDir(this->pathSelected);
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.png" << "*.bmp";
    QStringList imgFileNames = dir->entryList(nameFilters);


    for (int i=0;i<imgFileNames.size();i++)
    {
        this->ui->lstFiles->addItem(imgFileNames[i]);
    }

}

void MainWindow::on_lstFiles_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{

    if(current == nullptr) return;

    QString imgFileName = current->text();
    imgFileName = this->pathSelected + "/" + imgFileName;
    std::string path = imgFileName.toLocal8Bit().toStdString();

    cv::Mat mat = cv::imread(path);

    if(mat.cols>1000)
    {
        cv::Mat resize;
        cv::Size outsize;
        outsize.width = mat.cols*0.5;
        outsize.height = mat.rows*0.5;
        cv::resize(mat,resize,outsize,0,0,cv::INTER_NEAREST);
        //this->showProcessBySobel(resize);
        this->showMat(resize);
        resize.release();

    }
    else {
            this->showMat(mat);
            //this->showProcessBySobel(mat);
    }

}
