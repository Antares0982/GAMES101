#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
//
// Created by LEI XU on 4/27/19.
//

#include "Texture.hpp"


inline Eigen::Vector3f Texture::gettexel(const int &row, const int &col) {
    auto color = image_data.at<cv::Vec3b>(row, col);
    return {color[0], color[1], color[2]};
}


Eigen::Vector3f Texture::getColorBilinear(float u, float v) {
    auto u_img = int(u * width);
    auto v_img = int((1 - v) * height);
    auto u_img_n = (u_img == width - 1) ? u_img : u_img + 1;
    auto v_img_n = (v_img == height - 1) ? v_img : v_img + 1;
    float dx = v_img != v_img_n ? (1 - v) * height - v_img : 0;
    float dy = u_img != u_img_n ? u * width - u_img : 0;

    auto color00 = gettexel(v_img, u_img);
    auto color10 = gettexel(v_img_n, u_img);

    auto u0 = color00 + dx * (color10 - color00);

    if (dy) {
        auto color01 = gettexel(v_img, u_img_n);
        auto color11 = gettexel(v_img_n, u_img_n);
        auto u1 = color01 + dx * (color11 - color01);
        return u0 + dy * (u1 - u0);
    }

    return u0;
}

#pragma clang diagnostic pop
