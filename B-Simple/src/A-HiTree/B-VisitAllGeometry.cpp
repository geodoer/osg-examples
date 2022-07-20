/*
 * Author: geodoer
 * Time: 2022/07/19
 * Desc: ����OSGB�µ�Geometry������֯�����������������OBJ��ʽ
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
	std::vector<vec3>          vertexs; //����
	std::vector<vec3>          normals; //��������OSG�ṩ���ַ������󶨷�ʽ����һ���Ƕ��㷨�ߣ���ע�⣩
	std::vector<vec2>          uvs;     //uv
	std::vector<vec4>          colors;  //��ɫ��OSG�ṩ������ɫ�󶨷�ʽ����һ���Ƕ�����ɫ����ע�⣩
	std::vector<unsigned int>  indices; //�����������

	//������OBJ���в鿴������ֻ����Ķ������
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

			auto mesh = as(*geometry); //ת��������ݽṹ

			{//�򵥵�����OBJ����
				std::string path = DATA_PATH + std::to_string(i) + ".obj";
				mesh->exportObj(path);
				std::cout << "����:" << path << std::endl;
			}

			//TODO:������Ĵ���
			//...

			//
			//����ȥ
			//
			//[����] ֱ������
			//���⣺Node�������״̬������ֱ�����ã����ܻ��������
			//auto newGeometry = as(*mesh);
			//geode.setDrawable(i, newGeometry);
			//[����] ԭ���滻�����飩
			as(*mesh, *geometry);

			delete mesh;
		}
	}

protected:
	static TriangleMesh* as(const osg::Geometry& geometry)
	{
		//
		//���ǻ���Ч�����ã�������ʹ�ã�
		//
		//osg::ref_ptr<osgUtil::Tessellator>tscx = new osgUtil::Tessellator();
		//tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
		//tscx->setBoundaryOnly(false);   //�����Ƿ�ֻ��ʾ�߽�
		//tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);
		//tscx->retessellatePolygons(geometry);

		//
		//geometry -> trianglemesh
		//
		TriangleMesh* pMesh = new TriangleMesh;
		auto& mesh = *pMesh;

		//����
		if (const auto vertex_array = dynamic_cast<const osg::Vec3Array*>(geometry.getVertexArray()))
		{
			for (auto i = 0; i < vertex_array->size(); ++i)
			{
				osg::Vec3 xyz = vertex_array->at(i);
				mesh.vertexs.emplace_back(vec3{ xyz[0], xyz[1], xyz[2] });
			}

			if(auto size = geometry.getVertexAttribArrayList().size())
			{
				std::cerr << "��������" << size <<"�������ԣ���δ���������ԣ���ע�⣡��" << geometry.getNormalBinding() << std::endl;
			}
		}

		//��������
		if (const auto normal_array = dynamic_cast<const osg::Vec3Array*>(geometry.getNormalArray()))
		{
			for (auto i = 0; i < normal_array->size(); ++i)
			{
				auto xyz = normal_array->at(i);
				mesh.normals.emplace_back(vec3{ xyz[0], xyz[1], xyz[2] });
			}

			if(geometry.getNormalBinding() != osg::Geometry::BIND_PER_VERTEX) //���㷨��
			{
				std::cerr << "�����������𶥵�ģ�Ҫע�⣡��" << geometry.getNormalBinding() << std::endl;
			}
		}

		//��ɫ����
		if (const auto color_array = dynamic_cast<const osg::Vec4Array*>(geometry.getColorArray()))
		{
			for (auto i = 0; i < color_array->size(); ++i)
			{
				auto rgba = color_array->at(i);
				mesh.colors.emplace_back(vec4{ rgba[0], rgba[1], rgba[2],rgba[3] });
			}

			if(geometry.getColorBinding() != osg::Geometry::BIND_PER_VERTEX)
			{
				std::cerr << "��ɫ�����𶥵�ģ�Ҫע�⣡��" << geometry.getColorBinding() << std::endl;
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
				std::cerr << "[Warning] ������������ĸ���" << index << "����ǰֻ�ܴ����һ����" << std::endl;
			}
		}

		//����
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
			case osg::PrimitiveSet::TRIANGLE_STRIP: //��������״
				{
					auto indexNum = primitiveSet->getNumIndices();

					for (decltype(indexNum) i = 0; i < indexNum - 2; i++)
					{
						//�˴�����Ϊ�������ż���й�
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
			case osg::PrimitiveSet::TRIANGLE_FAN:   //������_����
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
					std::cerr << "[Warning] PrimitiveSet����δ���������" << std::endl;
					break;
				}
			}
		}

		return pMesh;
	}
	static osg::Geometry* as(const TriangleMesh& mesh)
	{
		auto geometry = new osg::Geometry();

		//����
		if(!mesh.vertexs.empty()) {
			auto vertexs  = new osg::Vec3Array();

			for (auto& v : mesh.vertexs)
			{
				vertexs->push_back(osg::Vec3(v[0], v[1], v[2]));
			}

			geometry->setVertexArray(vertexs);
		}
		//������
		if(!mesh.normals.empty()) {
			auto normals = new osg::Vec3Array();

			for(auto& n : mesh.normals)
			{
				normals->push_back(osg::Vec3( n[0], n[1], n[2] ));
			}

			geometry->setNormalArray(normals);
		}
		//��ɫ
		if(!mesh.colors.empty()) {
			auto colors = new osg::Vec4Array();

			for(auto& color : mesh.colors)
			{
				colors->push_back(osg::Vec4( color[0], color[1], color[2], color[3] ));
			}

			geometry->setColorArray(colors);
		}
		//��������
		if(!mesh.uvs.empty()) {
			auto uvs = new osg::Vec2Array();

			for(auto& uv : mesh.uvs)
			{
				uvs->push_back(osg::Vec2( uv[0], uv[1] ));
			}

			geometry->setTexCoordArray(0, uvs);
		}

		//����
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
		//����
		if (!mesh.vertexs.empty()) 
		{
			auto vertexs = new osg::Vec3Array();

			for (auto& v : mesh.vertexs)
			{
				vertexs->push_back(osg::Vec3(v[0], v[1], v[2]));
			}

			geometry.setVertexArray(vertexs);
		}
		//������
		if (!mesh.normals.empty()) {
			auto normals = new osg::Vec3Array();

			for (auto& n : mesh.normals)
			{
				normals->push_back(osg::Vec3(n[0], n[1], n[2]));
			}

			//! ����Ҫ��ԭ�ȵİ󶨷�ʽ����
			geometry.setNormalArray(normals, static_cast<osg::Array::Binding>(geometry.getNormalBinding()));
		}
		//��ɫ
		if (!mesh.colors.empty()) {

			auto colors = new osg::Vec4Array();

			for (auto& color : mesh.colors)
			{
				colors->push_back(osg::Vec4(color[0], color[1], color[2], color[3]));
			}

			//! ����Ҫ��ԭ�ȵİ󶨷�ʽ����
			geometry.setColorArray(colors, static_cast<osg::Array::Binding>(geometry.getColorBinding()));
		}
		//��������
		if (!mesh.uvs.empty()) {
			auto uvs = new osg::Vec2Array();

			for (auto& uv : mesh.uvs)
			{
				uvs->push_back(osg::Vec2(uv[0], uv[1]));
			}

			geometry.setTexCoordArray(0, uvs);
		}

		//����
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

	//����
	MeshVisitor visitor;
	if (root)
	{
		root->accept(visitor);
	}

	//����
	osgDB::writeNodeFile(*root, osgb_output_path);

	system("pause");
	return 0;
}
