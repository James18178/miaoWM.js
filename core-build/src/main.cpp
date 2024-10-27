#include <iostream>
#include <string>

#include <opencv2/core/version.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>


using namespace emscripten;

static const std::string version = "miaowuWaterMark-release-v1.0.0";

static const std::string opencv_version = CV_VERSION;

// 声明
std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url = false);
cv::Mat simple_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color);
cv::Mat full_screen_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color);
bool can_place_text(int x, int y, const cv::Mat& occupied, const cv::Size& text_size, const cv::Mat& edges);
void occupy_region(int x, int y, cv::Mat& occupied, const cv::Size& text_size);
cv::Mat edge_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color, double gain);
std::string cvobj_to_base64(const cv::Mat& opencv_img);
cv::Scalar string_to_scalar(const std::string& input);

// base64定义
static const char* base64_chars[2] = {
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "+/",

             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789"
             "-_"};

std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url) {

    size_t len_encoded = (in_len +2) / 3 * 4;

    unsigned char trailing_char = url ? '.' : '=';

    const char* base64_chars_ = base64_chars[url];

    std::string ret;
    ret.reserve(len_encoded);

    unsigned int pos = 0;

    while (pos < in_len) {
        ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

        if (pos+1 < in_len) {
           ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

           if (pos+2 < in_len) {
              ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
              ret.push_back(base64_chars_[  bytes_to_encode[pos + 2] & 0x3f]);
           }
           else {
              ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
              ret.push_back(trailing_char);
           }
        }
        else {

            ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
            ret.push_back(trailing_char);
            ret.push_back(trailing_char);
        }

        pos += 3;
    }

    return ret;
}

// 简单水印
// void simple_watermark(const std::string& image_path, const std::string& output_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255)) {
cv::Mat simple_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255)) {
    cv::Mat img = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    int h = img.rows;
    int w = img.cols;
    cv::Mat layer(h, w, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::Point text_position(50, h - 50);
    cv::putText(layer, text, text_position, cv::FONT_HERSHEY_SIMPLEX, 2, font_color, 6, cv::LINE_AA);
    cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
    cv::Mat result;
    cv::addWeighted(img, 1.0, layer, 1.0, 0, result);
    // cv::imwrite(output_path, result);
    return result;
}

// 全屏水印
// void full_screen_watermark(const std::string& image_path, const std::string& output_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255)) {
cv::Mat full_screen_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255)) {
    cv::Mat img = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    int h = img.rows;
    int w = img.cols;
    int a_w = 10;
    cv::Mat layer(h, w, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::Point text_position;
    cv::Scalar bg_color(0, 0, 0, 0);
    for (int w_s = 0; w_s < w / text.length(); w_s++) {
        if (w_s % 2 == 0) {
            a_w += text.length() * 13;
            continue;
        }
        int a_h = 50;
        for (int i = 0; i < h / 2; i++) {
            text_position = cv::Point(a_w, a_h);
            cv::putText(layer, text, text_position, cv::FONT_HERSHEY_SIMPLEX, 0.5, font_color, 1, cv::LINE_AA);
            a_h += 20;
        }
        a_w += text.length() * 13;
    }
    cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
    cv::Mat result;
    cv::addWeighted(img, 1.0, layer, 1.0, 0, result);
    // cv::imwrite(output_path, result, {cv::IMWRITE_PNG_COMPRESSION, 0});
    return result;
}

bool can_place_text(int x, int y, const cv::Mat& occupied, const cv::Size& text_size, const cv::Mat& edges) {
    if (x + text_size.width >= edges.cols || y + text_size.height >= edges.rows) {
        return false;
    }
    for (int i = 0; i < text_size.height; i++) {
        for (int j = 0; j < text_size.width; j++) {
            if (occupied.at<uchar>(y + i, x + j) != 0) {
                return false;
            }
        }
    }
    return true;
}

void occupy_region(int x, int y, cv::Mat& occupied, const cv::Size& text_size) {
    for (int i = 0; i < text_size.height; i++) {
        for (int j = 0; j < text_size.width; j++) {
            occupied.at<uchar>(y + i, x + j) = 255;
        }
    }
}

// 边缘水印
// void edge_watermark(const std::string& image_path, const std::string& output_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255), double gain = 0.2) {
cv::Mat edge_watermark_lib(const std::string& image_path, const std::string& text, const cv::Scalar& font_color = cv::Scalar(255, 255, 255), double gain = 0.2) {
    cv::Mat image = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    int h = image.rows;
    int w = image.cols;
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::Mat edges;
    cv::Canny(gray, edges, 100, 200);
    cv::Mat occupied = cv::Mat::zeros(edges.size(), CV_8UC1);
    cv::Mat layer(h, w, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::Point text_position;
    cv::Size text_size(7 * text.length(), 10);
    for (int y = 0; y < edges.rows; y++) {
        for (int x = 0; x < edges.cols; x++) {
            if (edges.at<uchar>(y, x) != 0 && can_place_text(x, y, occupied, text_size, edges)) {
                text_position = cv::Point(x, y);
                cv::putText(layer, text, text_position, cv::FONT_HERSHEY_SIMPLEX, 0.5, font_color, 1, cv::LINE_AA);
                occupy_region(x, y, occupied, text_size);
            }
        }
    }
    cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);
    cv::Mat result;
    cv::addWeighted(image, 1.0, layer, gain, 0, result);
    // cv::imwrite(output_path, result, {cv::IMWRITE_PNG_COMPRESSION, 0});
    return result;
}

std::string cvobj_to_base64(const cv::Mat& opencv_img) {
    if (opencv_img.empty()) {
        std::cerr << "Error loading Image" << std::endl;
        return "error";
    }
    std::vector<uchar> buffer;
    cv::imencode(".png", opencv_img, buffer);
    return base64_encode(buffer.data(), buffer.size());
}

cv::Scalar string_to_scalar(const std::string& input) {
    std::istringstream iss(input);
    std::string token;
    std::vector<int> values;

    // Tokenize the input string based on commas
    while (std::getline(iss, token, ',')) {
        try {
            // Convert each token to an integer
            values.push_back(std::stoi(token));
        } catch (const std::exception& e) {
            return false;
        }
    }
    try {
        return cv::Scalar(values[0], values[1], values[2]);
    } catch (const std::exception& e) { 
        return false;
    }

}

// wasm 对外扩展
std::string simple_watermark_main(const std::string& fileName, const std::string& text, const std::string& colorData) {
    std::string result = cvobj_to_base64(simple_watermark_lib(fileName, text, string_to_scalar(colorData)));
    return result;
}

std::string full_screen_watermark_main(const std::string& fileName, const std::string& text, const std::string& colorData) {
    std::string result = cvobj_to_base64(full_screen_watermark_lib(fileName, text, string_to_scalar(colorData)));
    return result;
}

std::string edge_watermark_main(const std::string& fileName, const std::string& text, const std::string& colorData, double gain) {
    std::string result = cvobj_to_base64(edge_watermark_lib(fileName, text, string_to_scalar(colorData), gain));
    return result;
}

EMSCRIPTEN_BINDINGS(recognizer)
{
    constant("version", version);
    constant("opencv_version", opencv_version);
    function("simple_watermark", &simple_watermark_main);
    function("full_screen_watermark", &full_screen_watermark_main);
    function("edge_watermark", &edge_watermark_main);
}