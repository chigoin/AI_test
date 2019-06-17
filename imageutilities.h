#ifndef IMAGEUTILITIES_H
#define IMAGEUTILITIES_H

#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class ImageUtilities
{
public:
    ImageUtilities();

    static QImage Mat2QImage(cv::Mat mat, QImage::Format format);
};

#endif // IMAGEUTILITIES_H
