
#include <helper/OpenGL_Utils.h>
#include <helper/OpenGL_Objects.h>
#include <ImGUI/ImGUI_Utils.h>
#include <cmath>

Universal_Type_Wrapper<int> ui("test int", 1, 1, 99, 10);
Universal_Type_Wrapper<float> uf("test float", 1, 1, 99);
int unwrapped_i;
float unwrapped_f;
Universal_Type_Wrapper<int &> wi("wrapped int", std::ref(unwrapped_i), 1, 99);
Universal_Type_Wrapper<float &> wf("wrapped float", std::ref(unwrapped_f), 1, 1, 99);

struct test_Group_Type
{
	Universal_Type_Wrapper<int> ui = {"inside test int", 1, 1, 99, 10};
	Universal_Type_Wrapper<float> uf = {"inside test float", 1, 1, 99};
	auto GetAllAttr()
	{
		return std::tie(ui, uf);
	}
} g_t;
Universal_Group_Wrapper<test_Group_Type&> w_g_t("grouped attributes", g_t);
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

	glViewport(0, 0, window_width / 2, window_height / 2);

	// clang-format off
	ProgramObject program = Helper::CreateProgram(
		ShaderObject(GL_VERTEX_SHADER, readFile("../media/shaders/"s + "quick_use_simple" + "/this.vs.glsl")),
		ShaderObject(GL_FRAGMENT_SHADER, readFile("../media/shaders/"s +"quick_use_simple" + "/this.fs.glsl"))
		);

	constexpr float L=0.8,R=-L;
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
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		vao->bind();
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLuint *)(0) + 3);
		vao->unbind();
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		static float f = 0.0f;
		static int counter = 0;
		using namespace ImGui;
		bool show_crash_window = true;
		static float fs[10] = {
			0,
		};
		Begin("test");
		BeginTabBar("Sad");
		if (BeginTabItem("b"))
		{
			using UT = decltype(ui);
			using T = UT::ValueType;
			using DT = UT::DT;
			constexpr bool x = std::is_same_v<DT, int>;
			auto constexpr xx = Visible_Attr_Type<UT>;
			auto constexpr xsx = Qualified_Be_Wrapped<test_Group_Type>;
			auto constexpr xssx = std::is_class_v<test_Group_Type&>;

			static_assert(x);
			Draw_element(ui);
			Draw_element(uf);
			Draw_element(wi);
			Draw_element(wf);
			Draw_element(w_g_t);

			EndTabItem();
		}
		EndTabBar();
		End();
		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}
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
TEST(l, r)
{
	constexpr bool iss[] = {
		Visible_Attr_Type<Universal_Type_Wrapper<int>>,
		// Visible_Attr_Type<Universal_Type_Wrapper<float>>
	};
	tmain();
}
