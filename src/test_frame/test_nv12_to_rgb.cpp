#include <gtest/gtest.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <map>
#include <memory>

using glm::mat4;
using glm::min;
using glm::radians;
using glm::rotate;
using glm::vec3;

// primitive attribute, this will be trans into shader for distinguising current-handling primitive
#define PRIMITIVE_NONE 0
#define PRIMITIVE_CIRCLE_POINT 1
#define PRIMITIVE_LINE 2

constexpr int screenWidth = 800, screenHeight = 600;

CameraWithController camera(screenWidth, screenHeight);

void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
	camera.mouseCallback(xpos, ypos);
}
void scrollCallback(GLFWwindow *window, double offsetX, double offsetY)
{
	camera.scrollCallback(offsetX, offsetY);
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	camera.framebuffer_size_callback(window, width, height);
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
	camera.processInput(window);
}

int tmain()
{
	{ // set camera position
		camera.cameraPos = vec3(0, 0, 2);
	}

	glfwInit();
	glfwWindowHint(GL_SAMPLES, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	auto window = glfwCreateWindow(screenWidth, screenHeight, "Learning OpenGL", NULL, NULL);
	if (!window)
	{
		std::cout << "create widnow failed" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Light::OpenGLContext gl_context(window);
	gl_context.init();

	glViewport(0, 0, screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	// shader && program
	ShaderObject vs = {GL_VERTEX_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.vs.glsl")};
	ShaderObject fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.fs.glsl")};
	ShaderObject draw_line_fs = {GL_FRAGMENT_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/draw_axiss.fs.glsl")};

	ProgramObject program = Helper::CreateProgram(vs, fs);
	ProgramObject draw_line_program = Helper::CreateProgram(vs, draw_line_fs);
	// load texture
	GLuint texture;
	TextureObject t1 = Helper::CreateTexture(GL_TEXTURE0, "../media/texture/bmp/skiing.dib");
	texture = t1.GetTTexture();

	GLuint texture2;
	TextureObject t2 = Helper::CreateTexture(GL_TEXTURE1, "../media/texture/bmp/2004050204170.bmp");
	texture2 = t2.GetTTexture();

	GL_ERROR_STOP();

	auto intAsFloat = [](int32_t i)
	{
		float ret = *reinterpret_cast<float *>(&i);
		return ret;
	};
	constexpr auto __L = 1.f, __R = -1.f;
	constexpr auto _L = -0.f, _R = 0.3f, _D = 0.0f;
	constexpr auto _LL = -0.3f, _RR = 0.3f;
	constexpr auto _L1 = -0.8f, _R1 = 0.0f, _B1 = -0.2f, _T1 = -_B1;
	constexpr auto _L2 = -0.0f, _R2 = 0.8f, _B2 = -0.2f, _T2 = -_B2;
	// position : 3     color : 2       primitive : 1
	// clang-format off
	float vertices[] = {
		_L1, _B1, _D, __L, __L,PRIMITIVE_NONE,
		_R1, _B1, _D, __R, __L,PRIMITIVE_NONE,
		_R1, _T1, _D, __R, __R,PRIMITIVE_NONE,
		_L1, _T1, _D, __L, __R,PRIMITIVE_NONE,

		_L2, _B2, _D, __L, __L,PRIMITIVE_NONE,
		_R2, _B2, _D, __R, __L,PRIMITIVE_NONE,
		_R2, _T2, _D, __R, __R,PRIMITIVE_NONE,
		_L2, _T2, _D, __L, __R,PRIMITIVE_NONE,
		// debugging O(0,0,0) X(1,0,0) Y(0,1,0)
		// line 
		0,0,0, 0,0, PRIMITIVE_LINE,
		1,0,0, 0,0, PRIMITIVE_LINE,
		0,1,0, 0,0, PRIMITIVE_LINE,
		// point
		0,0,0, 0,0, PRIMITIVE_CIRCLE_POINT,
		1,0,0, 0,0, PRIMITIVE_CIRCLE_POINT,
		0,1,0, 0,0, PRIMITIVE_CIRCLE_POINT,
	};
	GLuint indices[] = {
		0, 1, 2,    // left triangle one
		0, 2, 3,    // left triangle two
		4, 5, 6,    //right triangle one
		4, 6, 7,    //right triangle two
		8,9,8,10
	};
	//clang-format on
	
	auto texture_BOL = Light::BufferLayout({
		Light::BufferElement(Light::ShaderDataType::Float3,"Pos",false)
		 ,Light::BufferElement(Light::ShaderDataType::Float2,"aTexCoord",false)
		 ,Light::BufferElement(Light::ShaderDataType::Float,"aChooseTex",false) 
		});
    auto texture_AO=std::make_shared<Light::OpenGLVertexArray>();
	auto texture_BO =std::make_shared<Light::OpenGLVertexBuffer>(vertices, sizeof(vertices));
	auto texture_EO =std::make_shared<Light::OpenGLIndexBuffer>(indices, sizeof(indices));
	texture_BO->create(vertices,sizeof(vertices));
	texture_BO->setLayout(texture_BOL);
	texture_EO->create(indices,sizeof(indices)/sizeof(float));

	texture_AO->addVertexBuffer(texture_BO);
	texture_AO->setIndexBuffer(texture_EO);
	
	auto uniforms = program.getUniforms();
	auto attributes = program.getAttributes();
	GL_ERROR_STOP();
	// the follow two all work
	glUseProgram(program.getProgram());
	// glUniform1i(glGetUniformLocation(program.getProgram(),"ourTexture1"),0);
	program.setInt("ourTexture2", 1);
	program.setInt("ourTexture1", 0);
	std::map<int, int> frame_record;
	frame_record[-1] = 0;

	// depth test && SSAA anti-aliasing
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		double tm = glfwGetTime();
		camera.Tick(tm);
		processInput(window);
		glfwPollEvents();
		glClearColor(0.2, .2, .3, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program.getProgram());
		// coordinate transform
		mat4 model = glm::mat4(1.0f);
		// model = rotate<float>(mat4(1.f), tm * radians<float>(-55), vec3(0.5, 1.0, 0));
		mat4 view = camera.GetViewMatrix();
		view = glm::translate(view, glm::vec3(0.0f, 0.0f,
			-1 // sinf(tm)*2
		));
		auto rate = float(screenWidth) / screenHeight;
		auto projection = camera.GetProjectionMatrix();
		GL_ERROR_STOP();
		// shut camera view-changing
		//model=projection= view=mat4(1.0f);
		//
		program.setFloat("mixRate", sinf(tm) / 2 + 0.5);
		int model_loc = program.getUniformLocation("model");
		int view_loc = program.getUniformLocation("view");
		int proj_loc = program.getUniformLocation("projection");
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(proj_loc, 1, GL_FALSE, value_ptr(projection));
		texture_AO->bind();
		// left rectangle
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 3 + (GLuint*)0);
		// right rectangle
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 6 + (GLuint*)0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 9 + (GLuint*)0);

		GL_ERROR_STOP();



		// draw O,X,Y   see shader: O:black, X:red, Y:green
		glPointSize(20);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.5));
		glUseProgram(draw_line_program.getProgram());
		auto d_attributes = draw_line_program.getAttributes();
		auto d_uniforms = draw_line_program.getUniforms();
		_ASSERT(d_uniforms.find("model") != d_uniforms.end());
		_ASSERT(d_uniforms.find("view") != d_uniforms.end());
		_ASSERT(d_uniforms.find("projection") != d_uniforms.end());
		_ASSERT(d_uniforms.find("primitive_id") != d_uniforms.end());
		glUniformMatrix4fv(d_uniforms["model"].position, 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(d_uniforms["view"].position, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(d_uniforms["projection"].position, 1, GL_FALSE, value_ptr(projection));

		texture_AO->bind();

		glLineWidth(10);
		// X-axis + Y-axis
		glUniform1i(d_uniforms["primitive_id"].position, PRIMITIVE_LINE);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 12 + (GLuint*)0);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 14 + (GLuint*)0);
		// O + X + Y
		glUniform1i(d_uniforms["primitive_id"].position, PRIMITIVE_CIRCLE_POINT);
		glDrawElements(GL_POINTS, 3, GL_UNSIGNED_INT, 13 + (GLuint*)0);

		GL_ERROR_STOP();
		
		gl_context.swapBuffers();
	}
	return 0;
}
TEST(l, r)
{
	tmain();
}