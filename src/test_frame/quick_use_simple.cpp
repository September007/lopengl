#include <gtest/gtest.h>
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <GLFW/glfw3.h>

constexpr int window_width = 1440, window_height = 900;
TEST(l, r)
{

	glfwInit();
	glfwWindowHint(GL_SAMPLES, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto window = glfwCreateWindow(window_width, window_height, "quick_use_simple", NULL, NULL);
	_ASSERT(window != nullptr);
	Light::OpenGLContext context = window;
	context.init();

	glViewport(0, 0, window_width, window_height);

	ProgramObject program = Helper::CreateProgram(
		ShaderObject(GL_VERTEX_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.vs.glsl")),
		ShaderObject(GL_FRAGMENT_SHADER, readFile("../media/shaders/" + getSrcFileNameOnlyName(__FILE__) + "/this.fs.glsl")));

	// clang-format off
	constexpr float L=0.8,R=-0.8;
	constexpr float TL=0,TR=1;

	float vertices[]={
		L,L,0,TL,TL,
		L,R,0,TL,TR,
		R,R,0,TR,TR,
		R,L,0,TR,TL,
	};
	uint32_t indices[]={
		0,1,2,
		0,2,3,
	};
	Light::BufferLayout layout = {
		Light::BufferElement(Light::ShaderDataType::Float3, "aPos", false),
		Light::BufferElement(Light::ShaderDataType::Float2, "aTexCoord", false)
		};
	// clang-format on
	auto vao = std::shared_ptr<Light::VertexArray>(Light::VertexArray::create());
	auto vbo = std::shared_ptr<Light::VertexBuffer>(Light::VertexBuffer::create(vertices, sizeof(vertices)));
	auto veo = std::shared_ptr<Light::IndexBuffer>(Light::IndexBuffer::create(indices, sizeof(indices)));

	vbo->setLayout(layout);
	vao->addVertexBuffer(vbo);
	vao->setIndexBuffer(veo);

	auto texture = Helper::CreateTexture(GL_TEXTURE0, "../media/texture/bmp/2004050204170.bmp");
	glUseProgram(program.getProgram());
	program.setInt("aTexture", 0);

	// set attribute location
	program.prepareVBO(*vbo.get());
	glClearColor(0.2, 0.2, 0.0, 1);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		// enable shutdown
		glfwPollEvents();
		vao->bind();
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLuint *)(0) + 3);
		vao->unbind();

		context.swapBuffers();
	}
	glfwTerminate();
}