//
// Created by goksu on 4/6/19.
//

#include "rasterizer.hpp"
#include <algorithm>
#include <math.h>
#include <vector>

// Change super-sampling number here
#define SAMPLE_SIZE 2

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions) {
    auto id = get_next_id();
    pos_buf.emplace(id, positions);
    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices) {
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols) {
    auto id = get_next_id();
    col_buf.emplace(id, cols);
    return {id};
}

auto inline to_vec4(const Eigen::Vector3f &v3, const float &w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static inline bool insideTriangle(const float &x, const float &y, const Vector3f *_v) {
    Vector3f p;
    p << x, y, 1;
    bool sign = (_v[0] - _v[2]).cross(p - _v[2])[2] > 0;
    for (int i = 0; i < 2; ++i)
        if (sign ^ ((_v[i + 1] - _v[i]).cross(p - _v[i])[2] > 0)) return false;
    return true;
}

static inline std::tuple<float, float, float> computeBarycentric2D(const float &x, const float &y, const Vector3f *v) {
    float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
               (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() -
                v[2].x() * v[1].y());
    float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
               (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() -
                v[0].x() * v[2].y());
    float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
               (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() -
                v[1].x() * v[0].y());
    return {c1, c2, c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type) {
    auto &buf = pos_buf[pos_buffer.pos_id];
    auto &ind = ind_buf[ind_buffer.ind_id];
    auto &col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto &i : ind) {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)};
        //Homogeneous division
        for (auto &vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto &vert : v) {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i) {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle &t) {
    auto v = t.toVector4();

    float l = t.v[0][0], r = t.v[0][0], u = t.v[0][1], d = t.v[0][1];
    for (int i = 1; i < 3; ++i) {
        l = std::min(t.v[i][0], l);
        r = std::max(t.v[i][0], r);
        d = std::min(t.v[i][1], d);
        u = std::max(t.v[i][1], u);
    }

    const int rr = std::min(width - 1, int(std::ceil(r)));
    const int uu = std::min(height - 1, int(std::ceil(u)));

    const float gapsize = 1.0f / SAMPLE_SIZE;
    const float startsat = gapsize / 2.0f;

    for (int x = std::max(0, int(std::floor(l))); x <= rr; ++x) {
        for (int y = std::max(0, int(std::floor(d))); y <= uu; ++y) {
            const int depth_index_start = (x + width * (height - y - 1)) * SAMPLE_SIZE * SAMPLE_SIZE;
            Matrix<float, 3, 1> pixelcolor{0, 0, 0};

            for (int i = 0; i < SAMPLE_SIZE; ++i) {
                for (int j = 0; j < SAMPLE_SIZE; ++j) {
                    const int depth_index = depth_index_start + i * SAMPLE_SIZE + j;

                    if (insideTriangle(x + startsat + i * gapsize, y + startsat + j * gapsize, t.v)) {
                        auto [alpha, beta, gamma] = computeBarycentric2D(x + startsat + i * gapsize, y + startsat + j * gapsize, t.v);
                        float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());

                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;

                        if (depth_buf[depth_index] > -z_interpolated) {
                            depth_buf[depth_index] = -z_interpolated;
                            frame_sample_buf[depth_index] = t.getColor();
                        }
                    }

                    pixelcolor += frame_sample_buf[depth_index];
                }
            }

            pixelcolor /= (SAMPLE_SIZE * SAMPLE_SIZE);
            set_pixel({x, y, 1}, pixelcolor);
        }
    }
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // If so, use the following code to get the interpolated z value.

    //set_pixel({x, y, 1}, t.getColor());
    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f &m) {
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f &v) {
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f &p) {
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff) {
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(frame_sample_buf.begin(), frame_sample_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
    frame_buf.resize(w * h);
    frame_sample_buf.resize(w * h * SAMPLE_SIZE * SAMPLE_SIZE);
    depth_buf.resize(w * h * SAMPLE_SIZE * SAMPLE_SIZE);
}

int rst::rasterizer::get_index(int x, int y) {
    return (height - 1 - y) * width + x;
}

Eigen::Vector3f rst::rasterizer::get_pixel(const Eigen::Vector3f &point) {
    return frame_buf[(height - 1 - point.y()) * width + point.x()];
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f &point, const Eigen::Vector3f &color) {
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}

// clang-format on
