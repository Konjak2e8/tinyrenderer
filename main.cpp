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
Vec3f barycentric(std::vector<Vec2i> &pts, Vec2i P)
{
    int xa = pts[0].x, ya = pts[0].y, xb = pts[1].x, yb = pts[1].y, xc = pts[2].x, yc = pts[2].y;
    int x = P.x, y = P.y;

    float gamma = static_cast<float>((ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya) / static_cast<float>((ya - yb) * xc + (xb - xa) * yc + xa * yb - xb * ya);
    float beta = static_cast<float>((ya - yc) * x + (xc - xa) * y + xa * yc - xc * ya) / static_cast<float>((ya - yc) * xb + (xc - xa) * yb + xa * yc - xc * ya);
    float alpha = 1 - gamma - beta;

    return Vec3f(alpha, beta, gamma);
}

void triangle(std::vector<Vec2i> &pts, TGAImage &image, const TGAColor &color)
{
    Vec2i bboxMin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxMax(0, 0);

    for (auto pt : pts)
    {
        bboxMin.x = std::min(bboxMin.x, pt.x);
        bboxMin.y = std::min(bboxMin.y, pt.y);
        bboxMax.x = std::max(bboxMax.x, pt.x);
        bboxMax.y = std::max(bboxMax.y, pt.y);
    }

    for (int i = bboxMin.x; i <= bboxMax.x; i++)
    {
        for (int j = bboxMin.y; j <= bboxMax.y; j++)
        {
            Vec2i p(i, j);
            Vec3f bary = barycentric(pts, p);
            if (bary.x < -0.1 || bary.y < -0.1 || bary.z < -0.1)
                continue;
            image.set(p.x, p.y, color);
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

    Vec3f light_dir(0, 0, -1); // light direction
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        std::vector<Vec2i> screenCoords(3);
        std::vector<Vec3f> worldCoords(3);
        for (int j = 0; j < 3; j++)
        {
            worldCoords[j] = model->vert(face[j]);
            screenCoords[j] = Vec2i((worldCoords[j].x + 1.) * width / 2., (worldCoords[j].y + 1.) * height / 2.); // transform to screen coordinates
        }
        Vec3f normal = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]); // triangle normal vector
        normal.normalize();
        float intensity = normal * light_dir;
        if (intensity > 0) // backface culling
            triangle(screenCoords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    }

    image.flip_vertically(); // have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
