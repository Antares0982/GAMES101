//
// Created by goksu on 2/25/20.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

#include "Renderer.hpp"
#include "Scene.hpp"
#include <fstream>
#include <mutex>
#include <thread>


#define MAX_THREADS 16
const int spp = 8; // 16

const Scene *sceneptr = nullptr;

struct multithreadargs {
    std::vector<Vector3f> *arg1;
    int arg2;
};

constexpr float deg2rad(const float &deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.000001;

static volatile std::atomic_int progress = 0;
static int totaltask = 0;

void do_render_multi_thread(multithreadargs *arg) {
    auto &&[frameptr, index] = *arg;

    Vector3f eye_pos(278, 273, -800);
    float scale = std::tan(deg2rad(sceneptr->fov * 0.5));
    float imageAspectRatio = sceneptr->width / (float) sceneptr->height;

    const int maxgrid = sceneptr->width * sceneptr->height;
    for (; index < maxgrid; index += MAX_THREADS) {
        int j = index / sceneptr->width;
        int i = index % sceneptr->width;
        float x = (2 * (i + 0.5f) / (float) sceneptr->width - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (j + 0.5f) / (float) sceneptr->height) * scale;

        Vector3f dir = normalize(Vector3f(-x, y, 1));
        for (int k = 0; k < spp; k++) {
            (*frameptr)[index] += sceneptr->castRay(Ray(eye_pos, dir), 0) / spp;
        }
        progress++;
    }
}


// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene &scene) {
    progress = 0;
    totaltask = scene.width * scene.height;
    std::vector<Vector3f> framebuffer(totaltask);

    //    float scale = tan(deg2rad(scene.fov * 0.5));
    //    float imageAspectRatio = scene.width / (float) scene.height;
    //    Vector3f eye_pos(278, 273, -800);
    //    int m = 0;

    // change the spp value to change sample ammount

    std::cout << "SPP: " << spp << "\n";

    std::thread ths[MAX_THREADS];

    multithreadargs arg[MAX_THREADS];

    // initialize args
    progress = 0;
    sceneptr = &scene;
    for (int i = 0; i < MAX_THREADS; ++i) {
        arg[i] = multithreadargs{&framebuffer, i};
        ths[i] = std::thread(do_render_multi_thread, &arg[i]);
        ths[i].detach();
    }

    int progresscount = 0;
    while (progress != totaltask) {
        if (progresscount != progress) {
            progresscount = progress;
            UpdateProgress(progresscount / (float) totaltask);
        }
    }
    // task done
    UpdateProgress(1.f);

    sceneptr = nullptr;

    // save framebuffer to file
    FILE *fp = fopen("binary.ppm", "wb");
    (void) fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}

#pragma clang diagnostic pop
