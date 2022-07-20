/*
 * Author: geodoer
 * Time: 2022/07/19
 * Desc: 遍历OSGB下的Geometry，并组织成三角网，并输出成OBJ格式
 */
#include <vector>
#include <array>
#include <osg/Geometry>
#include <osg/NodeVisitor>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Tessellator>

using precision = double;
using vec2 = std::array<precision, 2>;
using vec3 = std::array<precision, 3>;
using vec4 = std::array<precision, 4>;

struct TriangleMesh
{
	std::vector<vec3>          vertexs; //顶点
	std::vector<vec3>          normals; //法向量（OSG提供多种法向量绑定方式，不一定是顶点法线，请注意）
	std::vector<vec2>          uvs;     //uv
	std::vector<vec4>          colors;  //颜色（OSG提供多种颜色绑定方式，不一定是顶点颜色，请注意）
	std::vector<unsigned int>  indices; //三角面的索引

	//导出成OBJ进行查看，这里只处理的顶点和面
	void exportObj(const std::string& path) const
	{
		std::ios_base::sync_with_stdio(false);
		std::cin.tie(0);
		std::ofstream out(path);

		for(auto& p : vertexs)
		{
			out << "v " << p[0]
				<< " " << p[1]
				<< " " << p[2]
				<< std::endl;
		}
		out.flush();

		auto faceSize = indices.size() / 3;
		for(decltype(faceSize) faceIdx{0}; faceIdx < faceSize; ++faceIdx)
		{
			auto begin = faceIdx * 3;
			out << "f " << indices[begin] + 1
				<< " " << indices[begin + 1] + 1
				<< " " << indices[begin + 2] + 1
				<< std::endl;
		}

		out.flush();
		out.close();
	}
};

class MeshVisitor : public osg::NodeVisitor
{
public:
	MeshVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}

	void apply(osg::Geode& geode) override
	{
		auto size = geode.getNumDrawables();

		for(decltype(size) i{0}; i<size; ++i)
		{
			auto drawable = geode.getDrawable(i);
			auto geometry = dynamic_cast<osg::Geometry*>(drawable);
			if(!geometry)
			{
				continue;
			}

			auto mesh = as(*geometry); //转成你的数据结构

			{//简单导出成OBJ看看
				std::string path = DATA_PATH + std::to_string(i) + ".obj";
				mesh->exportObj(path);
				std::cout << "保存:" << path << std::endl;
			}

			//TODO:加上你的处理
			//...

			//
			//塞回去
			//
			//[方法] 直接设置
			//问题：Node里可能有状态关联，直接设置，可能会出现问题
			//auto newGeometry = as(*mesh);
			//geode.setDrawable(i, newGeometry);
			//[方法] 原地替换（建议）
			as(*mesh, *geometry);

			delete mesh;
		}
	}

protected:
	static TriangleMesh* as(const osg::Geometry& geometry)
	{
		//
		//三角化（效果不好，不建议使用）
		//
		//osg::ref_ptr<osgUtil::Tessellator>tscx = new osgUtil::Tessellator();
		//tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
		//tscx->setBoundaryOnly(false);   //设置是否只显示边界
		//tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);
		//tscx->retessellatePolygons(geometry);

		//
		//geometry -> trianglemesh
		//
		TriangleMesh* pMesh = new TriangleMesh;
		auto& mesh = *pMesh;

		//顶点
		if (const auto vertex_array = dynamic_cast<const osg::Vec3Array*>(geometry.getVertexArray()))
		{
			for (auto i = 0; i < vertex_array->size(); ++i)
			{
				osg::Vec3 xyz = vertex_array->at(i);
				mesh.vertexs.emplace_back(vec3{ xyz[0], xyz[1], xyz[2] });
			}

			if(auto size = geometry.getVertexAttribArrayList().size())
			{
				std::cerr << "此数据有" << size <<"顶点属性，暂未处理顶点属性，请注意！（" << geometry.getNormalBinding() << std::endl;
			}
		}

		//法线数组
		if (const auto normal_array = dynamic_cast<const osg::Vec3Array*>(geometry.getNormalArray()))
		{
			for (auto i = 0; i < normal_array->size(); ++i)
			{
				auto xyz = normal_array->at(i);
				mesh.normals.emplace_back(vec3{ xyz[0], xyz[1], xyz[2] });
			}

			if(geometry.getNormalBinding() != osg::Geometry::BIND_PER_VERTEX) //顶点法向
			{
				std::cerr << "法向量不是逐顶点的，要注意！（" << geometry.getNormalBinding() << std::endl;
			}
		}

		//颜色数组
		if (const auto color_array = dynamic_cast<const osg::Vec4Array*>(geometry.getColorArray()))
		{
			for (auto i = 0; i < color_array->size(); ++i)
			{
				auto rgba = color_array->at(i);
				mesh.colors.emplace_back(vec4{ rgba[0], rgba[1], rgba[2],rgba[3] });
			}

			if(geometry.getColorBinding() != osg::Geometry::BIND_PER_VERTEX)
			{
				std::cerr << "颜色不是逐顶点的，要注意！（" << geometry.getColorBinding() << std::endl;
			}
		}

		//uv
		if (const auto texcoor_array = dynamic_cast<const osg::Vec2Array*>(geometry.getTexCoordArray(0)))
		{
			for (auto i = 0; i < texcoor_array->size(); ++i)
			{
				auto& coor = texcoor_array->at(i);
				mesh.uvs.emplace_back(vec2{ coor[0], coor[1] });
			}

			if (const int index = geometry.getTexCoordArrayList().size() > 1)
			{
				std::cerr << "[Warning] 纹理坐标数组的个数" << index << "，当前只能处理第一个！" << std::endl;
			}
		}

		//索引
		for (auto& primitiveSet : geometry.getPrimitiveSetList())
		{
			const auto mode = static_cast<osg::PrimitiveSet::Mode>(primitiveSet->getMode());

			switch (mode)
			{
			case osg::PrimitiveSet::TRIANGLES:
				{
					auto indexNum = primitiveSet->getNumIndices();

					for (decltype(indexNum) i = 0; i < indexNum; ++i)
					{
						mesh.indices.emplace_back(primitiveSet->index(i));
					}
					break;
				}
			case osg::PrimitiveSet::TRIANGLE_STRIP: //三角形条状
				{
					auto indexNum = primitiveSet->getNumIndices();

					for (decltype(indexNum) i = 0; i < indexNum - 2; i++)
					{
						//此处索引为何与基数偶数有关
						if (i % 2 == 0)
						{
							mesh.indices.emplace_back(primitiveSet->index(i));
							mesh.indices.emplace_back(primitiveSet->index(i + 1));
							mesh.indices.emplace_back(primitiveSet->index(i + 2));
						}
						else
						{
							mesh.indices.emplace_back(primitiveSet->index(i));
							mesh.indices.emplace_back(primitiveSet->index(i + 2));
							mesh.indices.emplace_back(primitiveSet->index(i + 1));
						}
					}
					break;
				}
			case osg::PrimitiveSet::TRIANGLE_FAN:   //三角形_扇形
				{
					auto indexNum = primitiveSet->getNumIndices();

					for (decltype(indexNum) k = 1; k < indexNum - 1; ++k)
					{
						mesh.indices.emplace_back(primitiveSet->index(0));
						mesh.indices.emplace_back(primitiveSet->index(k));
						mesh.indices.emplace_back(primitiveSet->index(k + 1));
					}
					break;
				}
			case osg::PrimitiveSet::POINTS:
			case osg::PrimitiveSet::LINES:
			case osg::PrimitiveSet::LINE_STRIP:
			case osg::PrimitiveSet::LINE_LOOP:
			case osg::PrimitiveSet::QUADS:
			case osg::PrimitiveSet::QUAD_STRIP:
			case osg::PrimitiveSet::POLYGON:
			case osg::PrimitiveSet::LINES_ADJACENCY:
			case osg::PrimitiveSet::LINE_STRIP_ADJACENCY:
			case osg::PrimitiveSet::TRIANGLES_ADJACENCY:
			case osg::PrimitiveSet::TRIANGLE_STRIP_ADJACENCY:
			case osg::PrimitiveSet::PATCHES:
			default:
				{
					std::cerr << "[Warning] PrimitiveSet出现未处理的类型" << std::endl;
					break;
				}
			}
		}

		return pMesh;
	}
	static osg::Geometry* as(const TriangleMesh& mesh)
	{
		auto geometry = new osg::Geometry();

		//顶点
		if(!mesh.vertexs.empty()) {
			auto vertexs  = new osg::Vec3Array();

			for (auto& v : mesh.vertexs)
			{
				vertexs->push_back(osg::Vec3(v[0], v[1], v[2]));
			}

			geometry->setVertexArray(vertexs);
		}
		//法向量
		if(!mesh.normals.empty()) {
			auto normals = new osg::Vec3Array();

			for(auto& n : mesh.normals)
			{
				normals->push_back(osg::Vec3( n[0], n[1], n[2] ));
			}

			geometry->setNormalArray(normals);
		}
		//颜色
		if(!mesh.colors.empty()) {
			auto colors = new osg::Vec4Array();

			for(auto& color : mesh.colors)
			{
				colors->push_back(osg::Vec4( color[0], color[1], color[2], color[3] ));
			}

			geometry->setColorArray(colors);
		}
		//纹理坐标
		if(!mesh.uvs.empty()) {
			auto uvs = new osg::Vec2Array();

			for(auto& uv : mesh.uvs)
			{
				uvs->push_back(osg::Vec2( uv[0], uv[1] ));
			}

			geometry->setTexCoordArray(0, uvs);
		}

		//索引
		if(!mesh.indices.empty())
		{
			auto drawElements = new osg::DrawElementsUShort(osg::DrawArrays::TRIANGLES);

			for(auto& idx : mesh.indices)
			{
				drawElements->addElement(idx);
			}

			geometry->addPrimitiveSet(drawElements);
		}

		return geometry;
	}
	static void as(const TriangleMesh& mesh, osg::Geometry& geometry)
	{
		//顶点
		if (!mesh.vertexs.empty()) 
		{
			auto vertexs = new osg::Vec3Array();

			for (auto& v : mesh.vertexs)
			{
				vertexs->push_back(osg::Vec3(v[0], v[1], v[2]));
			}

			geometry.setVertexArray(vertexs);
		}
		//法向量
		if (!mesh.normals.empty()) {
			auto normals = new osg::Vec3Array();

			for (auto& n : mesh.normals)
			{
				normals->push_back(osg::Vec3(n[0], n[1], n[2]));
			}

			//! 这里要把原先的绑定方式放入
			geometry.setNormalArray(normals, static_cast<osg::Array::Binding>(geometry.getNormalBinding()));
		}
		//颜色
		if (!mesh.colors.empty()) {

			auto colors = new osg::Vec4Array();

			for (auto& color : mesh.colors)
			{
				colors->push_back(osg::Vec4(color[0], color[1], color[2], color[3]));
			}

			//! 这里要把原先的绑定方式放入
			geometry.setColorArray(colors, static_cast<osg::Array::Binding>(geometry.getColorBinding()));
		}
		//纹理坐标
		if (!mesh.uvs.empty()) {
			auto uvs = new osg::Vec2Array();

			for (auto& uv : mesh.uvs)
			{
				uvs->push_back(osg::Vec2(uv[0], uv[1]));
			}

			geometry.setTexCoordArray(0, uvs);
		}

		//索引
		if (!mesh.indices.empty())
		{
			geometry.getPrimitiveSetList().clear();

			auto drawElements = new osg::DrawElementsUShort(osg::DrawArrays::TRIANGLES);

			for (auto& idx : mesh.indices)
			{
				drawElements->addElement(idx);
			}

			geometry.addPrimitiveSet(drawElements);
		}
	}
};

int main(int argc, char** argv)
{
	std::string osgb_path = DATA_PATH "glider.osg";
	std::string osgb_output_path = DATA_PATH "glider.copy.osg";

	osg::Node* root = osgDB::readNodeFile(osgb_path);

	//遍历
	MeshVisitor visitor;
	if (root)
	{
		root->accept(visitor);
	}

	//保存
	osgDB::writeNodeFile(*root, osgb_output_path);

	system("pause");
	return 0;
}
