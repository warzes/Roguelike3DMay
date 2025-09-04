#include "stdafx.h"
#include "Noise.h"
#include "Random.h"
#include "CoreMath.h"
//=============================================================================
float Noise::PerlinNoise(glm::vec3 pos, float freq)
{
	pos = pos * freq;
	glm::vec4 p = glm::vec4(pos, 0.0f);

	glm::vec4 Pi0 = glm::mod(glm::floor(p), freq);
	glm::vec4 Pi1 = glm::mod(Pi0 + 1.0f, freq);
	glm::vec4 Pf0 = math::Fract(p);
	glm::vec4 Pf1 = Pf0 - 1.0f;
	glm::vec4 ix = glm::vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
	glm::vec4 iy = glm::vec4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
	glm::vec4 iz0 = glm::vec4(Pi0.z);
	glm::vec4 iz1 = glm::vec4(Pi1.z);
	glm::vec4 iw0 = glm::vec4(Pi0.w);
	glm::vec4 iw1 = glm::vec4(Pi1.w);

	glm::vec4 ixy = Noise::permute(Noise::permute(ix) + iy);
	glm::vec4 ixy0 = Noise::permute(ixy + iz0);
	glm::vec4 ixy1 = Noise::permute(ixy + iz1);
	glm::vec4 ixy00 = Noise::permute(ixy0 + iw0);
	glm::vec4 ixy01 = Noise::permute(ixy0 + iw1);
	glm::vec4 ixy10 = Noise::permute(ixy1 + iw0);
	glm::vec4 ixy11 = Noise::permute(ixy1 + iw1);

	glm::vec4 gx00 = ixy00 / 7.0f;
	glm::vec4 gy00 = glm::floor(gx00) / 7.0f;
	glm::vec4 gz00 = glm::floor(gy00) / 6.0f;
	gx00 = math::Fract(gx00) - 0.5f;
	gy00 = math::Fract(gy00) - 0.5f;
	gz00 = math::Fract(gz00) - 0.5f;
	glm::vec4 gw00 = 0.5f - glm::abs(gx00) - glm::abs(gy00) - glm::abs(gz00);
	glm::vec4 sw00 = math::Step(glm::vec4(0.0f), gw00);
	gx00 = gx00 - sw00 * (math::Step(gx00, 0.0f) - 0.5f);
	gy00 = gy00 - sw00 * (math::Step(gy00, 0.0f) - 0.5f);

	glm::vec4 gx01 = ixy01 / 7.0f;
	glm::vec4 gy01 = glm::floor(gx01) / 7.0f;
	glm::vec4 gz01 = glm::floor(gy01) / 6.0f;
	gx01 = math::Fract(gx01) - 0.5f;
	gy01 = math::Fract(gy01) - 0.5f;
	gz01 = math::Fract(gz01) - 0.5f;
	glm::vec4 gw01 = 0.5f - glm::abs(gx01) - glm::abs(gy01) - glm::abs(gz01);
	glm::vec4 sw01 = math::Step(glm::vec4(0.0f), gw01);
	gx01 = gx01 - sw01 * (math::Step(gx01, 0.0f) - 0.5f);
	gy01 = gy01 - sw01 * (math::Step(gy01, 0.0f) - 0.5f);

	glm::vec4 gx10 = ixy10 / 7.0f;
	glm::vec4 gy10 = glm::floor(gx10) / 7.0f;
	glm::vec4 gz10 = glm::floor(gy10) / 6.0f;
	gx10 = math::Fract(gx10) - 0.5f;
	gy10 = math::Fract(gy10) - 0.5f;
	gz10 = math::Fract(gz10) - 0.5f;
	glm::vec4 gw10 = 0.5f - glm::abs(gx10) - glm::abs(gy10) - glm::abs(gz10);
	glm::vec4 sw10 = math::Step(glm::vec4(0.0f), gw10);
	gx10 = gx10 - sw10 * (math::Step(gx10, 0.0f) - 0.5f);
	gy10 = gy10 - sw10 * (math::Step(gy10, 0.0f) - 0.5f);

	glm::vec4 gx11 = ixy11 / 7.0f;
	glm::vec4 gy11 = glm::floor(gx11) / 7.0f;
	glm::vec4 gz11 = glm::floor(gy11) / 6.0f;
	gx11 = math::Fract(gx11) - 0.5f;
	gy11 = math::Fract(gy11) - 0.5f;
	gz11 = math::Fract(gz11) - 0.5f;
	glm::vec4 gw11 = 0.5f - glm::abs(gx11) - glm::abs(gy11) - glm::abs(gz11);
	glm::vec4 sw11 = math::Step(glm::vec4(0.0f), gw11);
	gx11 = gx11 - sw11 * (math::Step(gx11, 0.0f) - 0.5f);
	gy11 = gy11 - sw11 * (math::Step(gy11, 0.0f) - 0.5f);

	glm::vec4 g0000 = glm::vec4(gx00.x, gy00.x, gz00.x, gw00.x);
	glm::vec4 g1000 = glm::vec4(gx00.y, gy00.y, gz00.y, gw00.y);
	glm::vec4 g0100 = glm::vec4(gx00.z, gy00.z, gz00.z, gw00.z);
	glm::vec4 g1100 = glm::vec4(gx00.w, gy00.w, gz00.w, gw00.w);

	glm::vec4 g0010 = glm::vec4(gx10.x, gy10.x, gz10.x, gw10.x);
	glm::vec4 g1010 = glm::vec4(gx10.y, gy10.y, gz10.y, gw10.y);
	glm::vec4 g0110 = glm::vec4(gx10.z, gy10.z, gz10.z, gw10.z);
	glm::vec4 g1110 = glm::vec4(gx10.w, gy10.w, gz10.w, gw10.w);

	glm::vec4 g0001 = glm::vec4(gx01.x, gy01.x, gz01.x, gw01.x);
	glm::vec4 g1001 = glm::vec4(gx01.y, gy01.y, gz01.y, gw01.y);
	glm::vec4 g0101 = glm::vec4(gx01.z, gy01.z, gz01.z, gw01.z);
	glm::vec4 g1101 = glm::vec4(gx01.w, gy01.w, gz01.w, gw01.w);

	glm::vec4 g0011 = glm::vec4(gx11.x, gy11.x, gz11.x, gw11.x);
	glm::vec4 g1011 = glm::vec4(gx11.y, gy11.y, gz11.y, gw11.y);
	glm::vec4 g0111 = glm::vec4(gx11.z, gy11.z, gz11.z, gw11.z);
	glm::vec4 g1111 = glm::vec4(gx11.w, gy11.w, gz11.w, gw11.w);

	glm::vec4 norm00 = FastInvSqrt(glm::vec4(glm::dot(g0000, g0000), glm::dot(g0100, g0100), glm::dot(g1000, g1000), glm::dot(g1100, g1100)));
	g0000 = g0000 * norm00.x;
	g0100 = g0100 * norm00.y;
	g1000 = g1000 * norm00.z;
	g1100 = g1100 * norm00.w;
	glm::vec4 norm01 = FastInvSqrt(glm::vec4(glm::dot(g0001, g0001), glm::dot(g0101, g0101), glm::dot(g1001, g1001), glm::dot(g1101, g1101)));
	g0001 = g0001 * norm01.x;
	g0101 = g0101 * norm01.y;
	g1001 = g1001 * norm01.z;
	g1101 = g1101 * norm01.w;

	glm::vec4 norm10 = FastInvSqrt(glm::vec4(glm::dot(g0010, g0010), glm::dot(g0110, g0110), glm::dot(g1010, g1010), glm::dot(g1110, g1110)));
	g0010 = g0010 * norm10.x;
	g0110 = g0110 * norm10.y;
	g1010 = g1010 * norm10.z;
	g1110 = g1110 * norm10.w;

	glm::vec4 norm11 = FastInvSqrt(glm::vec4(glm::dot(g0011, g0011), glm::dot(g0111, g0111), glm::dot(g1011, g1011), glm::dot(g1111, g1111)));
	g0011 = g0011 * norm11.x;
	g0111 = g0111 * norm11.y;
	g1011 = g1011 * norm11.z;
	g1111 = g1111 * norm11.w;

	float n0000 = glm::dot(g0000, Pf0);
	float n1000 = glm::dot(g1000, glm::vec4(Pf1.x, Pf0.y, Pf0.z, Pf0.w));
	float n0100 = glm::dot(g0100, glm::vec4(Pf0.x, Pf1.y, Pf0.z, Pf0.w));
	float n1100 = glm::dot(g1100, glm::vec4(Pf1.x, Pf1.y, Pf0.z, Pf0.w));
	float n0010 = glm::dot(g0010, glm::vec4(Pf0.x, Pf0.y, Pf1.z, Pf0.w));
	float n1010 = glm::dot(g1010, glm::vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
	float n0110 = glm::dot(g0110, glm::vec4(Pf0.x, Pf1.y, Pf1.z, Pf0.w));
	float n1110 = glm::dot(g1110, glm::vec4(Pf1.x, Pf1.y, Pf1.z, Pf0.w));
	float n0001 = glm::dot(g0001, glm::vec4(Pf0.x, Pf0.y, Pf0.z, Pf1.w));
	float n1001 = glm::dot(g1001, glm::vec4(Pf1.x, Pf0.y, Pf0.z, Pf1.w));
	float n0101 = glm::dot(g0101, glm::vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
	float n1101 = glm::dot(g1101, glm::vec4(Pf1.x, Pf1.y, Pf0.z, Pf1.w));
	float n0011 = glm::dot(g0011, glm::vec4(Pf0.x, Pf0.y, Pf1.z, Pf1.w));
	float n1011 = glm::dot(g1011, glm::vec4(Pf1.x, Pf0.y, Pf1.z, Pf1.w));
	float n0111 = glm::dot(g0111, glm::vec4(Pf0.x, Pf1.y, Pf1.z, Pf1.w));
	float n1111 = glm::dot(g1111, Pf1);

	glm::vec4 fade = Noise::fade(Pf0);
	glm::vec4 n0w = glm::lerp(glm::vec4(n0000, n1000, n0100, n1100), glm::vec4(n0001, n1001, n0101, n1101), fade.w);
	glm::vec4 n1w = glm::lerp(glm::vec4(n0010, n1010, n0110, n1110), glm::vec4(n0011, n1011, n0111, n1111), fade.w);
	glm::vec4 nzw = glm::lerp(n0w, n1w, fade.z);
	glm::vec3 nyzw = glm::lerp(glm::vec3(nzw.x, nzw.y, 0.0f), glm::vec3(nzw.z, nzw.w, 0.0f), fade.y);
	float nxyzw = glm::lerp(nyzw.x, nyzw.y, fade.x);

	return 2.2f * nxyzw * 0.5f + 0.5f;
}
//=============================================================================
float Noise::PerlinFbm(const glm::vec3& pos, float freq, int octaveCount)
{
	float sum = 0.0f;
	float weightSum = 0.0f;
	float weight = 0.5f;

	for (int i = 0; i < octaveCount; i++)
	{
		float noise = Noise::PerlinNoise(pos, freq);

		sum += noise * weight;
		weightSum += weight;

		weight *= weight;
		freq *= 2.0f;
	}

	float fbm = glm::min(sum / weightSum, 1.0f);
	fbm = glm::max(fbm, 0.0f);
	return fbm;
}
//=============================================================================
float Noise::WorleyNoise(const glm::vec3& pos, float cellCount)
{
	const glm::vec3 pCell = pos * cellCount;
	float cellNoise = 1.0e10;

	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			for (int k = -1; k <= 1; k++)
			{
				glm::vec3 tp = glm::floor(pCell) + glm::vec3(i, j, k);
				glm::vec3 tpMod = glm::mod(tp, cellCount);
				glm::vec3 inte = glm::floor(tpMod);
				glm::vec3 frac = math::Fract(tpMod);

				frac = frac * frac * (3.0f - 2.0f * frac);
				float n = inte.x + inte.y * 57.0f + inte.z * 113.0f;
				float noise = glm::lerp(
					glm::lerp(
						glm::lerp(Random::Hash(n + 0.0f), Random::Hash(n + 1.0f), frac.x),
						glm::lerp(Random::Hash(n + 57.0f), Random::Hash(n + 58.0f), frac.x),
						frac.y),
					glm::lerp(
						glm::lerp(Random::Hash(n + 113.0f), Random::Hash(n + 114.0f), frac.x),
						glm::lerp(Random::Hash(n + 170.0f), Random::Hash(n + 171.0f), frac.x),
						frac.y),
					frac.z);

				tp = pCell - tp - noise;
				cellNoise = glm::min(cellNoise, glm::dot(tp, tp));
			}

	cellNoise = glm::min(cellNoise, 1.0f);
	cellNoise = glm::max(cellNoise, 0.0f);

	return 1.0f - cellNoise;
}
//=============================================================================
float Noise::WorleyFbm(const glm::vec3& pos, float cellCount, float freqs[3])
{
	float fbm = 0.0f;
	fbm += Noise::WorleyNoise(pos, cellCount * freqs[0]) * 0.625f;
	fbm += Noise::WorleyNoise(pos, cellCount * freqs[1]) * 0.25f;
	fbm += Noise::WorleyNoise(pos, cellCount * freqs[2]) * 0.125f;

	return fbm;
}
//=============================================================================
float Noise::CurlNoise(const glm::vec3& pos)
{
	return 0.0f;
}
//=============================================================================
glm::vec4 Noise::permute(const glm::vec4& a)
{
	return glm::mod(a * a * 34.0f + a, 289.0f);
}
//=============================================================================
glm::vec3 Noise::fade(const glm::vec3& a)
{
	return (a * a * a) * (a * a * 6.0f - 15.0f * a + 10.0f);
}
//=============================================================================
glm::vec4 Noise::fade(const glm::vec4& a)
{
	return (a * a * a) * (a * a * 6.0f - 15.0f * a + 10.0f);
}
//=============================================================================