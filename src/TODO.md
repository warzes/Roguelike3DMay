===============================================================================
OpenGL
===============================================================================
раздельные шейдеры
const char* vtx = ...
const char* frg = ...
const GLuint vs = glCreateShaderProgramv( GL_VERTEX_SHADER, 1,
&vtx);
const GLuint fs = glCreateShaderProgramv( GL_FRAGMENT_SHADER, 1,
&frg);
GLuint pipeline;
glCreateProgramPipelines(1, &pipeline);
glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vs);
glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fs);
glBindProgramPipeline(pipeline);