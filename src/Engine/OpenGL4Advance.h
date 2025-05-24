#pragma once

#include "OpenGL4Simple.h"

namespace gl4
{
	class Query final
	{
	public:
		Query(GLenum type);
		Query(const Query&) = delete;
		Query(Query&&) = default;
		~Query();

		Query& operator=(const Query&) = delete;

		void Begin();
		void End();

	private:
		GLenum m_type;
		GLuint m_id;
	};
} // namespace gl4A