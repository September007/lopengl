#include <Light/buffer.hpp>
#include <GLFW/glfw3.h>
#include <helper/msg_center.h>
namespace Light
{
	VertexBuffer* VertexBuffer::create(float* vertices, uint32_t size)
	{
		return new OpenGLVertexBuffer(vertices, size);
	}

	IndexBuffer* IndexBuffer::create(uint32_t* indices, uint32_t count)
	{
		return new OpenGLIndexBuffer(indices, count);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		glGenBuffers(1, &m_rendererId);
		bind();

		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	// xucl: note there we change the mean of count to sizeof buffer
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : m_count(count/sizeof(uint32_t))
	{
		glGenBuffers(1, &m_rendererId);
		bind();
 
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count , indices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_rendererId);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_rendererId);
	}

	void OpenGLVertexBuffer::bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererId);
	}

	void OpenGLVertexBuffer::unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId);
	}

	void OpenGLIndexBuffer::unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	uint32_t OpenGLIndexBuffer::getCount() const
	{
		return m_count;
	}
}



namespace Light
{
	GLenum Shader2OpenGLType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4: 	return GL_FLOAT;
		case ShaderDataType::Int:
		case ShaderDataType::Int2:
		case ShaderDataType::Int3:
		case ShaderDataType::Int4: 		return GL_INT;
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4: 		return GL_FLOAT;
		case ShaderDataType::Bool: 		return GL_BOOL;
		default:
			LIGHT_CORE_ERROR("Unsupported Shader data type");
			return 0;
		}
	}

	VertexArray* VertexArray::create()
	{
		return new OpenGLVertexArray();
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glGenVertexArrays(1, &m_rendererId);
	}

	OpenGLVertexArray::~OpenGLVertexArray() = default;

	void OpenGLVertexArray::bind() const
	{
		glBindVertexArray(m_rendererId);
	}

	void OpenGLVertexArray::unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vbo)
	{
		if(vbo->getLayout().getElements().size() == 0)
		{
			LIGHT_CORE_ERROR("Buffer Layout not set!");
			return;
		}

		glBindVertexArray(m_rendererId);
		vbo->bind();

		const auto& layout = vbo->getLayout();

		uint32_t index = 0;
		for(const auto& element: layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.getComponentCount(),
				Shader2OpenGLType(element.getType()),
				element.isNormalized() ? GL_TRUE : GL_FALSE,
				layout.getStride(),
				INT2VOIDP(element.getOffset()));
			index++;
		}
		m_vertexBuffers.push_back(vbo);
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& ibo)
	{
		glBindVertexArray(m_rendererId);
		ibo->bind();
        m_indexBuffer = ibo;
		glBindVertexArray(0);
	}
}

namespace Light
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_windowHandle(windowHandle)
	{
		if(!windowHandle)
		{
			LIGHT_CORE_CRITICAL("Could not create OpenGL Context: Handle is NULL");
			exit(1);
		}
	}

	OpenGLContext::~OpenGLContext() = default;

#if 0
	static void glDebugCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar* message,
		[[maybe_unused]] const void* userParam)
	{
		auto const src_str = [&]() -> std::string
		{
			switch(source)
			{
				case GL_DEBUG_SOURCE_API: return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
				case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
				case GL_DEBUG_SOURCE_APPLICATION: return "Application";
				case GL_DEBUG_SOURCE_OTHER: return "Other";
				default: return "Unknown";
			}
		}();

		auto const type_str = [&]() -> std::string
		{
			switch(type)
			{
				case GL_DEBUG_TYPE_ERROR: return "Error";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behavior";
				case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
				case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
				case GL_DEBUG_TYPE_MARKER: return "Marker";
				case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
				case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
				case GL_DEBUG_TYPE_OTHER: return "Other";
				default: return "Unknown";
			}
		}();

		auto const severity_str = [&]() -> std::string
		{
			switch(severity)
			{
				case GL_DEBUG_SEVERITY_HIGH: return "High";
				case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
				case GL_DEBUG_SEVERITY_LOW: return "Low";
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
				default: return "Unknown";
			}
		}();

		std::string msg;
		msg.reserve(length);
		msg.assign(message, length);

		if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		{
			LIGHT_CORE_INFO("OpenGL({0}, {1}, {2}, {3}): {4}", id, src_str, type_str, severity_str, msg);
		}
		else if(severity == GL_DEBUG_SEVERITY_HIGH)
		{
			LIGHT_CORE_ERROR("OpenGL({0}, {1}, {2}, {3}): {4}", id, src_str, type_str, severity_str, msg);
		}
		else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
		{
			LIGHT_CORE_WARN("OpenGL({0}, {1}, {2}, {3}): {4}", id, src_str, type_str, severity_str, msg);
		}
		else if(severity == GL_DEBUG_SEVERITY_LOW)
		{
			LIGHT_CORE_DEBUG("OpenGL({0}, {1}, {2}, {3}): {4}", id, src_str, type_str, severity_str, msg);
		}
	}
#endif

	void OpenGLContext::init()
	{
		glfwMakeContextCurrent(m_windowHandle);
		// xucl load this only one time
		static ScopeObject one_time_init{
			[]
			{
				int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
				if (!version)
				{
					LIGHT_CORE_CRITICAL("Could not initialize GLAD");
					exit(1);
				}
			},
			[] {}};
		CurrentContext()=this;

#if 0
		if(GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
		}
		else if (GLAD_GL_ARB_debug_output)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallbackARB(glDebugCallback, nullptr);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, nullptr, GL_FALSE);
		}
#endif

		LIGHT_CORE_INFO("OpenGL Vendor: {0}", glGetString(GL_VENDOR));
		LIGHT_CORE_INFO("OpenGL Device: {0}", glGetString(GL_RENDERER));
		LIGHT_CORE_INFO("OpenGL Version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::swapBuffers()
	{
		glfwSwapBuffers(m_windowHandle);
	}
}
