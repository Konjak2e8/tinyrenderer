#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

// 模型类
class Model
{
private:
	std::vector<Vec3f> verts_;				// 顶点集
	std::vector<std::vector<Vec3i>> faces_; // 面集
	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
	TGAImage diffuseMap_;
	void loadTexture(const std::string &filename, const char *suffix, TGAImage &image);

public:
	Model(const char *filename); // 导入.obj模型
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int idx);
	std::vector<int> face(int idx);
	Vec2i uv(int iface, int nverts);
	TGAColor diffuse(Vec2i uv);
};

#endif //__MODEL_H__