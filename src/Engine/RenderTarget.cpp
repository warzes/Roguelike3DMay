#include "stdafx.h"
#include "RenderTarget.h"
//=============================================================================
void RenderTarget::Init(uint16_t width, uint16_t height, RTAttachment colors, std::optional<RTAttachment> depth)
{
	Init(width, height, { &colors, 1 }, depth);
}
//=============================================================================
void RenderTarget::Init(uint16_t width, uint16_t height, std::span<RTAttachment> colors, std::optional<RTAttachment> depth)
{
	Close();

	m_width = width;
	m_height = height;

	m_colorTex.resize(colors.size());
	for (size_t i = 0; i < colors.size(); i++)
	{
		m_colorTex[i] = {
			.name    = colors[i].name, 
			.texture = gl::CreateTexture2D({ m_width, m_height }, colors[i].format, colors[i].name),
			.loadOp  = colors[i].loadOp
		};
	}

	if (depth.has_value())
	{
		assert(
			depth->format == gl::Format::D32_FLOAT ||
			depth->format == gl::Format::D32_UNORM ||
			depth->format == gl::Format::D24_UNORM ||
			depth->format == gl::Format::D16_UNORM ||
			depth->format == gl::Format::D32_FLOAT_S8_UINT ||
			depth->format == gl::Format::D24_UNORM_S8_UINT ||
			depth->format == gl::Format::S8_UINT);

		m_depthTex = {
			.name = depth->name,
			.texture = gl::CreateTexture2D({ width, height }, depth->format, depth->name),
			.loadOp = depth->loadOp
		};
	}
}
//=============================================================================
void RenderTarget::Close()
{
	for (size_t i = 0; i < m_colorTex.size(); i++)
	{
		m_colorTex[i].texture = std::nullopt;
	}
	m_colorTex.clear();
	m_depthTex.texture = std::nullopt;
}
//=============================================================================
void RenderTarget::SetSize(uint16_t width, uint16_t height)
{
	if (m_width == width && m_height == height) return;
	m_width = width;
	m_height = height;

	for (size_t i = 0; i < m_colorTex.size(); i++)
	{
		auto format = m_colorTex[i].texture->GetFormat();
		m_colorTex[i].texture = gl::CreateTexture2D({ m_width, m_height }, format, m_colorTex[i].name);
	}
	if (m_depthTex.texture.has_value())
	{
		auto format = m_depthTex.texture->GetFormat();
		m_depthTex.texture = gl::CreateTexture2D({ m_width, m_height }, format, m_depthTex.name);
	}
}
//=============================================================================
void RenderTarget::Begin(const glm::vec3& clearColor, float clearDepth)
{
	gl::RenderInfo ri{};

	std::vector<gl::RenderColorAttachment> colorAttachments;

	// colors
	if (m_colorTex.size() > 0)
	{
		// TODO: выделять память при инициализации, а не в кадре

		colorAttachments.reserve(m_colorTex.size());
		for (size_t i = 0; i < m_colorTex.size(); i++)
		{
			colorAttachments.push_back({
				.texture = *m_colorTex[i].texture,
				.loadOp = m_colorTex[i].loadOp,
				.clearValue = { clearColor, 1.0f } });
		}

		ri.colorAttachments = colorAttachments;
	}

	// depths
	if (m_depthTex.texture.has_value())
	{
		ri.depthAttachment = gl::RenderDepthStencilAttachment{
			.texture    = *m_depthTex.texture,
			.loadOp     = m_depthTex.loadOp,
			.clearValue = {.depth = clearDepth},
		};
	}
	
	gl::BeginRendering(ri);
}
//=============================================================================
void RenderTarget::End()
{
	gl::EndRendering();
}
//=============================================================================
const gl::Texture* RenderTarget::GetColor(size_t id) const
{
	assert(id < m_colorTex.size());
	assert(m_colorTex[id].texture.has_value());

	return &m_colorTex[id].texture.value();
}
//=============================================================================
const gl::Texture* RenderTarget::GetDepth() const
{
	if (!m_depthTex.texture.has_value()) return nullptr;
	return &m_depthTex.texture.value();
}
//=============================================================================
void RenderTarget::BlitToSwapChain(size_t id)
{
	// TODO: есть возможность задавать оффсет и размер (например чтобы блитило только на часть экрана. сделать такое)
	extern uint16_t GetWindowWidth();
	extern uint16_t GetWindowHeight();
	gl::BlitTextureToSwapChain(*GetColor(id), {}, {}, GetExtent(), {GetWindowWidth(), GetWindowHeight(), 1}, gl::MagFilter::Nearest);
}
//=============================================================================