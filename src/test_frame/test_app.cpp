
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>
#include <cmath>
#include <ImGUI/Shader_Context.h>
#include <ImGUI/CROP.h>
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
	// cc.AddTask(std::shared_ptr<NV12_to_RGB>(
	// 	new NV12_to_RGB("NV12_to_RGB",
	// 					readFile("../src/test_frame/glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.vs.glsl"),
	// 					readFile("../src/test_frame/glsl/HANDSOUT/nv12_t0_rgb/nv12_t0_rgb.fs.glsl"),
	// 					&cc),
	// 	&I_Render_Task::I_Render_Task_Deleter));
	cc.AddTask(std::shared_ptr<Test_Render_Task>(
		new Test_Render_Task("Test_Render_Task",
							 readFile("../media/shaders/quick_use_simple/this.vs.glsl"),
							 readFile("../media/shaders/quick_use_simple/this.fs.glsl"),
							 &cc),
		&I_Render_Task::I_Render_Task_Deleter));
		cc.AddTask(std::shared_ptr<I_Render_Task>(new CROP("crop",
						readFile("../src/test_frame/glsl/HANDSOUT/crop/crop.vs.glsl"),
						readFile("../src/test_frame/glsl/HANDSOUT/crop/crop.fs.glsl"),
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
		cc.Tick();
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