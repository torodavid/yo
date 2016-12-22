#include <osg/Texture2D>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>

#include <osg/MatrixTransform> // új

#ifdef _DEBUG
#pragma comment(lib, "osgd.lib")
#pragma comment(lib, "osgDBd.lib")
#pragma comment(lib, "osgViewerd.lib")
#else
#pragma comment(lib, "osg.lib")
#pragma comment(lib, "osgDB.lib")
#pragma comment(lib, "osgViewer.lib")
#endif

#pragma warning(disable : 4482 )

void calc_(float x, float y,
	osg::ref_ptr<osg::Vec3Array>& vertices,
	osg::ref_ptr<osg::Vec3Array>& normals,
	osg::ref_ptr<osg::Vec2Array>& texcoords
) {
	float u = -2 * osg::PI * x;
	float v = 2 * osg::PI * y;
	float R = 5.0f;
	float r = 2.0f;

	vertices->push_back(osg::Vec3(sin(u)*sin(v), cos(v), cos(u)*sin(v))); //GÖMB
	//vertices->push_back(osg::Vec3((R + r * cosf(u)) * cosf(v), r * sinf(u), (R + r * cosf(u)) * sinf(v))); //TÓRUSZ
	//vertices->push_back(osg::Vec3(sin(u), y*R, cos(u))); //HENGER
	normals->push_back(osg::Vec3(sin(u)*sin(v), cos(v), cos(u)*sin(v)));
	texcoords->push_back(osg::Vec2(x, 1-y * 0.5)); // fej
}

osg::ref_ptr<osg::Geometry> create_()
{
	// pozíciók
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;

	const int N = 16;
	float delta = 1.0 / N;
	for (int i = 0; i<N; ++i) {
		for (int j = 0; j<N; ++j) {
			float x = i*delta;
			float y = j*delta;
			// 1. háromszög: x,y x+delta,y y+delta,x
			calc_(x, y, vertices, normals, texcoords);
			calc_(x, y + delta, vertices, normals, texcoords);
			calc_(x + delta, y, vertices, normals, texcoords);
			// 2. háromszög: x+delta,y x+delta,y+delta y+delta,x
			calc_(x + delta, y, vertices, normals, texcoords);
			calc_(x, y + delta, vertices, normals, texcoords);
			calc_(x + delta, y + delta, vertices, normals, texcoords);
		}
	}

	// négyszög geometria
	osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
	quad->setUseVertexBufferObjects(true);

	// állítsuk be, hogy a VBO-ba milyen adatok kerüljenek
	quad->setVertexArray(vertices.get());
	quad->setNormalArray(normals.get());
	quad->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	quad->setTexCoordArray(0, texcoords.get());
	// kirajzolandó primitív meghatározása
	quad->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 6 * N*N));

	return quad;
}


int main(int argc, char** argv)
{
	// textúra betöltése
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	//osg::ref_ptr<osg::Image> image = osgDB::readImageFile("Images/land_shallow_topo_2048.jpg"); //valami texturaja
	
	texture->setImage(image.get());
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::WrapMode::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::WrapMode::REPEAT);

	osg::ref_ptr<osg::Geometry> objektum1 = create_1();
	osg::ref_ptr<osg::Geometry> objektum2 = create_2();
	osg::ref_ptr<osg::Geometry> objektum3 = create_3();

	osg::ref_ptr<osg::Geode> root = new osg::Geode; 
	osg::ref_ptr<osg::Geode> obj1_geode = new osg::Geode; 
	osg::ref_ptr<osg::Geode> obj2_geode = new osg::Geode; 

	root->addDrawable(objektum1.get());
	obj1_geode->addDrawable(objektum2.get());
	obj2_geode->addDrawable(objektum3.get());

	
	//mindegyik geode-re
	{
		// 0-ás mintavételezőre rakjuk rá a textúrát
		root->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
		osg::StateSet* state = root->getOrCreateStateSet();
		// kikapcsoljuk a megvilágítást
		state->setMode(GL_LIGHTING,
			osg::StateAttribute::OFF |
			osg::StateAttribute::PROTECTED);
		// bekapcsoljuk a hátrafelé néző lapok eldobását
		state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
	}

	// CSOPORT
	osg::ref_ptr<osg::Group> csoport = new osg::Group;
	
	
	//mindegyik obj-re trafo
	osg::ref_ptr<osg::MatrixTransform> valami_trafo = new osg::MatrixTransform;
	valami_trafo->setMatrix(osg::Matrix::translate(0,2.5,0));
	valami_trafo->addChild(root);

	//csoport->addChild(root.get()); // fej
	csoport->addChild(valami_trafo.get()); // fej


	//csoport->addChild(torzs_geode.get());
	csoport->addChild(torzs_trafo.get());

	// hozzuk létre a viewer-t és állítsuk be a gyökeret megjelenítendő adatnak
	osgViewer::Viewer viewer;
	//viewer.setSceneData(root.get());
	viewer.setSceneData(csoport.get());
	//viewer.setSceneData(root.get());

	// a (20,20) kezdeti pozícióba hozzunk létre egy 640x480-as ablakot
	viewer.setUpViewInWindow(20, 20, 640, 480);
	viewer.realize();

	// írjuk ki egy obj fájlba a parametrikus felületünket!
	osgDB::writeNodeFile(*(root.get()), "valami.obj");

	// adjuk át a vezérlést a viewer-nek
	return viewer.run();
}

/*// textúra betöltése
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile("vaz.jpg");
	texture->setImage(image.get());
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::WrapMode::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::WrapMode::REPEAT);

	// textúra betöltése
	osg::ref_ptr<osg::Texture2D> texture2 = new osg::Texture2D;
	osg::ref_ptr<osg::Image> image2 = osgDB::readImageFile("kerek.jpg");
	texture2->setImage(image2.get());
	texture2->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture2->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture2->setWrap(osg::Texture::WRAP_S, osg::Texture::WrapMode::REPEAT);
	texture2->setWrap(osg::Texture::WRAP_T, osg::Texture::WrapMode::REPEAT);

	// rakjuk be egy geode-ba a quad-ot, mint kirajzolandó elemet!
	osg::ref_ptr<osg::Geode> gombGeode = getGomb();
	osg::ref_ptr<osg::Geode> korlapGeode = getKorlapGeode();

	osg::ref_ptr<osg::MatrixTransform> gombTrafo = new osg::MatrixTransform;
	gombTrafo->setMatrix(osg::Matrix::scale(0.5, 1.0f, 0.5f) *
		osg::Matrix::rotate(osg::PI / 2.0f, 1, 0, 0));
	gombTrafo->addChild(gombGeode);

	osg::ref_ptr<osg::MatrixTransform> korlap1Trafo = new osg::MatrixTransform;
	korlap1Trafo->setMatrix(osg::Matrix::rotate( osg::PI, 0, 0, 1) * 
							osg::Matrix::scale(1 / sqrt(2) / 2.0, 1.0f, 1 / sqrt(2) / 2.0) *
							 osg::Matrix::translate(0, sqrt(2) / 2.0f, 0) *
		osg::Matrix::rotate(osg::PI / 2.0f, 1, 0, 0));
	korlap1Trafo->addChild(korlapGeode);


	osg::ref_ptr<osg::MatrixTransform> korlap2Trafo = new osg::MatrixTransform;
	korlap2Trafo->setMatrix(
		osg::Matrix::scale(1 / sqrt(2) / 2.0, 1.0f, 1 / sqrt(2) / 2.0) *
		osg::Matrix::translate(0, -sqrt(2) / 2.0f, 0) *
		osg::Matrix::rotate(osg::PI / 2.0f, 1, 0, 0));
	korlap2Trafo->addChild(korlapGeode);


	osg::ref_ptr<osg::Geode> torusGeode = getTorus();
	osg::ref_ptr<osg::MatrixTransform> torusTrafo1 = new osg::MatrixTransform;
	torusTrafo1->setMatrix( osg::Matrix::scale(0.5f, 0.5f, 2.5f) * 
						osg::Matrix::rotate(osg::PI / 2.0f, 0, 1, 0) *
						osg::Matrix::translate(0, -0.5f, -sqrt(2) / 2.0f - 0.5f));
	torusTrafo1->addChild(torusGeode);



	osg::ref_ptr<osg::MatrixTransform> torusTrafo2 = new osg::MatrixTransform;
	torusTrafo2->setMatrix(osg::Matrix::scale(0.5f, 0.5f, 2.5f) *
		osg::Matrix::rotate(osg::PI / 2.0f, 0, 1, 0) *
		osg::Matrix::translate(0, -0.5f, +sqrt(2) / 2.0f + 0.5f));
	torusTrafo2->addChild(torusGeode);


	osg::ref_ptr<osg::Group> vaz = new osg::Group;
	vaz->addChild(gombTrafo);
	vaz->addChild(korlap1Trafo);
	vaz->addChild(korlap2Trafo);
	vaz->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());


	osg::ref_ptr<osg::Group> kerek1 = new osg::Group;
	osg::ref_ptr<osg::Group> kerek2 = new osg::Group;
	kerek1->addChild(torusTrafo1);
	kerek2->addChild(torusTrafo2);

	kerek1->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture2.get());
	kerek2->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture2.get());

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(getCoordSystem());
	root->addChild(vaz);
	root->addChild(kerek1);
	root->addChild(kerek2);
	//root->addChild(torusGeode);

	// 0-ás mintavételezőre rakjuk rá a textúrát
	osg::StateSet* state = root->getOrCreateStateSet();
	// kikapcsoljuk a megvilágítást
	state->setMode(GL_LIGHTING,
		osg::StateAttribute::OFF |
		osg::StateAttribute::PROTECTED);
	// bekapcsoljuk a hátrafelé néző lapok eldobását
	state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);


	// hozzuk létre a viewer-t és állítsuk be a gyökeret megjelenítendő adatnak
	osgViewer::Viewer viewer;
	viewer.setSceneData(root.get());

	// a (20,20) kezdeti pozícióba hozzunk létre egy 640x480-as ablakot
	viewer.setUpViewInWindow(20, 20, 640, 480);
	viewer.realize();

	// írjuk ki egy obj fájlba a parametrikus felületünket!
	osgDB::writeNodeFile(*(vaz.get()), "vaz.obj");
	osgDB::writeNodeFile(*(kerek1.get()), "kerek1.obj");

	// adjuk át a vezérlést a viewer-nek
	return viewer.run();*/