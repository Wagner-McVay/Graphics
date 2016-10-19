
#include "crenderutils.h"

#include "GLM\ext.hpp"

void main()
{
	Window context;
	context.init(1280, 720);

	// ASSETS
	// --------------------
	Geometry quad = makeGeometry(quad_verts, 4, quad_tris, 6);
	Geometry spear = loadOBJ("../res/models/soulspear.obj");
	Geometry sphere = loadOBJ("../res/models/sphere.obj");

	Texture spear_normal = loadTexture("../res/textures/soulspear_normal.tga");
	Texture spear_diffuse = loadTexture("../res/textures/soulspear_diffuse.tga");
	Texture spear_specular = loadTexture("../res/textures/soulspear_specular.tga");

	Texture stone_normal = loadTexture("../res/textures/sandstone_n.png");
	Texture stone_diffuse = loadTexture("../res/textures/sandstone_d.png");
	Texture stone_specular = loadTexture("../res/textures/sandstone_h.png");


	const unsigned char norm_pixels[4] = {127, 127, 255, 255};
	Texture vertex_normals = makeTexture(1, 1, 4, norm_pixels);

	const unsigned char white_pixels[4] = { 255, 255, 255, 255 };
	Texture white = makeTexture(1, 1, 4, white_pixels);

	Shader gpass = loadShader("../res/shaders/gpass.vert",
							  "../res/shaders/gpass.frag");
	
	Shader lpass = loadShader("../res/shaders/lspass.vert",
							  "../res/shaders/lspass.frag", false, true);
	
	Shader post  = loadShader("../res/shaders/quad.vert",
							  "../res/shaders/quad.frag", false);

	Shader blur  = loadShader("../res/shaders/post.vert",
							  "../res/shaders/post.frag", false);

	Shader spass = loadShader("../res/shaders/spass.vert",
							  "../res/shaders/spass.frag", true, false, false);



	// Renderer

	// Framebuffers
	bool floatTexs[] = { false, true, false, true };

	Framebuffer gframe = makeFramebuffer(1280, 720, 4, floatTexs);
	Framebuffer lframe = makeFramebuffer(1280, 720, 3);
	//Framebuffer nframe = makeFramebuffer(1280, 720, 1);
	Framebuffer sframe = makeFramebuffer(1024, 1024, 0);
	Framebuffer screen = { 0, 1280, 720 };
	
	// Model Matrices
	glm::mat4 model, view, proj;

	model = glm::translate(glm::vec3(0,-1,0));
	view  = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	proj  = glm::perspective(45.f, 1280.f/720, 1.f, 100.f);

	glm::mat4 lightProj = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
	glm::mat4   redView = glm::lookAt(glm::normalize(-glm::vec3(1, -1, -1)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::vec4   redColor = glm::vec4(1, 0, 0, 1);

	glm::mat4 spearModel	;
	glm::mat4 sphereModel	= glm::translate(glm::vec3(0.3f, -1, -0.2f));
	glm::mat4 wallModel		= glm::rotate(45.f, glm::vec3(0,-1,0)) * glm::translate(glm::vec3(0,0,-2)) * glm::scale(glm::vec3(2,2,1));

	float time = 0;

	while (context.step())
	{
		time += 0.016f;
		

		spearModel = glm::rotate(time, glm::vec3(0,1,0)) * glm::translate(glm::vec3(0, -1, 0));


		/////////////////////////////////////////////////////
		// Geometry Pass
		//
		// Only the textures, geometry, and model matrix really differs
		// from object to object.
		clearFramebuffer(gframe);
		tdraw(gpass, spear, gframe, spearModel, view, proj,
						spear_diffuse, spear_normal, spear_specular);

		tdraw(gpass, sphere, gframe, model, view, proj,
			white, vertex_normals, white);

		tdraw(gpass, quad, gframe, 
								   glm::rotate(45.f, glm::vec3(0, -1, 0))*
								   glm::translate(glm::vec3(0,0,-2)) *
								   glm::scale(glm::vec3(2,2,1)),									
								view, proj,
			stone_diffuse, stone_normal, stone_specular);

		//tdraw(blur, quad, nframe, gframe.colors[1]);


	/////////////////////////////////////////////////////
	//// Lights!
	////
	//// Each call is a different light
	//// They all use the same information from the g-pass,
	//// but provide different colors/directions.
		clearFramebuffer(lframe);

		clearFramebuffer(sframe);
		tdraw(spass, spear, sframe, spearModel, redView, lightProj);
		tdraw(spass, sphere, sframe, sphereModel, redView, lightProj);
		tdraw(spass, quad, sframe, wallModel, redView, lightProj);
		// Light Aggregation
		tdraw(lpass, quad, lframe, view,
			gframe.colors[0], gframe.colors[1], gframe.colors[2], gframe.colors[3],
			sframe.depth, redColor, redView, lightProj);

		//////////////////////////////////////////////////
		// Debug Rendering Stuff. Just single textures to quads-
		// drawing most of the images I've gathered so far.

		for(int i = 0; i < 4; ++i)
		{
			glm::mat4 mod =
				glm::translate(glm::vec3(-.75f + .5*i, 0.75f, 0)) * 
				glm::scale(glm::vec3(0.25f,0.25f,1.f));
			tdraw(post, quad, screen, gframe.colors[i], mod);
		}

		glm::mat4 mod =
			glm::translate(glm::vec3(-.75f, 0.25f, 0)) *
			glm::scale(glm::vec3(0.25f, 0.25f, 1.f));
		tdraw(post, quad, screen, gframe.depth, mod);
	
		mod =
			glm::translate(glm::vec3(-.5f, -0.5f, 0)) *
			glm::scale(glm::vec3(0.5f, 0.5f, 1.f));
		tdraw(post, quad, screen, lframe.colors[0], mod);

		mod =
			glm::translate(glm::vec3(.25f, 0.25f, 0)) *
			glm::scale(glm::vec3(0.25f, 0.25f, 1.f));
		tdraw(post, quad, screen, lframe.colors[1], mod);

		mod =
			glm::translate(glm::vec3(.75f, 0.25f, 0)) *
			glm::scale(glm::vec3(0.25f, 0.25f, 1.f));
		tdraw(post, quad, screen, lframe.colors[2], mod);

		mod =
			glm::translate(glm::vec3(.75f, 0.25f, 0)) *
			glm::scale(glm::vec3(0.25f, 0.25f, 1.f));
		tdraw(post, quad, screen, sframe.depth, mod);
	}

	context.term();
}