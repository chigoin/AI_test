#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QStringList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPixmap>
#include <QPixmapCache>
#include <QImage>
#include <QTabWidget>
#include <QWidget>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "imageutilities.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString pathSelected;

    //void showMat(cv::Mat mat);
    void showProcessByColor(cv::Mat mat);
    void showProcessBySobel(cv::Mat mat);

    void showMatSplitResult(cv::Mat , std::vector<cv::Rect> );

    QLabel* generateImageLabel(cv::Mat mat, QImage::Format format);


private slots:
    void selectDir();
    void on_lstFiles_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};

#endif // MAINWINDOW_H
