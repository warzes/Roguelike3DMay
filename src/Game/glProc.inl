#ifndef _GL_INIT_PROC_
#define _GL_INIT_PROC_

typedef char GLchar;
typedef unsigned int GLbitfield;
typedef signed long long int GLsizeiptr;
typedef signed long long int GLintptr;

// Shaders
#pragma region [ Shaders ]

#define GL_COMPUTE_SHADER 0x91B9
#define GL_VERTEX_SHADER 0x8B31
#define GL_TESS_CONTROL_SHADER 0x8E88
#define GL_TESS_EVALUATION_SHADER 0x8E87
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_FRAGMENT_SHADER 0x8B30

#define GL_SHADER_TYPE 0x8B4F
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_SHADER_SOURCE_LENGTH 0x8B88

#pragma endregion

// Program
#pragma region [ Program ]

#define GL_DELETE_STATUS 0x8B80
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS 0x92D9
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_COMPUTE_WORK_GROUP_SIZE 0x8267
#define GL_PROGRAM_BINARY_LENGTH 0x8741
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_GEOMETRY_VERTICES_OUT 0x8916
#define GL_GEOMETRY_INPUT_TYPE 0x8917
#define GL_GEOMETRY_OUTPUT_TYPE 0x8918

#pragma endregion

// Buffer
#pragma region [ Buffer ]

#define GL_ARRAY_BUFFER 0x8892
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_DISPATCH_INDIRECT_BUFFER 0x90EE
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_QUERY_BUFFER 0x9192
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_UNIFORM_BUFFER 0x8A11

#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_DYNAMIC_STORAGE_BIT 0x0100
#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_PERSISTENT_BIT 0x0040
#define GL_MAP_COHERENT_BIT 0x0080
#define GL_CLIENT_STORAGE_BIT 0x0200

#pragma endregion




#endif // _GL_INIT_PROC_

// Shaders
#pragma region [ Shaders ]

GL_PROC(GLuint, glCreateShader, GLuint shader)
GL_PROC(void, glDeleteShader, GLuint shader)
GL_PROC(void, glShaderSource, GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
GL_PROC(void, glCompileShader, GLuint shader)
GL_PROC(void, glGetShaderiv, GLuint shader, GLenum pname, GLint* params)
GL_PROC(void, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog)

#pragma endregion

// Program
#pragma region [ Program ]

GL_PROC(GLuint, glCreateProgram)
GL_PROC(void, glDeleteProgram, GLuint program)
GL_PROC(void, glAttachShader, GLuint program, GLuint shader)
GL_PROC(void, glDetachShader, GLuint program, GLuint shader)
GL_PROC(void, glLinkProgram, GLuint program)
GL_PROC(void, glUseProgram, GLuint program)
GL_PROC(void, glGetProgramiv, GLuint program, GLenum pname, GLint* params)
GL_PROC(void, glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog)

#pragma endregion

// Shaders Uniforms
#pragma region [ Shaders Uniforms ]

GL_PROC(GLint, glGetUniformLocation, GLuint program, const GLchar* name)
GL_PROC(void, glProgramUniform1f, GLuint program, GLint location, GLfloat v0)
GL_PROC(void, glProgramUniform2f, GLuint program, GLint location, GLfloat v0, GLfloat v1)
GL_PROC(void, glProgramUniform3f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
GL_PROC(void, glProgramUniform4f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
GL_PROC(void, glProgramUniform1i, GLuint program, GLint location, GLint v0)
GL_PROC(void, glProgramUniform2i, GLuint program, GLint location, GLint v0, GLint v1)
GL_PROC(void, glProgramUniform3i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
GL_PROC(void, glProgramUniform4i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
GL_PROC(void, glProgramUniform1ui, GLuint program, GLint location, GLuint v0)
GL_PROC(void, glProgramUniform2ui, GLuint program, GLint location, GLuint v0, GLuint v1)
GL_PROC(void, glProgramUniform3ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
GL_PROC(void, glProgramUniform4ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
GL_PROC(void, glProgramUniform1fv, GLuint program, GLint location, GLsizei count, const GLfloat* value)
GL_PROC(void, glProgramUniform2fv, GLuint program, GLint location, GLsizei count, const GLfloat* value)
GL_PROC(void, glProgramUniform3fv, GLuint program, GLint location, GLsizei count, const GLfloat* value)
GL_PROC(void, glProgramUniform4fv, GLuint program, GLint location, GLsizei count, const GLfloat* value)
GL_PROC(void, glProgramUniform1iv, GLuint program, GLint location, GLsizei count, const GLint* value)
GL_PROC(void, glProgramUniform2iv, GLuint program, GLint location, GLsizei count, const GLint* value)
GL_PROC(void, glProgramUniform3iv, GLuint program, GLint location, GLsizei count, const GLint* value)
GL_PROC(void, glProgramUniform4iv, GLuint program, GLint location, GLsizei count, const GLint* value)
GL_PROC(void, glProgramUniform1uiv, GLuint program, GLint location, GLsizei count, const GLuint* value)
GL_PROC(void, glProgramUniform2uiv, GLuint program, GLint location, GLsizei count, const GLuint* value)
GL_PROC(void, glProgramUniform3uiv, GLuint program, GLint location, GLsizei count, const GLuint* value)
GL_PROC(void, glProgramUniform4uiv, GLuint program, GLint location, GLsizei count, const GLuint* value)
GL_PROC(void, glProgramUniformMatrix2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix2x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix3x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix2x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix4x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix3x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_PROC(void, glProgramUniformMatrix4x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)

#pragma endregion

// Buffer
#pragma region [ Buffer ]

	GL_PROC(void, glCreateBuffers, GLsizei n, GLuint* buffers)
	GL_PROC(void, glDeleteBuffers, GLsizei n, const GLuint* buffers)

GL_PROC(void, glNamedBufferData, GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
GL_PROC(void, glNamedBufferStorage, GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags)
GL_PROC(void, glNamedBufferSubData, GLuint buffer, GLintptr offset, GLsizei size, const void* data)
GL_PROC(void, glClearNamedBufferData, GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)



GL_PROC(void, glBindBuffer, GLenum target, GLuint buffer)
GL_PROC(void, glBindBufferRange, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)

#pragma endregion

// VAO
#pragma region [ VAO ]

	GL_PROC(void, glCreateVertexArrays, GLsizei n, GLuint* arrays)
	GL_PROC(void, glDeleteVertexArrays, GLsizei n, const GLuint* arrays)



GL_PROC(void, glVertexArrayVertexBuffer, GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
	GL_PROC(void, glVertexArrayAttribFormat, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
	GL_PROC(void, glVertexArrayAttribBinding, GLuint vaobj, GLuint attribindex, GLuint bindingindex)
	GL_PROC(void, glEnableVertexArrayAttrib, GLuint vaobj, GLuint index)
	GL_PROC(void, glBindVertexArray, GLuint array)


#pragma endregion