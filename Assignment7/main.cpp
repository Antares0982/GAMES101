#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv) {
    // Change the definition here to change resolution
    Scene scene(784, 784);

    Material *red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material *green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material *white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material *light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
    light->Kd = Vector3f(0.65f);

    Material *microfacet = new Material(MICROFACET, Vector3f(0.0f));
    microfacet->Ks = Vector3f(0.4f);
    // microfacet->Kd = Vector3f(0.1f);
    // microfacet->Kd = Vector3f(0.2f, 0.2f, 0.05f);
    // pink one
    microfacet->Kd = Vector3f(1.f, 105.f / 255.f, 180.f / 255.f);

    Material *microfacet_2 = new Material(MICROFACET, Vector3f(0.0f));
    microfacet_2->Ks = Vector3f(0.4f);
    // microfacet_2->Kd = Vector3f(0.1f);
    //microfacet_2->Kd = Vector3f(0.05f, 0.05f, 0.2f);
    // blue one
    microfacet_2->Kd = Vector3f(0.f, 191.f / 255.f, 1.f);

    Material *microfacet_3 = new Material(MICROFACET, Vector3f(0.0f));
    microfacet_3->Ks = Vector3f(0.4f);
    // golden one
    microfacet_3->Kd = Vector3f(1.f, 215.f / 255.f, 0.f);

    #if WIN32 || _WIN32
    MeshTriangle floor("..\\..\\..\\models\\cornellbox\\floor.obj", white);
    // MeshTriangle shortbox("..\\..\\..\\models\\cornellbox\\shortbox.obj", white);
    // MeshTriangle tallbox("..\\..\\..\\models\\cornellbox\\tallbox.obj", white);
    MeshTriangle left("..\\..\\..\\models\\cornellbox\\left.obj", red);
    MeshTriangle right("..\\..\\..\\models\\cornellbox\\right.obj", green);
    MeshTriangle light_("..\\..\\..\\models\\cornellbox\\light.obj", light);
    #else
    MeshTriangle floor("../models/cornellbox/floor.obj", white);
    // MeshTriangle shortbox("../models/cornellbox/shortbox.obj", white);
    // MeshTriangle tallbox("../models/cornellbox/tallbox.obj", white);
    MeshTriangle left("../models/cornellbox/left.obj", red);
    MeshTriangle right("../models/cornellbox/right.obj", green);
    MeshTriangle light_("../models/cornellbox/light.obj", light);
    #endif

    Sphere sphere_l(Vector3f(150, 100, 400), 100.0f, microfacet);
    Sphere sphere_r(Vector3f(400, 100, 400), 100.0f, microfacet_2);
    Sphere sphere_m(Vector3f(275, 100, 400 - 157), 100.0f, microfacet_3);

    scene.Add(&floor);
    //    scene.Add(&shortbox);
    //    scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.Add(&sphere_l);
    scene.Add(&sphere_r);
    scene.Add(&sphere_m);

    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    std::cout << "Average time spent per sample: " << (float) std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() / spp << " seconds\n";
    return 0;
}
