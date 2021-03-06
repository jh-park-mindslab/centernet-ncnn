#include <iostream>
#include <opencv2/opencv.hpp>
#include "cpp/ncnn_centernet.h"
#include <sys/time.h>


float getElapse(struct timeval *tv1,struct timeval *tv2)
{
    float t = 0.0f;
    if (tv1->tv_sec == tv2->tv_sec)
        t = (tv2->tv_usec - tv1->tv_usec)/1000.0f;
    else
        t = ((tv2->tv_sec - tv1->tv_sec) * 1000 * 1000 + tv2->tv_usec - tv1->tv_usec)/1000.0f;
    return t;
}

/*
 * Draw objects
 * @bgr : cv::Mat (BGR)
 * @obj_info : obj info
 */
static void draw_objects(const cv::Mat& bgr, const std::vector<ObjInfo> obj_info, float ratio_w, float ratio_h)
{
    static const char* class_names[] = {"person", };

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < obj_info.size(); i++) {
        if (obj_info[i].score > 0.25) {

            const ObjInfo& obj = obj_info[i];

            fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.score, obj.x1, obj.y1, obj.x2, obj.y2);

            cv::rectangle(image, cv::Point(obj.x1 / ratio_w, obj.y1 / ratio_h), cv::Point(obj.x2 /ratio_w, obj.y2 / ratio_h), cv::Scalar(255, 0, 0));

            char text[256];
            sprintf(text, "%s %.1f%%", class_names[obj.label], obj.score * 100);

            int baseLine = 0;
            cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            int x = obj.x1 / ratio_w;
            int y = obj.y1 / ratio_h - label_size.height - baseLine;
            if (y < 0)
                y = 0;
            if (x + label_size.width > image.cols)
                x = image.cols - label_size.width;

            cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                    cv::Scalar(255, 255, 255), -1);

            cv::putText(image, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        }
    }

    cv::imwrite("result.jpg", image);
}


int main(int argc, char** argv) {

    if (argc !=3) {
        std::cout << " .exe mode_path image_file" << std::endl;
        return -1;
    }

    struct timeval  tv1,tv2;
    struct timezone tz1,tz2;

    std::string model_path = argv[1];
    std::string image_file = argv[2];

    Centerobj centerobj;
    centerobj.init(model_path);

    cv::Mat image = cv::imread(image_file);

    int ori_w = image.cols;
    int ori_h = image.rows;
    printf("ori_w, ori_h : %d %d \n", ori_w, ori_h);

    float ratio = 512 * 1.0 / ori_w;
    if (ori_h > ori_w) {
        ratio = 512 * 1.0 / ori_h;
    }
    float ratio_w = 512 * 1.0 / ori_w;
    float ratio_h = 512 * 1.0 / ori_h;

    printf("ratio : %f \n", ratio);
    printf("ori_w * ratio, ori_h * ratio : %lf %lf \n", ori_w * ratio, ori_h * ratio);

    cv::Mat img_resize;
    cv::resize(image, img_resize, cv::Size(512, 512), (0, 0), (0, 0), cv::INTER_LINEAR);
    printf("img_resize.cols, img_resize.rows : %d %d \n", img_resize.cols, img_resize.rows);
    std::vector<ObjInfo> obj_info;
    ncnn::Mat inmat = ncnn::Mat::from_pixels(img_resize.data, ncnn::Mat::PIXEL_BGR, img_resize.cols, img_resize.rows);
    std::cout << "cols = " << img_resize.cols << ", " << "rows = " << img_resize.rows << std::endl;
    for(int i = 0; i < 5; i++) {
        gettimeofday(&tv1, &tz1);
        centerobj.detect(inmat, obj_info, img_resize.cols, img_resize.rows);
        gettimeofday(&tv2, &tz2);
        float tc = getElapse(&tv1, &tv2);
        std::cout << "time : " << tc << "ms" << std::endl;
    }

    if (0 < obj_info.size()) {
        draw_objects(image, obj_info, ratio_w, ratio_h);
        printf("draw_objects is done. \n");
    } else {
        printf("obj_info.size() is 0 \n");
    }

    // cv::imshow("test.jpg", image);
    // cv::waitKey();

    return 0;
}
