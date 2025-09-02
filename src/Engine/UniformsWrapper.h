#pragma once

template<typename T>
class UniformsWrapper final
{
public:
	static_assert(alignof(T) <= 16, "T must be aligned to 16 bytes or less");
	static_assert(sizeof(T) % 16 == 0, "Size of T should be multiple of 16 for std140");

	UniformsWrapper() = default;
	UniformsWrapper(const UniformsWrapper&) = delete;
	UniformsWrapper& operator=(const UniformsWrapper&) = delete;
	UniformsWrapper(UniformsWrapper&&) noexcept = default;
	UniformsWrapper& operator=(UniformsWrapper&&) noexcept = default;
	~UniformsWrapper() { Close(); }

	void Init()
	{
		m_ubo = gl::Buffer(sizeof(T), gl::BufferStorageFlag::DynamicStorage);
	}

	void Close()
	{
		m_ubo = std::nullopt;
		m_needsUpdate = true;
	}

	void Update()
	{
		if (!m_ubo) Init();
		m_needsUpdate = false;
		m_ubo->UpdateData(m_data);
	}

	void Bind(uint32_t index)
	{
		if (m_needsUpdate) Update();
		gl::Cmd::BindUniformBuffer(index, *m_ubo);
	}

	T* operator->() { m_needsUpdate = true; return &m_data; }
	T& operator*() { m_needsUpdate = true; return m_data; }

private:
	alignas(16) T             m_data{};
	std::optional<gl::Buffer> m_ubo;
	bool                      m_needsUpdate{ true };
};