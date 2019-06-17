#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::showProcessByColor(cv::Mat mat)
{
    this->ui->tabWidget->clear();

    QImage image = ImageUtilities::Mat2QImage(mat,QImage::Format_RGB888);

    //构建Tab页面
    QWidget* tabOriginal = this->generateImageLabel(mat,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabOriginal,tr("原图"));

    //HCV色彩空间

    cv::Mat matHSV;
    cv::cvtColor(mat,matHSV,cv::COLOR_BGR2HSV);
    QWidget* tabHSV = this->generateImageLabel(matHSV,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabHSV,tr("HSV"));

    cv::Mat* matHSVs = new cv::Mat[3];
    cv::split(matHSV,matHSVs);

    QWidget* tabH = this->generateImageLabel(matHSVs[0],QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabH,tr("H"));

    QWidget* tabS = this->generateImageLabel(matHSVs[1],QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabS,tr("S"));

    QWidget* tabV = this->generateImageLabel(matHSVs[2],QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabV,tr("V"));

    //均衡化处理

    cv::Mat matEqualization_V;
    cv::equalizeHist(matHSVs[0],matEqualization_V);
    QWidget* tabE_V = this->generateImageLabel(matEqualization_V,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabE_V,tr("E_V"));

    //颜色识别
    cv::Mat matHSV_E;
    cv::merge(matHSVs,3,matHSV_E);
    QWidget* tabHSV_E = this->generateImageLabel(matHSV_E,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabHSV_E,tr("HSV_E"));

    cv::Scalar yellow_Low(100,43,46);
    cv::Scalar yellow_Up(124,255,255);
    cv::Mat matYellow;

    cv::inRange(matHSV_E,yellow_Low,yellow_Up,matYellow);
    QWidget* tabYellow = this->generateImageLabel(matYellow,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabYellow,tr("Yellow"));

    //膨胀腐蚀
    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3) );
    cv::Mat matYellow_dilate;
    cv::dilate(matYellow,matYellow_dilate,element);
    QWidget* tabYellow_dilate = this->generateImageLabel(matYellow_dilate,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabYellow_dilate,tr("Yellow_dilate"));
    cv::Mat matYellow_erode;
    cv::erode(matYellow_dilate,matYellow_erode,element);
    QWidget* tabYellow_erode = this->generateImageLabel(matYellow_erode,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabYellow_erode,tr("Yellow_erode"));

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(matYellow_dilate,contours,cv::RETR_TREE,cv::CHAIN_APPROX_NONE);
    cv::Mat matClone = mat.clone();
    cv::drawContours(matClone,contours,-1,cv::Scalar(0,0,255),2);
    QWidget* tabmatClone = this->generateImageLabel(matClone,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabmatClone,tr("contours"));

    //求轮廓最小外接矩形
    cv::Mat matRects = mat.clone();
    std::vector<cv::Rect> rects;
    for (int index = 0;index < contours.size();index++)
    {
        cv::Rect rect = cv::boundingRect(contours[index]);
        rects.push_back(rect);
        cv::rectangle(matRects,rect,cv::Scalar(255,0,0),1);

    }
    QWidget* tabRects = this->generateImageLabel(matRects,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabRects,tr("Rects"));

    this->showMatSplitResult(mat,rects);

    mat.release();
    matHSV.release();
    matEqualization_V.release();
    matHSV_E.release();
    matYellow.release();
    matYellow_erode.release();
    matYellow_dilate.release();
    matRects.release();
    matClone.release();
}

void MainWindow::showProcessBySobel(cv::Mat mat)
{
    if(mat.empty()) return;
    this->ui->tabWidget->clear();

    QImage image = ImageUtilities::Mat2QImage(mat,QImage::Format_RGB888);

    //构建Tab页面
    QWidget* tabOriginal = this->generateImageLabel(mat,QImage::Format_RGB888);
    this->ui->tabWidget->addTab(tabOriginal,tr("原图"));

    //高斯模糊
    cv::Mat matBlur;
    cv::GaussianBlur(mat,matBlur,cv::Size(3,3),0);
    QWidget* tabBlur = this->generateImageLabel(matBlur,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabBlur,tr("模糊图"));

    //灰度处理
    cv::Mat matGray ;
    cv::cvtColor(matBlur,matGray,cv::COLOR_BGR2GRAY);
    QWidget* tabGray = this->generateImageLabel(matGray,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabGray,tr("灰度图"));

    cv::Mat grad_X;
    cv::Sobel(matGray,grad_X,CV_16S,1,0);
    cv::Mat grad_Xabs;
    cv::convertScaleAbs(grad_X,grad_Xabs);
    cv::Mat grad_Y;
    cv::Sobel(matGray,grad_Y,CV_16S,1,0);
    cv::Mat grad_Yabs;
    cv::convertScaleAbs(grad_Y,grad_Yabs);

    cv::Mat matGrad;
    cv::addWeighted(grad_Xabs,1,grad_Yabs,0,0,matGrad);
    QWidget* tabGrad = this->generateImageLabel(matGrad,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabGrad,tr("梯度图"));

    cv::Mat matThreshold;
    cv::threshold(matGrad,matThreshold,100,255,cv::THRESH_BINARY);
    QWidget* tabThreshold = this->generateImageLabel(matThreshold,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabThreshold,tr("二值化"));

    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(7,1) );
    cv::Mat matMorphologyEx_C;
    cv::morphologyEx(matThreshold,matMorphologyEx_C,cv::MORPH_CLOSE,element);
    QWidget* tabMorphologyEx_C = this->generateImageLabel(matMorphologyEx_C,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabMorphologyEx_C,tr("闭操作"));

    cv::Mat matYellow_erode;
    cv::erode(matMorphologyEx_C,matYellow_erode,element);
    QWidget* tabYellow_erode = this->generateImageLabel(matYellow_erode,QImage::Format_Grayscale8);
    this->ui->tabWidget->addTab(tabYellow_erode,tr("腐蚀"));


}

QLabel* MainWindow::generateImageLabel(cv::Mat mat,QImage::Format format)
{
    QLabel* lblImage = new QLabel();
    /*QImage image;
    if(mat.cols>1000)
    {
        cv::Mat resize;
        cv::Size outsize;
        outsize.width = mat.cols*0.15;
        outsize.height = mat.rows*0.15;
        cv::resize(mat,resize,outsize,0,0,cv::INTER_NEAREST);
        image = ImageUtilities::Mat2QImage(resize,format);
        resize.release();
    }
    else
    {
        image = ImageUtilities::Mat2QImage(mat,format);
    }*/
    QImage image = ImageUtilities::Mat2QImage(mat,format);
    QPixmap pixmap = QPixmap::fromImage(image);

    lblImage->setPixmap(pixmap);
    lblImage->resize(pixmap.size());

    return lblImage;
}

void MainWindow::showMatSplitResult(cv::Mat mat, std::vector<cv::Rect> rects)
{
    this->ui->lstImages->clear();

    if(mat.empty()) return;
    for (int index = 0; index < rects.size(); index++)
    {
        cv::Mat roi = mat(rects[index]);
        QPixmap pixmap = QPixmap::fromImage(ImageUtilities::Mat2QImage(roi,QImage::Format_RGB888));
        QIcon iconImage(pixmap);
        QListWidgetItem *listWidgetItem = new QListWidgetItem();
        listWidgetItem->setIcon(iconImage);
        this->ui->lstImages->addItem(listWidgetItem);
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
        this->showProcessByColor(resize);
        resize.release();

    }
    else {
            //this->showProcessBySobel(mat);
        this->showProcessBySobel(mat);
    }

}
