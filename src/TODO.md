===============================================================================
OpenGL
===============================================================================

-------------------------------------------------------------------------------
OpenGL4Shader
-------------------------------------------------------------------------------
- в архитектуру в стиле вулкана хорошо кладутся разделенные шейдеры, вместо Program. Переделать







===============================================================================
OLD
===============================================================================
https://github.com/FatemehAmereh/SSR
https://github.com/xtozero/SSR


Framebuffer
подумать как сделать создание фреймбуфера, с учетом что цветом/глубиной может быть как текстура, так и рендербуфер.
плюс надо еще правильно удалять
возможно стоит сделать такое
	struct GLTexture { GLuint id; }
	struct GLRenderBuffer { GLuint id; }
и для всего - чтобы не усложнять
тогда можно проще удалять, например GLDelete() которая правильно удаляет

что-то типа
void Renderer::switchDepthTestState(bool depth_test) {
	if (depth_test != _state.depthTest) {
		if (depth_test) {
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
		_state.depthTest = depth_test;
	}

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