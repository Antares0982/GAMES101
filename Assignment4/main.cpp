#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

#include <iostream>
#include <opencv2/opencv.hpp>

#define CONTROL_PT_NUM 6

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) {
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < CONTROL_PT_NUM) {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) {
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

__attribute__((unused)) cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, const float &t) {
    // TODO: Implement de Casteljau's algorithm
    // Useless
    return cv::Point2f();
}

// return C_n^i
int binar(const int &n, const int &i) {
    if (i > n / 2) return binar(n, n - i);
    int ans = 1;
    for (int j = n; j > n - i; --j) ans *= j;
    for (int j = 2; j <= i; ++j) ans /= j;
    return ans;
}

// draw bezier curve
void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) {
    int *coe = new int[control_points.size()];
    for (int i = 0; i < (control_points.size() + 1) / 2; ++i) {
        coe[i] = binar(control_points.size() - 1, i);
        coe[control_points.size() - 1 - i] = coe[i];
    }
    int directions[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};
    for (float t = 0.0f; t <= 1.0f; t += 0.001f) {
        cv::Point2f point = {0, 0};
        for (int i = 0; i < control_points.size(); ++i) {
            point += coe[i] * (i ? std::pow(t, i) : 1.0f) * ((control_points.size() - 1 - i) ? std::pow(1 - t, control_points.size() - 1 - i) : 1) * control_points[i];
        }
        // antialiasing
        auto ptx = int(point.x);
        auto pty = int(point.y);
        for (auto dire : directions) {
            auto xx = ptx + dire[0];
            auto yy = pty + dire[1];
            float dist = std::pow(std::abs(xx - point.x), 2.0f) + std::pow(std::abs(yy - point.y), 2.0f);
            dist /= 2;
            window.at<cv::Vec3b>(yy, xx)[1] = std::max((unsigned char) ((1 - dist) * (1 - dist) * 255.0f), window.at<cv::Vec3b>(yy, xx)[1]);
        }
        // not antialiasing
        // window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }

    delete[] coe;
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.
}

int main() {
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) {
        for (auto &point : control_points) {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == CONTROL_PT_NUM) {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}

#pragma clang diagnostic pop
