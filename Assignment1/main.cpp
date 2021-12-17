#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr float MY_PI = 3.1415926f;

Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    Eigen::Matrix4f ans;
    float c = std::cos(angle / 180.0 * MY_PI);
    float s = std::sin(angle / 180.0 * MY_PI);
    float c_m1 = 1 - c;
    float x = axis.x();
    float y = axis.y();
    float z = axis.z();
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    ans << c + c_m1 * x * x, c_m1 * xy - c * z, c_m1 * zx + s * y, 0,
            c_m1 * xy + s * z, c + c_m1 * y * y, c_m1 * yz - s * x, 0,
            c_m1 * zx - s * y, c_m1 * yz + s * x, c + c_m1 * z * z, 0,
            0, 0, 0, 1;
    return ans;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
            0, 1, 0, -eye_pos[1],
            0, 0, 1, -eye_pos[2],
            0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    float c = std::cos(rotation_angle / 180.0f * MY_PI);
    float s = std::sin(rotation_angle / 180.0f * MY_PI);
    model << c, -s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar) {
    float top = tan(eye_fov / 2.0f * MY_PI / 180.0f);
    float right = top * aspect_ratio;

    Eigen::Matrix4f projection;

    projection << -1 / right, 0, 0, 0,
            0, -1 / top, 0, 0,
            0, 0, zNear + zFar, -zNear * zFar,
            0, 0, 1, 0;

    return projection;
}

int main(int argc, const char **argv) {
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4)
            filename = std::string(argv[3]);
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2},
                                     {0, 2, -2},
                                     {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        if (key == 'a') {
            angle += 10;
        } else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
