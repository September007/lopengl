
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>
#include <cmath>
#include <ImGUI/Shader_Context.h>

static void glfw_error_callback(int error, const char *description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
constexpr int window_width = 1440, window_height = 900;
using namespace std::string_literals;

int tmain()
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	// Decide GL+GLSL versions
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if (window == NULL)
		return 1;
	Light::OpenGLContext context = window;
	context.init();

	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	bool show_demo_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	CentralController cc;
	// cc.tasks.push_back(std::shared_ptr<NV12_to_RGB>(
	// 	new NV12_to_RGB("NV12_to_RGB",
	// 					readFile("./glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.vs.glsl"),
	// 					readFile("./glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.fs.glsl"),
	// 					&cc),
	// 	&I_Render_Task::I_Render_Task_Deleter));
	cc.tasks.push_back(std::shared_ptr<Test_Render_Task>(
		new Test_Render_Task("Test_Render_Task",
							 readFile("../media/shaders/quick_use_simple/this.vs.glsl"),
							 readFile("../media/shaders/quick_use_simple/this.fs.glsl"),
							 &cc),
		&I_Render_Task::I_Render_Task_Deleter));
	// Main loop
	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Shader Tasks Prepare and execute
		// cc.Tick();
		auto t = dynamic_cast<Test_Render_Task *>(cc.tasks[0].get());
		switch (0)
		{
		case 0:
			cc.Tick();
			break;
		case 1:
			t->PrepareExecutingParameters();
			t->Execute();
			t->ShowConfig();
			break;
		case 2:
			auto &vao = t->vao;
			auto &vbo = t->vbo;
			auto &veo = t->veo;
			auto &tex = t->tex;
			auto &program = t->program;
			auto &params = t->params;

			program = Helper::CreateProgram(ShaderObject(GL_VERTEX_SHADER, readFile(params->vsSrc.data)),
											ShaderObject(GL_FRAGMENT_SHADER, readFile(params->fsSrc.data)));
			program.use();
			std::tie(vao, vbo, veo) = simpleV_ABE_O<3>();

			Light::BufferLayout layout = {
				Light::BufferElement(Light::ShaderDataType::Float3, "aPos", false),
				Light::BufferElement(Light::ShaderDataType::Float2, "aTexCoord", false)};
			tex = Helper::CreateTexture(GL_TEXTURE1, params->shader_params->texturePath.data);
			program.prepareVBO(*vbo.get());
			glClearColor(0.2, 0.2, 0.0, 1);
			program.setInt(params->shader_params->texturePath.GetName(), tex.targetTexture - GL_TEXTURE0);

			vbo->setLayout(layout);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(veo);
			program.unuse();

			program.use();
			vao->bind();
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLuint *)(0) + 3);
			vao->unbind();

			glUseProgram(0);

			program.unuse();
			t->ShowConfig();
		};

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glClear(GL_COLOR_BUFFER_BIT);
		//  enable shutdown
		context.swapBuffers();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

#include <gtest/gtest.h>

TEST(test_app, main)
{
	tmain();
}