#include <osgViewer/Viewer>
#include <osgDB/ReadFile>

int main()
{
	//https://github.com/openscenegraph/OpenSceneGraph-Data
	std::string data_path = DATA_PATH "glider.copy.osg";
	
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(data_path);
	viewer->setSceneData(node.get());
	return viewer->run();
}
