#pragma once

#define SE_SCOPED_SAMPLE(name) profiler::ScopedProfile __FILE__##__LINE__(name)

namespace profiler
{
	struct ScopedProfile final
	{
		ScopedProfile(const std::string& Name);
		~ScopedProfile();

		std::string name;
	};

	void Init();
	void Close();
	void BeginSample(const std::string& name);
	void EndSample(const std::string& name);
	void BeginFrame();
	void EndFrame();

	void Ui();

} // namespace profiler