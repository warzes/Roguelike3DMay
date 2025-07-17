это еще https://github.com/Brooklyn-Dev/ray-tracing/blob/master/shaders/fragment.glsl



===============================================================================
OpenGL
===============================================================================

-------------------------------------------------------------------------------
FBO
-------------------------------------------------------------------------------
сейчас FBO создаются неявно внутри gl4::BeginRendering и аналогов. Возможно сделать возможность создавать самому и также передавать их в gl4::BeginRendering.

-------------------------------------------------------------------------------
OpenGL4Shader
-------------------------------------------------------------------------------
- в архитектуру в стиле вулкана хорошо кладутся разделенные шейдеры, вместо Program. Переделать

-------------------------------------------------------------------------------
OpenGL4Buffer
-------------------------------------------------------------------------------
- из OpenGL4Simple взять методы:
	- динамически изменяемый буфер (сейчас создается StorageBuffer, но иногда нужна возможность менять размер)
	- маппинг по факту (сейчас буфер мапится при создании и закрывается при удалении. сделать возможность мапить по месту. также проверить разницу производительности. qwen говорит что разницы особо нет)
	- возможно SetSubData/CopySubData/ClearData/ClearSubData/InvalidateSubData/MapRange/FlushMappedRange/GetBufferPointer/GetSubData/BindBufferBase/BindBufferRange




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