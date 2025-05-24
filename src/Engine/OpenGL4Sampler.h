#pragma once

#include "OpenGL4Core.h"

namespace gl4
{
	namespace detail
	{
		class SamplerCache;
	}

	/// @brief Parameters for the constructor of Sampler
	struct SamplerState final
	{
		bool operator==(const SamplerState&) const noexcept = default;

		float lodBias{ 0 };
		float minLod{ -1000 };
		float maxLod{ 1000 };

		MinFilter minFilter{ MinFilter::Linear };
		MagFilter magFilter{ MagFilter::Linear };
		AddressMode addressModeU{ AddressMode::ClampToEdge };
		AddressMode addressModeV{ AddressMode::ClampToEdge };
		AddressMode addressModeW{ AddressMode::ClampToEdge };
		BorderColor borderColor{ BorderColor::FloatOpaqueWhite };
		SampleCount anisotropy{ SampleCount::Samples1 };
		bool compareEnable{ false };
		CompareOp compareOp{ CompareOp::Never };
	};

	/// @brief Encapsulates an OpenGL sampler
	class Sampler final
	{
	public:
		explicit Sampler(const SamplerState& samplerState);

		[[nodiscard]] bool IsValid() const noexcept { return m_id > 0; }

		[[nodiscard]] GLuint Handle() const noexcept { return m_id; }
		[[nodiscard]] operator GLuint() const noexcept { return m_id; }

	private:
		friend detail::SamplerCache;
		Sampler() = default; // you cannot create samplers out of thin air
		explicit Sampler(uint32_t id) : m_id(id) {}

		GLuint m_id{};
	};
} // namespace gl4