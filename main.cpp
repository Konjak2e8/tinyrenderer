#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 800, height = 800;
Model *model = NULL;

void line(Vec2i t0, Vec2i t1, TGAImage &image, const TGAColor &color)
{
    int x0 = t0.x, y0 = t0.y, x1 = t1.x, y1 = t1.y;
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        std::swap(x0, y0), std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1)
    {
        std::swap(x0, x1), std::swap(y0, y1);
    }
    int dx = x1 - x0, dy = y1 - y0;
    int derror2 = std::abs(dy) * 2, error2 = 0, y = y0;

    for (int x = x0; x <= x1; x++)
    {
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
        error2 += derror2;
        if (error2 > dx)
        {
            y += (y1 > y0 ? 1 : -1); // accumulated error > 0.5
            error2 -= dx * 2;
        }
    }
}

// 求三角形重心坐标
Vec3f barycentric(Vec3f *pts, Vec3f P)
{
    float xa = pts[0].x, ya = pts[0].y, xb = pts[1].x, yb = pts[1].y, xc = pts[2].x, yc = pts[2].y;
    float x = P.x, y = P.y;

    float gamma = ((ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya) / ((ya - yb) * xc + (xb - xa) * yc + xa * yb - xb * ya);
    float beta = ((ya - yc) * x + (xc - xa) * y + xa * yc - xc * ya) / ((ya - yc) * xb + (xc - xa) * yb + xa * yc - xc * ya);
    float alpha = 1. - gamma - beta;

    if (std::abs(gamma) > 1e-2)
        return Vec3f(alpha, beta, gamma);
    return Vec3f(-1, 1, 1);
}

Vec3f world2screen(const Vec3f &v)
{
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

template <class T>
T min(T x, T y, T z)
{
    return std::min(std::min(x, y), z);
}

template <class T>
T max(T x, T y, T z)
{
    return std::max(std::max(x, y), z);
}

void triangle(Vec3f *pts, Vec2i *uvs, float *zBuffer, TGAImage &image, float intensity)
{
    int min_x = std::floor(min(pts[0].x, pts[1].x, pts[2].x));
    int max_x = std::ceil(max(pts[0].x, pts[1].x, pts[2].x));
    int min_y = std::floor(min(pts[0].y, pts[1].y, pts[2].y));
    int max_y = std::ceil(max(pts[0].y, pts[1].y, pts[2].y));

    for (int i = min_x; i <= max_x; i++)
    {
        for (int j = min_y; j <= max_y; j++)
        {
            Vec3f P(i, j, 0);
            Vec2i uvP;
            Vec3f baryCoord = barycentric(pts, P);
            if (baryCoord.x < -0.01 || baryCoord.y < -0.01 || baryCoord.z < -0.01)
                continue;
            float z_interpolation = baryCoord.x * pts[0].z + baryCoord.y * pts[1].z + baryCoord.z * pts[2].z;
            uvP = uvs[0] * baryCoord.x + uvs[1] * baryCoord.y + uvs[2] * baryCoord.z;
            if (z_interpolation > zBuffer[static_cast<int>(P.x + P.y * width)])
            {
                zBuffer[static_cast<int>(i + j * width)] = z_interpolation;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
            }
        }
    }
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);

    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }

    float *zBuffer = new float[width * height];

    for (int i = 0; i < width * height; i++)
    {
        zBuffer[i] = -std::numeric_limits<float>::max();
    }

    Vec3f light_dir(0, 0, -1); // light direction
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f screenCoords[3];
        Vec3f worldCoords[3];
        for (int j = 0; j < 3; j++)
        {
            worldCoords[j] = model->vert(face[j]);
            screenCoords[j] = world2screen(worldCoords[j]); // transform to screen coordinates
        }
        Vec3f normal = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]); // triangle normal vector
        normal.normalize();
        float intensity = normal * light_dir;
        if (intensity > 0) // backface culling
        {
            Vec2i uv[3];
            for (int j = 0; j < 3; j++)
                uv[j] = model->uvmap(i, j);
            triangle(screenCoords, uv, zBuffer, image, intensity);
        }
    }

    image.flip_vertically(); // have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
