//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include "global.hpp"

enum MaterialType {
    DIFFUSE,
    MICROFACET
};

class Material {


private:
    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) {
            cosi = -cosi;
        } else {
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        } else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

public: // static member function
    // Compute reflection direction
    static Vector3f reflect(const Vector3f &I, const Vector3f &N) {
        // return I - 2 * dotProduct(I, N) * N;
        // assume dot(I,N)>0
        return -I + 2 * dotProduct(I, N) * N;
    }

    static Vector3f toWorld(const Vector3f &a, const Vector3f &N) {
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)) {
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
        } else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }

public:
    MaterialType m_type;
    //Vector3f m_color;
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    //Texture tex;

    inline Material(MaterialType t = DIFFUSE, Vector3f e = Vector3f(0, 0, 0));
    inline MaterialType getType();
    //inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline std::pair<Vector3f, float> sample(const Vector3f &wi, const Vector3f &N);
    // given a ray, calculate the PdF of this ray, deprecated
    // inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
};

Material::Material(MaterialType t, Vector3f e) {
    m_type = t;
    //m_color = c;
    m_emission = e;
}

MaterialType Material::getType() { return m_type; }
///Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() { return m_emission; }
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON)
        return true;
    else
        return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


inline std::pair<Vector3f, float> random_sample(const Vector3f &wi, const Vector3f &N) {
    // uniform sample on the hemisphere
    float x_1 = get_random_float(), x_2 = get_random_float();
    float z = std::fabs(1.0f - x_1); //2.0f * x_1); z \in (0,1]
    float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
    Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
    return std::make_pair(Material::toWorld(localRay, N), 0.5f / M_PI);
}

// 重要性采样的范围，实际上是弧度制，范围应在[0, Pi/2)选取
#define SAMPLE_ANGLE 0.525f

// 重要性采样的概率，设为(1.f - cosf(SAMPLE_ANGLE))则大致和均匀随机采样没有区别
// 有`SAMPLE_INPORTANCE`的概率在镜面反射射线方向`SAMPLE_ANGLE`附近采样
#define SAMPLE_INPORTANCE 0.3f

inline std::pair<Vector3f, float> importance_sample(const Vector3f &wi, const Vector3f &N) {
    const float angle = SAMPLE_ANGLE;

    // SAMPLE_INPORTANCE 概率在附近采样
    float x_1 = get_random_float();
    float z;
    bool near_sample = x_1 < SAMPLE_INPORTANCE;
    if (near_sample) {
        // 附近采样
        x_1 /= SAMPLE_INPORTANCE;        // cast x_1 to `[0,1)`
        z = 1 - x_1 * (1 - cosf(angle)); // z \in (cosf(angle), 1]
    } else {
        // 远处采样
        x_1 = (1 - x_1) / (1.f - SAMPLE_INPORTANCE); // cast x_1 to `(0,1]`
        z = cosf(angle) * x_1;                       // z \in (0, cosf(angle)]
    }
    float r = sqrtf(1 - z * z);
    float phi = 2 * M_PI * get_random_float();
    auto ans = Material::toWorld(Vector3f(r * cosf(phi), r * sinf(phi), z), Material::reflect(wi, N));

    return std::make_pair((dotProduct(ans, N) < 0.f) ? -ans : ans, near_sample ? (SAMPLE_INPORTANCE * 0.5f * M_1_PI / (1 - cosf(angle))) : ((1.f - SAMPLE_INPORTANCE) * 0.5f * M_1_PI / cosf(angle)));
}

std::pair<Vector3f, float> Material::sample(const Vector3f &wi, const Vector3f &N) {
    switch (m_type) {
        case DIFFUSE: {
            // 一般随机采样
            return random_sample(wi, N);
        }
        case MICROFACET: {
            // 重要性采样
            // assumptions: wi, N are normalized
            return importance_sample(wi, N);

            // 一般随机采样
            // return random_sample(wi, N);
        }
    }
    throw std::exception();
}

// deprecated
//float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
//    //wi 是被采样出来的
//    switch (m_type) {
//        case DIFFUSE: {
//            // uniform sample probability 1 / (2 * PI)
//            if (dotProduct(wo, N) > 0.0f)
//                return 0.5f / M_PI;
//            else
//                return 0.0f;
//            // break;
//        }
//        case MICROFACET: {
//            if (dotProduct(wo, N) <= 0.f) return 0.f;
//            auto ct = acosf(dotProduct(wi, N));
//            const float angle = SAMPLE_ANGLE;
//
//            auto wii = normalize(wi);
//            auto refl = reflect(wo, N);
//            if (std::abs(dotProduct(wii, refl)) > cosf(angle))
//                return SAMPLE_INPORTANCE * 0.5f * M_1_PI / (1 - cosf(angle));
//            return (1.f - SAMPLE_INPORTANCE) * 0.5f * M_1_PI / cosf(angle);
//        }
//    }
//    throw std::exception();
//}

// change these to see other microfacet materials

#define alpha2 0.04f
#define IOR 1.85f

#ifndef _WIN32
constexpr
#else 
inline
#endif // !_WIN32
float lam(const float &th) {
    return (-1.f + sqrtf(1.f + alpha2 * (1.f / (th * th) - 1.f))) / 2.0f;
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
    switch (m_type) {
        case DIFFUSE: {
            // calculate the contribution of diffuse model
            float cosalpha = dotProduct(N, wo);
            if (cosalpha > 0.0f) {
                Vector3f diffuse = Kd / M_PI;
                return diffuse;
            }
            return {0.0f};
        }
        case MICROFACET: {
            if (dotProduct(N, wo) <= 0.f || dotProduct(N, wi) <= 0.f) return {0.f};
            auto h = normalize(normalize(wi) + normalize(wo));

            // evaluate cos^4, tan^2
            auto costh4rv = dotProduct(h, normalize(N));
            costh4rv *= costh4rv;
            auto tanth2 = 1.f / costh4rv - 1.f;
            costh4rv *= costh4rv;

            auto idot = dotProduct(normalize(wi), normalize(N));
            auto wdot = dotProduct(normalize(wo), normalize(N));
            // to evaluate:
            // D(h) = e^(-tan^2(\theta_h)/alpha^2)/(pi*alpha^2*cos^4 \theta_h)
            // ans = F(i,h) G(i,o,h) D(h)/(4(n,i)(n,o))

            float mask = 1 / ((1 + lam(idot)) * (1 + lam(wdot)));
            float dh = M_1_PI * costh4rv * expf(-tanth2 / alpha2) / alpha2;
            float fres;
            fresnel(-wi, N, IOR, fres);
            float kd = 1 - fres;
            float ans = fres * dh * mask / std::max(EPSILON, 4 * idot * wdot);

            return Ks * ans + Kd * kd * M_1_PI;
        }
    }
    throw std::exception();
}

#endif //RAYTRACING_MATERIAL_H
