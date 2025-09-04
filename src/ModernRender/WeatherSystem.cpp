#include "stdafx.h"
#include "WeatherSystem.h"
#include "GameLight.h"
#include "LightManager.h"
//=============================================================================
#define FIXEDUPDATE_TIME 0.01f//In seconds
//=============================================================================
bool WeatherSystem::Init(LightManager* lightManager)
{
	m_lightManager = lightManager;

	m_isTimeStop = true;

	//m_skySphereMesh.SetMeshData(MeshManager::Instance()->GetBuildinSphere());
	//m_skySphereMesh.SetMaterial(0);
	//m_skySphereMesh.SetScale(glm::vec3(10000.0f, 10000.0f, 10000.0f));
	//m_skySphereMesh.SetPosition(glm::vec3(0.0f));
	m_latitude = 0.0f;
	m_day = 180;
	m_hour = 12.0f;
	m_turbidity = 4.0f;
	m_exposure = 30.0f;
	m_timeSpeed = 150.0f;
	m_lightR = 3000.0f;
	m_updateSunPos = true;

	m_windDir = glm::vec4(0.5f, 0.0f, 1.0f, 10.0f);
	m_cloudBias = glm::vec3(0.0f);

	m_sunLightID = m_lightManager->CreateNewLight(LightType::Direction, 1.0f);
	GameLight* sun = m_lightManager->GetLight(m_sunLightID);
	sun->SetNearClip(0.01f);
	sun->SetFarClip(glm::max(m_lightR * 2.0f, 1000.0f));
	sun->SetLightSize(6);
	sun->SetShadowMapSize(2048);

	m_cloudMaxAltitude = 6000.0f;
	m_cloudMinAltitude = 1500.0f;
	m_cloudCoverage = 0.55f;
	m_cloudPrecipitation = 1.0f;
	m_updateCloudPos = true;
	GenerateCloudNoise();

	m_fogDensity = 0.02f;
	m_fogColor = glm::vec3(1.0f, 1.0f, 1.0f);
	m_fogMaxAltitude = 20.0f;
	m_fogPrecipitation = 20.0f;

	Update();

	return true;
}
//=============================================================================
void WeatherSystem::Close()
{

}
//=============================================================================
void WeatherSystem::updateAtmosphere()
{
	//X->E	-X->W
	//Z->S	-Z->N
	float l = m_latitude * glm::pi<float>()/180.0f;
	float paraA = 0.17f * sin(4.0f * glm::pi<float>() * (m_day - 80.0f) / 373.0f);
	float paraB = 0.129f * sin(2.0f * glm::pi<float>() * (m_day - 8.0f) / 355.0f);
	float t = m_hour + paraA - paraB;
	float delta = 0.4093f * sin(2.0f * glm::pi<float>() * (m_day - 81.0f) / 368.0f);
	m_thetaS = glm::pi<float>() * 0.5f - asin(sin(l) * sin(delta) - cos(l) * cos(delta) * cos(glm::pi<float>() * t / 12.0f));
	float Phi = atan((-cos(delta) * sin(glm::pi<float>() * t / 12.0f)) / (cos(l) * sin(delta) - sin(l) * cos(delta) * cos(glm::pi<float>() * t / 12.0f)));
	m_wsSunPos.y = m_lightR * cos(m_thetaS);
	m_wsSunPos.x = -m_lightR * sin(m_thetaS) * sin(Phi);
	m_wsSunPos.z = m_lightR * sin(m_thetaS) * cos(Phi);
	m_wsSunPos.w = 1.0f;

	float X = ((4.0f / 9.0f) - (m_turbidity / 120.0f)) * (glm::pi<float>() - 2.0f * m_thetaS);
	m_zenith.z = (4.0453f * m_turbidity - 4.9710f) * tan(X) - 0.2155f * m_turbidity + 2.4192f;

	float ThetaS_3 = pow(m_thetaS, 3.0f);
	float ThetaS_2 = pow(m_thetaS, 2.0f);
	float T_2 = m_turbidity * m_turbidity;
	glm::vec4 temp(T_2 * 0.00166f - m_turbidity * 0.02903f + 0.11693f,
		-T_2 * 0.00375f + m_turbidity * 0.06377f - 0.21196f,
		T_2 * 0.00209f - m_turbidity * 0.03202f + 0.06052f,
		m_turbidity * 0.00394f + 0.25886f);
	m_zenith.x = ThetaS_3 * temp.x + ThetaS_2 * temp.y + m_thetaS * temp.z + temp.w;

	temp = glm::vec4(T_2 * 0.00275f - m_turbidity * 0.04214f + 0.15346f,
		-T_2 * 0.00610f + m_turbidity * 0.08970f - 0.26756f,
		T_2 * 0.00317f - m_turbidity * 0.04156f + 0.06670f,
		m_turbidity * 0.00516f + 0.26688f);
	m_zenith.y = ThetaS_3 * temp.x + ThetaS_2 * temp.y + m_thetaS * temp.z + temp.w;

	GameLight* sun = m_lightManager->GetLight(m_sunLightID);
	sun->SetPosition(m_wsSunPos);
	sun->SetColor(getSunColor());
	sun->LookAt(glm::vec3(0.0f));
	m_lightManager->SetZenithColor(getZenithColor());
}
//=============================================================================
void WeatherSystem::updateCloud()
{
	m_cloudBias = glm::normalize(glm::vec3(m_windDir.x, m_windDir.y, m_windDir.z)) * m_windDir.w * FIXEDUPDATE_TIME * m_timeSpeed * (m_isTimeStop ? 0.0f : 1.0f) + m_cloudBias;
}
//=============================================================================
void WeatherSystem::Update()
{
	m_hour += FIXEDUPDATE_TIME * 0.0002778f * m_timeSpeed * (m_isTimeStop ? 0.0f : 1.0f);
	if (m_hour >= 24.0f)
	{
		m_hour = 0.0f;
		m_day += 1;
		if (m_day >= 365)
			m_day = 0;
	}

	if (m_updateSunPos)
		updateAtmosphere();

	if (m_updateCloudPos)
		updateCloud();
}
//=============================================================================
unsigned int WeatherSystem::GetSunLightID()
{
	return m_sunLightID;
}
//=============================================================================
float WeatherSystem::GetSunR()
{
	return m_lightR;
}
//=============================================================================
void WeatherSystem::SetSunR(float r)
{
	m_lightR = glm::max(r, 0.0f);
}
//=============================================================================
void WeatherSystem::SetLatitude(float l)
{
	m_latitude = l;
}
//=============================================================================
float WeatherSystem::GetLatitude()
{
	return m_latitude;
}
//=============================================================================
void WeatherSystem::SetDay(int d)
{
	m_day = d;
}
//=============================================================================
int WeatherSystem::GetDay()
{
	return m_day;
}
//=============================================================================
void WeatherSystem::SetHour(float h)
{
	m_hour = h;
}
//=============================================================================
float WeatherSystem::GetHour()
{
	return m_hour;
}
//=============================================================================
void WeatherSystem::SetTurbidity(float t)
{
	m_turbidity = t;
}
//=============================================================================
float WeatherSystem::GetTurbidity()
{
	return m_turbidity;
}
//=============================================================================
void WeatherSystem::SetExposure(float exp)
{
	m_exposure = exp;
}
//=============================================================================
float WeatherSystem::GetExposure()
{
	return m_exposure;
}
//=============================================================================
Model* WeatherSystem::GetSkySphereMesh()
{
	return &m_skySphereMesh;
}
//=============================================================================
void WeatherSystem::SetSkySphereMeshRotation(glm::vec3 r)
{
	//m_skySphereMesh.SetRotation(r);
}
//=============================================================================
glm::vec3 WeatherSystem::GetSkySphereMeshRotation()
{
	return {};// m_skySphereMesh.GetRotation();
}
//=============================================================================
void WeatherSystem::SetTimeSpeed(float t)
{
	m_timeSpeed = t;
}
//=============================================================================
float WeatherSystem::GetTimeSpeed()
{
	return m_timeSpeed;
}
//=============================================================================
void WeatherSystem::SetWindDirection(glm::vec3 d)
{
	m_windDir.x = d.x;
	m_windDir.y = d.y;
	m_windDir.z = d.z;
}
//=============================================================================
glm::vec3 WeatherSystem::GetWindDirection()
{
	return m_windDir;
}
//=============================================================================
void WeatherSystem::SetWindStrength(float s)
{
	m_windDir.w = s;
}
//=============================================================================
float WeatherSystem::GetWindStrength()
{
	return m_windDir.w;
}
//=============================================================================
void WeatherSystem::SetCloudMaxAltitude(float a)
{
	m_cloudMaxAltitude = glm::max(m_cloudMinAltitude, a);
}
//=============================================================================
float WeatherSystem::GetCloudMaxAltitude()
{
	return m_cloudMaxAltitude;
}
//=============================================================================
void WeatherSystem::SetCloudMinAltitude(float a)
{
	m_cloudMinAltitude = glm::min(m_cloudMaxAltitude, a);
}
//=============================================================================
float WeatherSystem::GetCloudMinAltitude()
{
	return m_cloudMinAltitude;
}
//=============================================================================
void WeatherSystem::SetCloudCoverage(float c)
{
	m_cloudCoverage = c;
	m_cloudCoverage = glm::max(m_cloudCoverage, 0.0f);
	m_cloudCoverage = glm::min(m_cloudCoverage, 1.0f);
}
//=============================================================================
void WeatherSystem::AddCloudCoverage(float c)
{
	m_cloudCoverage += c;
	m_cloudCoverage = glm::max(m_cloudCoverage, 0.0f);
	m_cloudCoverage = glm::min(m_cloudCoverage, 1.0f);
}
//=============================================================================
float WeatherSystem::GetCloudCoverage()
{
	return m_cloudCoverage;
}
//=============================================================================
void WeatherSystem::SetCloudPrecipitation(float p)
{
	m_cloudPrecipitation = p;
}
//=============================================================================
void WeatherSystem::AddCloudPrecipitation(float p)
{
	m_cloudPrecipitation += p;
	m_cloudPrecipitation = glm::max(m_cloudPrecipitation, 0.0f);
}
//=============================================================================
float WeatherSystem::GetCloudPrecipitation()
{
	return m_cloudPrecipitation;
}
//=============================================================================
void WeatherSystem::SetCloudBias(glm::vec3 b)
{
	m_cloudBias = b;
}
//=============================================================================
void WeatherSystem::SetWeatherMap(std::string path)
{
	m_weatherMapPath = path;
	m_weatherMapId = TextureManager::GetTexture(path, false);
}
//=============================================================================
std::string WeatherSystem::GetWeatherMapPath()
{
	return m_weatherMapPath;
}
//=============================================================================
gl::Texture* WeatherSystem::GetWeatherMapId()
{
	return m_weatherMapId;
}
//=============================================================================
void WeatherSystem::GenerateCloudNoise()
{
	GLubyte* perlinWorley = new GLubyte[128 * 128 * 128 * 4];
	GLubyte* worley = new GLubyte[32 * 32 * 32 * 3];
	GLubyte* curl = new GLubyte[128 * 128 * 3];

	bool fileExist = true;
	std::fstream pwNoiseFile;
	pwNoiseFile.open("Resources\\Textures\\PerlinWorley.noise", std::ios::in | std::ios::binary);
	if (!pwNoiseFile)
	{
		fileExist = false;
		pwNoiseFile.open("Resources\\Textures\\PerlinWorley.noise", std::ios::out | std::ios::binary);
	}
	for (int i = 0; i < 128; i++)
		for (int j = 0; j < 128; j++)
			for (int k = 0; k < 128; k++)
			{
				glm::vec3 uv = glm::vec3(i * 0.0078125f, j * 0.0078125f, k * 0.0078125f);
				if (fileExist)
				{
					float noise;
					pwNoiseFile.read((char*)&noise, sizeof(float));
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 0] = (GLubyte)noise;

					pwNoiseFile.read((char*)&noise, sizeof(float));
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 1] = (GLubyte)noise;

					pwNoiseFile.read((char*)&noise, sizeof(float));
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 2] = (GLubyte)noise;

					pwNoiseFile.read((char*)&noise, sizeof(float));
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 3] = (GLubyte)noise;
				}
				else
				{
					float cellCount = 8.0f;
					float worleyNoise0 = Noise::WorleyNoise(uv, cellCount * 2.0f);
					float worleyNoise1 = Noise::WorleyNoise(uv, cellCount * 4.0f);
					float worleyNoise2 = Noise::WorleyNoise(uv, cellCount * 8.0f);
					float worleyNoise3 = Noise::WorleyNoise(uv, cellCount * 14.0f);
					float worleyNoise4 = Noise::WorleyNoise(uv, cellCount * 16.0f);
					float worleyNoise5 = Noise::WorleyNoise(uv, cellCount * 32.0f);

					float worleyFbm = worleyNoise0 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
					float noise = math::Remap(Noise::PerlinFbm(uv, 8.0f, 3), 0.0f, 1.0f, worleyFbm, 1.0f) * 255;
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 0] = (GLubyte)noise;
					pwNoiseFile.write((char*)&noise, sizeof(float));

					worleyFbm = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;
					noise = worleyFbm * 255;
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 1] = (GLubyte)noise;
					pwNoiseFile.write((char*)&noise, sizeof(float));

					worleyFbm = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
					noise = worleyFbm * 255;
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 2] = (GLubyte)noise;
					pwNoiseFile.write((char*)&noise, sizeof(float));

					worleyFbm = worleyNoise2 * 0.625f + worleyNoise4 * 0.25f + worleyNoise5 * 0.125f;
					noise = worleyFbm * 255;
					perlinWorley[i * 128 * 128 * 4 + j * 128 * 4 + k * 4 + 3] = (GLubyte)noise;
					pwNoiseFile.write((char*)&noise, sizeof(float));
				}
			}
	pwNoiseFile.close();

	fileExist = true;
	std::fstream wNoiseFile;
	wNoiseFile.open("Resources\\Textures\\Worley.noise", std::ios::in | std::ios::binary);
	if (!wNoiseFile)
	{
		fileExist = false;
		wNoiseFile.open("Resources\\Textures\\Worley.noise", std::ios::out | std::ios::binary);
	}
	for (int i = 0; i < 32; i++)
		for (int j = 0; j < 32; j++)
			for (int k = 0; k < 32; k++)
			{
				glm::vec3 uv = glm::vec3(i * 0.03125f, j * 0.03125f, k * 0.03125f);
				if (fileExist)
				{
					float noise;
					wNoiseFile.read((char*)&noise, sizeof(float));
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 0] = (GLubyte)noise;

					wNoiseFile.read((char*)&noise, sizeof(float));
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 1] = (GLubyte)noise;

					wNoiseFile.read((char*)&noise, sizeof(float));
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 2] = (GLubyte)noise;
				}
				else
				{
					float cellCount = 2.0f;
					float worleyNoise0 = Noise::WorleyNoise(uv, cellCount * 1.0f);
					float worleyNoise1 = Noise::WorleyNoise(uv, cellCount * 2.0f);
					float worleyNoise2 = Noise::WorleyNoise(uv, cellCount * 4.0f);
					float worleyNoise3 = Noise::WorleyNoise(uv, cellCount * 8.0f);
					float worleyNoise4 = Noise::WorleyNoise(uv, cellCount * 16.0f);

					float worleyFbm = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;
					float noise = worleyFbm * 255;
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 0] = (GLubyte)noise;
					wNoiseFile.write((char*)&noise, sizeof(float));

					worleyFbm = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
					noise = worleyFbm * 255;
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 1] = (GLubyte)noise;
					wNoiseFile.write((char*)&noise, sizeof(float));

					worleyFbm = worleyNoise2 * 0.625f + worleyNoise3 * 0.25f + worleyNoise4 * 0.125f;
					noise = worleyFbm * 255;
					worley[i * 32 * 32 * 3 + j * 32 * 3 + k * 3 + 2] = (GLubyte)noise;
					wNoiseFile.write((char*)&noise, sizeof(float));
				}
			}
	wNoiseFile.close();

	//glGenTextures(3, m_cloudNoises);
	//glBindTexture(GL_TEXTURE_3D, m_cloudNoises[0]);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 128, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, perlinWorley);
	//glGenerateMipmap(GL_TEXTURE_3D);

	//glBindTexture(GL_TEXTURE_3D, m_cloudNoises[1]);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, 32, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, worley);
	//glGenerateMipmap(GL_TEXTURE_3D);

	//glBindTexture(GL_TEXTURE_3D, 0);

	//m_cloudNoises[2] = TextureManager::GetTexture("CurlNoise", false);
}
////=============================================================================
//GLuint WeatherSystem::GetBaseNoise()
//{
//	return m_cloudNoises[0];
//}
////=============================================================================
//GLuint WeatherSystem::GetDetailNoise()
//{
//	return m_cloudNoises[1];
//}
////=============================================================================
//GLuint WeatherSystem::GetCurlNoise()
//{
//	return m_cloudNoises[2];
//}
//=============================================================================
void WeatherSystem::SetFogDensity(float d)
{
	m_fogDensity = d;
}
//=============================================================================
void WeatherSystem::AddFogDensity(float d)
{
	m_fogDensity += d;
	m_fogDensity = glm::max(m_fogDensity, 0.0f);
}
//=============================================================================
float WeatherSystem::GetFogDensity()
{
	return m_fogDensity;
}
//=============================================================================
void WeatherSystem::SetFogColor(glm::vec3 c)
{
	m_fogColor = c;
}
//=============================================================================
glm::vec3 WeatherSystem::GetFogColor()
{
	return m_fogColor;
}
//=============================================================================
void WeatherSystem::SetFogMaxAltitude(float a)
{
	m_fogMaxAltitude = a;
}
//=============================================================================
void WeatherSystem::AddFogMaxAltitude(float a)
{
	m_fogMaxAltitude += a;
}
//=============================================================================
float WeatherSystem::GetFogMaxAltitude()
{
	return m_fogMaxAltitude;
}
//=============================================================================
void WeatherSystem::SetFogPrecipitation(float p)
{
	m_fogPrecipitation = p;
}
//=============================================================================
void WeatherSystem::AddFogPrecipitation(float p)
{
	m_fogPrecipitation += p;
	m_fogPrecipitation = glm::max(m_fogPrecipitation, 0.0f);
}
//=============================================================================
float WeatherSystem::GetFogPrecipitation()
{
	return m_fogPrecipitation;
}
//=============================================================================
void WeatherSystem::ToggleTimeLapse()
{
	m_isTimeStop = !m_isTimeStop;
}
//=============================================================================
void WeatherSystem::SetTimeStop(bool bStop)
{
	m_isTimeStop = bStop;
}
//=============================================================================
bool WeatherSystem::IsTimeStop()
{
	return m_isTimeStop;
}
//=============================================================================
glm::vec4 WeatherSystem::getSunColor()
{
	if (m_wsSunPos.y <= 0.0f)
		return glm::vec4(0.002f, 0.002f, 0.002f, 0.002f);

	glm::vec3 up(0.0f, 1.0f, 0.0f);

	glm::vec3 vP = glm::normalize(m_wsSunPos);
	float cosTheta = glm::dot(vP, up);

	glm::vec4 skyColor(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 A(-0.0193f * m_turbidity - 0.2592f, -0.0167f * m_turbidity - 0.2608f, 0.1787f * m_turbidity - 1.4630f);
	glm::vec3 B(-0.0665f * m_turbidity + 0.0008f, -0.0950f * m_turbidity + 0.0092f, -0.3554f * m_turbidity + 0.4275f);
	glm::vec3 C(-0.0004f * m_turbidity + 0.2125f, -0.0079f * m_turbidity + 0.2102f, -0.0227f * m_turbidity + 5.3251f);
	glm::vec3 D(-0.0641f * m_turbidity - 0.8989f, -0.0441f * m_turbidity - 1.6537f, 0.1206f * m_turbidity - 2.5771f);
	glm::vec3 E(-0.0033f * m_turbidity + 0.0452f, -0.0109f * m_turbidity + 0.0529f, -0.0670f * m_turbidity + 0.3703f);

	glm::vec3 F1 = (1.0f + A * glm::exp(B / cosTheta)) * (1.0f + C + E);
	glm::vec3 F2 = (1.0f + A * glm::exp(B)) * (1.0f + C * glm::exp(D * m_thetaS) + E * pow(cos(m_thetaS), 2.0f));
	glm::vec3 xyY = m_zenith * (F1 / F2);
	xyY.z = 1.0f - exp((-1.0f / m_exposure) * xyY.z);

	skyColor = math::xyY2RGB(xyY);

	return skyColor;
}
//=============================================================================
glm::vec4 WeatherSystem::getZenithColor()
{
	if (m_wsSunPos.y <= 0.0f)
		return glm::vec4(0.002f, 0.002f, 0.002f, 0.002f);

	glm::vec3 vP(0.0f, 1.0f, 0.0f);
	glm::vec3 vS = glm::normalize(m_wsSunPos);
	float vSovP = glm::dot(vS, vP);
	float gamma = acos(vSovP);

	glm::vec4 zenithColor(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 A(-0.0193f * m_turbidity - 0.2592f, -0.0167f * m_turbidity - 0.2608f, 0.1787f * m_turbidity - 1.4630f);
	glm::vec3 B(-0.0665f * m_turbidity + 0.0008f, -0.0950f * m_turbidity + 0.0092f, -0.3554f * m_turbidity + 0.4275f);
	glm::vec3 C(-0.0004f * m_turbidity + 0.2125f, -0.0079f * m_turbidity + 0.2102f, -0.0227f * m_turbidity + 5.3251f);
	glm::vec3 D(-0.0641f * m_turbidity - 0.8989f, -0.0441f * m_turbidity - 1.6537f, 0.1206f * m_turbidity - 2.5771f);
	glm::vec3 E(-0.0033f * m_turbidity + 0.0452f, -0.0109f * m_turbidity + 0.0529f, -0.0670f * m_turbidity + 0.3703f);

	glm::vec3 F1 = (1.0f + A * glm::exp(B)) * (1.0f + C * glm::exp(D * gamma) + E * pow(vSovP, 2.0f));
	glm::vec3 F2 = (1.0f + A * glm::exp(B)) * (1.0f + C * glm::exp(D * m_thetaS) + E * pow(cos(m_thetaS), 2.0f));
	glm::vec3 xyY = m_zenith * (F1 / F2);
	xyY.z = 1.0f - exp((-1.0f / m_exposure) * xyY.z);

	zenithColor = math::xyY2RGB(xyY);

	return zenithColor;
}
//=============================================================================
float* WeatherSystem::GetShaderParas()
{
	m_shaderParas[0] = m_turbidity;
	m_shaderParas[1] = m_exposure;
	m_shaderParas[2] = m_thetaS;
	m_shaderParas[3] = m_wsSunPos.x;
	m_shaderParas[4] = m_wsSunPos.y;
	m_shaderParas[5] = m_wsSunPos.z;
	m_shaderParas[6] = m_wsSunPos.w;
	m_shaderParas[7] = m_zenith.x;
	m_shaderParas[8] = m_zenith.y;
	m_shaderParas[9] = m_zenith.z;

	return m_shaderParas;
}
//=============================================================================
glm::vec3 WeatherSystem::GetCloudBias()
{
	return m_cloudBias;
}
//=============================================================================