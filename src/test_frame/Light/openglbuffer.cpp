#include <Light/buffer.hpp>

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

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : m_count(count)
	{
		glGenBuffers(1, &m_rendererId);
		bind();

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
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
