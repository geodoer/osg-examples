/*
 * Author: geodoer
 * Time: 2022/07/18
 * Desc: 遍历OSGB的树状结构
 */
#include <osg/Node>
#include <osgDB/ReadFile>
#include <iostream>

using namespace std;

class InfoVisitor : public osg::NodeVisitor
{
public:
	InfoVisitor(): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), _indent(0)
	{
	}

	virtual void apply(osg::Node& node)
	{
		for (int i = 0; i < _indent; i++) cout << "    ";
		cout << "[" << _indent << "]" << node.libraryName()

			<< "::" << node.className() << endl;

		_indent++;
		traverse(node);
		_indent--;

		for (int i = 0; i < _indent; i++) cout << "    ";
		cout << "[" << _indent << "] " << node.libraryName()
			<< "::" << node.className() << endl;
	}

	virtual void apply(osg::Geode& node)
	{
		for (int i = 0; i < _indent; i++) cout << "    ";
		cout << "[" << _indent << "] " << node.libraryName()
			<< "::" << node.className() << endl;

		_indent++;
		for (unsigned int n = 0; n < node.getNumDrawables(); n++)
		{
			osg::Drawable* draw = node.getDrawable(n);
			if (!draw)
				continue;

			for (int i = 0; i < _indent; i++) cout << "    ";
			cout << "[" << _indent << "]" << draw->libraryName() << "::"
				<< draw->className() << endl;
		}

		traverse(node);
		_indent--;

		for (int i = 0; i < _indent; i++) cout << "    ";
		cout << "[" << _indent << "]" << node.libraryName()
			<< "::" << node.className() << endl;
	}

private:
	int _indent;
};


int main(int argc, char** argv)
{
	std::string osgb_path = R"(F:\data\algo\Monomer\3D\metadata.osgb)";

	osg::ArgumentParser parser(&argc, argv);
	osg::Node* root = osgDB::readNodeFiles(parser);

	if (!root)
	{
		root = osgDB::readNodeFile(osgb_path);
	}

	InfoVisitor infoVisitor;
	if (root)
	{
		root->accept(infoVisitor);
	}

	system("pause");
	return 0;
}
