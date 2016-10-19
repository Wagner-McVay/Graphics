

#include "crenderutils.h"
#include "window.h"
#include "Input.h"
#include "Vertex.h"

#include "GLM\glm.hpp"
#include "GLM\ext.hpp"

int main()
{
	Window	window;
	Input input;
	window.init(1280, 720);
	input.init(window);

	glm::mat4 view = glm::lookAt(glm::vec3(0.f, 1.5f, 3.f),  // eye
		glm::vec3(0.f, 1.5f, 0.f),  // center
		glm::vec3(0.f, 1.f, 0.f)); // up

	glm::mat4 proj = glm::perspective(45.f, 16 / 9.f, 1.f, 100.f);
	glm::mat4 modelC, modelS;

	Geometry soulspear = loadOBJ("../res/models/soulspear.obj");
	Shader   shader = loadShader("../res/shaders/phongVert.glsl",
								 "../res/shaders/phongFrag.glsl");



	Texture tarray[] = { loadTexture("../res/textures/soulspear_diffuse.tga"),
						 loadTexture("../res/textures/soulspear_specular.tga"),
						 loadTexture("../res/textures/soulspear_normal.tga") };

	Framebuffer frame  = makeFramebuffer(1280, 720, 1);
	Framebuffer frame2 = makeFramebuffer(1280, 720, 1);

	Framebuffer screen = { 0, 1280, 720, 1 };

	Vertex  verts[4] = { { {-1,-1,0,1 }, {}, {}, {0,0} },
						 { { 1,-1,0,1 }, {}, {}, {1,0} },
						 { { 1, 1,0,1 }, {}, {}, {1,1} },
						 { {-1, 1,0,1 }, {}, {}, {0,1} } };

	unsigned tris[] = {0,1,2,2,3,0};

	Geometry quad = makeGeometry(verts, 4, tris, 6);

	Shader   post = loadShader("../res/shaders/postVert.glsl",
							   "../res/shaders/postFrag.glsl");
	
	frame.colors[frame.nColors] = frame.depth;
	float time = 0;
	while (window.step())
	{
		clearFramebuffer(frame);
		clearFramebuffer(frame2);
		input.step();
		time += 0.016f;	
		modelC = glm::rotate(time, glm::normalize(glm::vec3(0, 1, 0)));
	
		drawFB(shader, soulspear, frame,
								glm::value_ptr(modelC),
								glm::value_ptr(view),
								glm::value_ptr(proj),
								tarray, 3);
		
		drawFB(post, quad, frame2, glm::value_ptr(glm::mat4(time)),
								   glm::value_ptr(glm::mat4()),
								   glm::value_ptr(glm::mat4()),
			frame.colors, frame.nColors+1);

			drawFB(post, quad, frame, glm::value_ptr(glm::mat4(time)),
									glm::value_ptr(glm::mat4()),
									glm::value_ptr(glm::mat4()),
									frame2.colors, frame2.nColors);


			drawFB(post, quad, frame2, glm::value_ptr(glm::mat4(time)),
				glm::value_ptr(glm::mat4()),
				glm::value_ptr(glm::mat4()),
				frame.colors, frame.nColors);

			drawFB(post, quad, screen, glm::value_ptr(glm::mat4(time)),
				glm::value_ptr(glm::mat4()),
				glm::value_ptr(glm::mat4()),
				frame2.colors, frame2.nColors);
	}
	
	freeFramebuffer(frame);
	freeShader(shader);
	freeGeometry(soulspear);
	for each(auto &t in tarray) freeTexture(t);
	window.term();
	return 0;
}