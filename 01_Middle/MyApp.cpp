#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void)
{
	m_textureID = 0;
	m_textureID_kerek = 0;
	m_textureID_vaz = 0;
	m_mesh = 0;
	m_mesh_kerek1 = 0;

	pos = glm::vec3(0, 0, 0);
	dir = glm::vec3(0, 0, 1);
	speed = 1.0f;
	bicaj_u = 0.0f;
	freeCamera = true;
	color_start_time = 0L;
	color_t = 0.0f;

	szinek[0] = glm::vec4(0, 178 / 255.0f, 235 / 255.0f, 1); //cián
	szinek[1] = glm::vec4(127 / 255.0f, 127 / 255.0f, 127 / 255.0f, 1); //szürke
	szinek[2] = glm::vec4(255 / 255.0f, 170/ 255.0f, 0, 1); //narancs
	szinek_index = 0;
}


CMyApp::~CMyApp(void)
{
}


GLuint CMyApp::GenTexture()
{
    unsigned char tex[256][256][3];
 
    for (int i=0; i<256; ++i)
        for (int j=0; j<256; ++j)
        {
			tex[i][j][0] = rand()%256;
			tex[i][j][1] = rand()%256;
			tex[i][j][2] = rand()%256;
        }
 
	GLuint tmpID;

	// generáljunk egy textúra erõforrás nevet
    glGenTextures(1, &tmpID);
	// aktiváljuk a most generált nevû textúrát
    glBindTexture(GL_TEXTURE_2D, tmpID);
	// töltsük fel adatokkal az...
    gluBuild2DMipmaps(  GL_TEXTURE_2D,	// aktív 2D textúrát
						GL_RGB8,		// a vörös, zöld és kék csatornákat 8-8 biten tárolja a textúra
						256, 256,		// 256x256 méretû legyen
						GL_RGB,				// a textúra forrása RGB értékeket tárol, ilyen sorrendben
						GL_UNSIGNED_BYTE,	// egy-egy színkopmonenst egy unsigned byte-ról kell olvasni
						tex);				// és a textúra adatait a rendszermemória ezen szegletébõl töltsük fel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// bilineáris szûrés kicsinyítéskor
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// és nagyításkor is
	glBindTexture(GL_TEXTURE_2D, 0);

	return tmpID;
}

bool CMyApp::Init()
{
	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// mélységi teszt bekapcsolása (takarás)

	//
	// geometria letrehozasa
	//
	m_vb.AddAttribute(0, 3);
	m_vb.AddAttribute(1, 3);
	m_vb.AddAttribute(2, 2);

	m_vb.AddData(0, -61.5,  0, -61.5);
	m_vb.AddData(0, 61.5,  0, -61.5);
	m_vb.AddData(0, -61.5,  0, 61.5);
	m_vb.AddData(0, 61.5,  0, 61.5);

	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);

	m_vb.AddData(2, 0, 0);
	m_vb.AddData(2, 1, 0);
	m_vb.AddData(2, 0, 1);
	m_vb.AddData(2, 1, 1);

	m_vb.AddIndex(1, 0, 2);
	m_vb.AddIndex(1, 2, 3);

	m_vb.InitBuffers();

	//
	// shaderek betöltése
	//
	m_program.AttachShader(GL_VERTEX_SHADER, "dirLight.vert");
	m_program.AttachShader(GL_FRAGMENT_SHADER, "dirLight.frag");

	m_program.BindAttribLoc(0, "vs_in_pos");
	m_program.BindAttribLoc(1, "vs_in_normal");
	m_program.BindAttribLoc(2, "vs_in_tex0");

	if ( !m_program.LinkProgram() )
	{
		return false;
	}

	//
	// egyéb inicializálás
	//

	m_camera.SetProj(45.0f, 640.0f/480.0f, 0.01f, 1000.0f);

	// textúra betöltése
	m_textureID = TextureFromFile("talaj.jpg");
	m_textureID_kerek = TextureFromFile("kerek.jpg");
	m_textureID_vaz = TextureFromFile("vaz.jpg");

	// mesh betöltés
	m_mesh = ObjParser::parse("vaz.obj");
	m_mesh->initBuffers();

	m_mesh_kerek1 = ObjParser::parse("kerek1.obj");
	m_mesh_kerek1->initBuffers();


	return true;
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &m_textureID);

	m_program.Clean();
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;


	last_time = SDL_GetTicks();
	
	glm::vec4 dir = glm::rotate<float>(bicaj_u, 0, 1, 0) * glm::vec4(0, 0, 1, 1);
	glm::vec3 dir2 = glm::vec3(dir.x, dir.y, dir.z);
	pos = pos + dir2 * delta_time * speed;


	if (!freeCamera) {
		m_camera.SetView(pos + glm::vec3(0, 2.5f, 0) - dir2* 3.0f,
			pos, glm::vec3(0, 1, 0));
	}

	m_camera.Update(delta_time);

	if (color_start_time != 0) {
		if (last_time - color_start_time > 2000) {
			color_start_time = 0;
			color_t = 0.0f;
			szinek_index++;
			if (szinek_index == 3) {
				szinek_index = 0;
			}
		}
		else {
			color_t = (last_time - color_start_time) / 2000.0f;
		}
	}
}

void CMyApp::RenderTalaj() {
	m_program.On();

	glm::mat4 matWorld = glm::mat4(1.0f);
	glm::mat4 matWorldIT = glm::transpose(glm::inverse(matWorld));
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform("world", matWorld);
	m_program.SetUniform("worldIT", matWorldIT);
	m_program.SetUniform("MVP", mvp);
	m_program.SetUniform("eye_pos", m_camera.GetEye());

	m_program.SetTexture("texImage", 0, m_textureID);

	// kapcsoljuk be a VAO-t (a VBO jön vele együtt)
	m_vb.On();

	m_vb.DrawIndexed(GL_TRIANGLES, 0, 6, 0);

	m_vb.Off();
	// shader kikapcsolasa
	m_program.Off();

}
void CMyApp::RenderBicaj(const glm::mat4 mat) {


	// 2. program
	m_program.On();

	glm::mat4 matWorld = mat * glm::translate<float>(0, 1, 0);
	glm::mat4 matWorldIT = glm::transpose(glm::inverse(matWorld));
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform("world", matWorld);
	m_program.SetUniform("worldIT", matWorldIT);
	m_program.SetUniform("MVP", mvp);
	m_program.SetUniform("eye_pos", m_camera.GetEye());

	//pontszeru feny a bringa felett 3-al
	m_program.SetUniform("light_pos", pos + glm::vec3(0, 3, 0));


	m_program.SetUniform("t", color_t);
	m_program.SetUniform("fromColor", szinek[szinek_index % 3]);
	m_program.SetUniform("toColor", szinek[(szinek_index + 1) % 3]);

	m_program.SetTexture("texImage", 0, m_textureID_vaz);

	m_mesh->draw();

	double t = SDL_GetTicks() / 3000.0f;
	matWorld = mat * glm::translate<float>(0, 1.0f, 0)  *
		glm::translate<float>(0, -0.5f, -sqrt(2) / 2.0f - 0.5f) *
		glm::rotate<float> (t * 360, 1, 0, 0) *
		glm::translate<float>(0, 0.5f, sqrt(2) / 2.0f + 0.5f);
	matWorldIT = glm::transpose(glm::inverse(matWorld));
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform("world", matWorld);
	m_program.SetUniform("worldIT", matWorldIT);
	m_program.SetUniform("MVP", mvp);

	m_program.SetTexture("texImage", 0, m_textureID_kerek);
	m_mesh_kerek1->draw();

	matWorld = mat * glm::translate<float>(0, 1.0f, 0)  *
		glm::translate<float>(0, -0.5f, sqrt(2) / 2.0f + 0.5f) *
		glm::rotate<float>(t * 360, 1, 0, 0) *
		glm::translate<float>(0, 0.5f, sqrt(2) / 2.0f + 0.5f);
	matWorldIT = glm::transpose(glm::inverse(matWorld));
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform("world", matWorld);
	m_program.SetUniform("worldIT", matWorldIT);
	m_program.SetUniform("MVP", mvp);
	m_mesh_kerek1->draw();

	m_program.Off();
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderTalaj();
	glm::mat4 world = glm::translate<float>(pos) * 
					 glm::rotate<float>(bicaj_u, 0, 1, 0);
	RenderBicaj(world);
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	if (freeCamera)
		m_camera.KeyboardDown(key);

	switch (key.keysym.sym) {
	case SDLK_RIGHT:
		bicaj_u -= 15.f;
		break;
	case SDLK_LEFT:
		bicaj_u += 15.f;
	}
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	if (freeCamera)
		m_camera.KeyboardUp(key);
	switch (key.keysym.sym) {
	case SDLK_UP:
		speed *= 1.2f;
		break;
	case SDLK_DOWN:
		speed *= 0.8f;
		break;
	case SDLK_c:
		freeCamera = !freeCamera;
		break;
	case SDLK_SPACE:
		color_start_time = SDL_GetTicks();
		break;
	}
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	if (freeCamera)
		m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_camera.Resize(_w, _h);
}