#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
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

    float div = 1.0 / static_cast<float>(x1 - x0);
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

int main(int argc, char **argv)
{
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.) * width / 2.;
            int y0 = (v0.y + 1.) * height / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y1 = (v1.y + 1.) * height / 2.;
            line(Vec2i(x0, y0), Vec2i(x1, y1), image, white);
        }
    }
    image.flip_vertically(); // have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
