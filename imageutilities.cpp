#include "imageutilities.h"

ImageUtilities::ImageUtilities()
{

}

QImage ImageUtilities::Mat2QImage(cv::Mat mat, QImage::Format format)
{
    cv::Mat temp = mat.clone();

        if(mat.channels() == 3) cv::cvtColor(temp,mat,cv::COLOR_BGR2RGB);

        QImage image = QImage((const unsigned char*)(temp.data),
                              temp.cols,
                              temp.rows,
                              temp.cols * temp.channels(),
                              format);

        temp.release();

        return image;

//        if(mat.channels() == 3)
//        {
//            cv::cvtColor(mat,mat,cv::COLOR_BGR2RGB);
//        }


//        QImage image = QImage(static_cast<uchar *>(mat.data),
//                              mat.cols,
//                              mat.rows,
//                              mat.cols * mat.channels(),
//                              format);

//        return image;
}
