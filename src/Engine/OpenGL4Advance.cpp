﻿#include "stdafx.h"
#include "OpenGL4Advance.h"
//=============================================================================
gl4::Query::Query(GLenum type)
{
	m_type = type;
	glCreateQueries(m_type, 1, &m_id);
}
//=============================================================================
gl4::Query::~Query()
{
	glDeleteQueries(1, &m_id);
}
//=============================================================================
void gl4::Query::Begin()
{
	glBeginQuery(m_type, m_id);
}
//=============================================================================
void gl4::Query::End()
{
	glEndQuery(m_type);
}
//=============================================================================