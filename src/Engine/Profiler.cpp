#include "stdafx.h"
#include "Profiler.h"
#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 5039)
#endif
#if defined(_WIN32)
#	undef APIENTRY
#	include <Windows.h> // TODO:
#endif
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
//=============================================================================
#define BUFFER_COUNT 3
#define MAX_SAMPLES 256
//=============================================================================
struct Profiler final
{
	struct Sample final
	{
		Sample()
		{
			glGenQueries(1, &query); // TODO: glCreateQueries
		}
		~Sample()
		{
			glDeleteQueries(1, &query);
		}


		std::string name;
		GLuint      query;
		bool        start = true;
		double      cpu_time;
		Sample*     end_sample;
	};

	struct Buffer final
	{
		std::vector<std::unique_ptr<Sample>> samples;
		size_t                               index = 0u;

		Buffer()
		{
			samples.resize(MAX_SAMPLES);

			for (uint32_t i = 0; i < MAX_SAMPLES; i++)
				samples[i] = nullptr;
		}
	};

	Profiler()
	{
#ifdef _WIN32
		QueryPerformanceFrequency(&m_frequency);
#endif
	}

	void BeginSample(std::string name)
	{
		size_t idx = m_sampleBuffers[m_writeBufferIdx].index++;

		if (!m_sampleBuffers[m_writeBufferIdx].samples[idx])
			m_sampleBuffers[m_writeBufferIdx].samples[idx] = std::make_unique<Sample>();

		auto& sample = m_sampleBuffers[m_writeBufferIdx].samples[idx];

		sample->name = name;
		glQueryCounter(sample->query, GL_TIMESTAMP);
		sample->end_sample = nullptr;
		sample->start = true;

#ifdef _WIN32
		LARGE_INTEGER cpu_time;
		QueryPerformanceCounter(&cpu_time);
		sample->cpu_time = (double)cpu_time.QuadPart * (1000000.0 / (double)m_frequency.QuadPart);
#else
		timeval cpu_time;
		gettimeofday(&cpu_time, nullptr);
		sample->cpu_time = (cpu_time.tv_sec * 1000000.0) + cpu_time.tv_usec;
#endif

		m_sampleStack.push(sample.get());
	}

	void EndSample(std::string name)
	{
		size_t idx = m_sampleBuffers[m_writeBufferIdx].index++;

		if (!m_sampleBuffers[m_writeBufferIdx].samples[idx])
			m_sampleBuffers[m_writeBufferIdx].samples[idx] = std::make_unique<Sample>();

		auto& sample = m_sampleBuffers[m_writeBufferIdx].samples[idx];

		sample->name = name;
		sample->start = false;
		glQueryCounter(sample->query, GL_TIMESTAMP);
		sample->end_sample = nullptr;

#ifdef _WIN32
		LARGE_INTEGER cpu_time;
		QueryPerformanceCounter(&cpu_time);
		sample->cpu_time = (double)cpu_time.QuadPart * (1000000.0 / (double)m_frequency.QuadPart);
#else
		timeval cpu_time;
		gettimeofday(&cpu_time, nullptr);
		sample->cpu_time = (cpu_time.tv_sec * 1000000.0) + cpu_time.tv_usec;
#endif

		Sample* start = m_sampleStack.top();

		start->end_sample = sample.get();

		m_sampleStack.pop();
	}

	void BeginFrame()
	{
		m_readBufferIdx++;
		m_writeBufferIdx++;

		if (m_readBufferIdx == 3)
			m_readBufferIdx = 0;

		if (m_writeBufferIdx == 3)
			m_writeBufferIdx = 0;
	}

	void EndFrame()
	{
		if (m_readBufferIdx >= 0)
			m_sampleBuffers[m_readBufferIdx].index = 0;
	}

	void Ui()
	{
		if (m_readBufferIdx >= 0)
		{
			for (size_t i = 0; i < m_sampleBuffers[m_readBufferIdx].index; i++)
			{
				auto& sample = m_sampleBuffers[m_readBufferIdx].samples[i];

				if (sample->start)
				{
					if (!m_shouldPopStack.empty())
					{
						if (!m_shouldPopStack.top())
						{
							m_shouldPopStack.push(false);
							continue;
						}
					}

					std::string id = std::to_string(i);

					uint64_t start_time = 0;
					uint64_t end_time = 0;

					glGetQueryObjectui64v(sample->query, GL_QUERY_RESULT, &start_time);
					glGetQueryObjectui64v(sample->end_sample->query, GL_QUERY_RESULT, &end_time);

					uint64_t gpu_time_diff = end_time - start_time;

					float gpu_time = float((double)gpu_time_diff / 1000000.0);
					float cpu_time = float(sample->end_sample->cpu_time - sample->cpu_time) * 0.001f;

					if (ImGui::TreeNode(id.c_str(), "%s | %f ms (CPU) | %f ms (GPU)", sample->name.c_str(), cpu_time, gpu_time))
						m_shouldPopStack.push(true);
					else
						m_shouldPopStack.push(false);
				}
				else
				{
					if (!m_shouldPopStack.empty())
					{
						bool should_pop = m_shouldPopStack.top();
						m_shouldPopStack.pop();

						if (should_pop)
							ImGui::TreePop();
					}
				}
			}
		}
	}

	int32_t             m_readBufferIdx = -3;
	int32_t             m_writeBufferIdx = -1;
	Buffer              m_sampleBuffers[BUFFER_COUNT];
	std::stack<Sample*> m_sampleStack;
	std::stack<bool>    m_shouldPopStack;

#ifdef _WIN32
	LARGE_INTEGER       m_frequency;
#endif
};
//=============================================================================
Profiler* g_profiler = nullptr;
//=============================================================================
profiler::ScopedProfile::ScopedProfile(const std::string& Name)
	: name(Name)
{
	BeginSample(name);
}
//=============================================================================
profiler::ScopedProfile::~ScopedProfile()
{
	EndSample(name);
}
//=============================================================================
void profiler::Init()
{
	g_profiler = new Profiler();
}
//=============================================================================
void profiler::Close()
{
	delete g_profiler; g_profiler = nullptr;
}
//=============================================================================
void profiler::BeginSample(const std::string& name)
{
	g_profiler->BeginSample(name);
}
//=============================================================================
void profiler::EndSample(const std::string& name)
{
	g_profiler->EndSample(name);
}
//=============================================================================
void profiler::BeginFrame()
{
	g_profiler->BeginFrame();
}
//=============================================================================
void profiler::EndFrame()
{
	g_profiler->EndFrame();
}
//=============================================================================
void profiler::Ui()
{
	g_profiler->Ui();
}
//=============================================================================