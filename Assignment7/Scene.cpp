#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const {
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum) {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object *> &objects,
        float &tNear, uint32_t &index, Object **hitObject) {
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
    // TODO Implement Path Tracing Algorithm here
    //    if (depth > maxDepth) return {0, 0, 0};
    Intersection itsec = intersect(ray);
    if (!itsec.happened) return {0, 0, 0};

    if (itsec.m->hasEmission()) { return {1}; }

    float light_pdf;
    Intersection wherelight;
    sampleLight(wherelight, light_pdf);

    Vector3f L_dir{0};

    auto wo = normalize(-ray.direction);
    auto &N = itsec.normal;
    auto ws = normalize(wherelight.coords - itsec.coords);

    if (wherelight.emit.norm()) {
        auto raylight = Ray(itsec.coords, ws);
        Intersection itsectolight = intersect(raylight);
        auto &NN = wherelight.normal;

        if (itsectolight.m && itsectolight.m->hasEmission()) {
            Vector3f ev = itsec.m->eval(wo, ws, N);
            float dominant = itsectolight.distance * itsectolight.distance * light_pdf;
            L_dir = wherelight.emit * ev * std::abs(dotProduct(ws, N)) * std::abs(dotProduct(ws, NN)) / dominant;
        }
    }

    if (get_random_float() > RussianRoulette)
        return L_dir;

    auto wi = itsec.m->sample(wo, N);
    auto rayindir = Ray(itsec.coords, wi);
    Vector3f L_indir = castRay(rayindir, depth + 1) * itsec.m->eval(wi, wo, N) * dotProduct(wi, N) / (itsec.m->pdf(wi, wo, N) * RussianRoulette);

    return L_dir + L_indir;
}

#pragma clang diagnostic pop
