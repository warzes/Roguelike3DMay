#pragma once

class LightManager;

// TODO: или система атмосферы
class WeatherSystem final
{
public:
	bool Init(LightManager* lightManager);
	void Close();

	void Update();

	//Sun
	unsigned int GetSunLightID();
	float GetSunR();
	void SetSunR(float r);

	//Atmosphere
	void SetLatitude(float l);
	float GetLatitude();
	void SetDay(int d);
	int GetDay();
	void SetHour(float h);
	float GetHour();
	void SetTurbidity(float t);
	float GetTurbidity();
	void SetExposure(float exp);
	float GetExposure();
	Model* GetSkySphereMesh();
	void SetSkySphereMeshRotation(glm::vec3 r);
	glm::vec3 GetSkySphereMeshRotation();

	//Wind
	void SetWindDirection(glm::vec3 d);
	glm::vec3 GetWindDirection();
	void SetWindStrength(float s);
	float GetWindStrength();

	//Cloud
	void SetCloudMaxAltitude(float a);
	float GetCloudMaxAltitude();
	void SetCloudMinAltitude(float a);
	float GetCloudMinAltitude();
	void SetCloudCoverage(float c);
	void AddCloudCoverage(float c);
	float GetCloudCoverage();
	void SetCloudPrecipitation(float p);
	void AddCloudPrecipitation(float p);
	float GetCloudPrecipitation();
	void SetCloudBias(glm::vec3 b);
	glm::vec3 GetCloudBias();
	void SetWeatherMap(std::string path);
	std::string GetWeatherMapPath();
	gl::Texture* GetWeatherMapId();
	void GenerateCloudNoise();
	//GLuint GetBaseNoise();
	//GLuint GetDetailNoise();
	//GLuint GetCurlNoise();

	//Fog
	void SetFogDensity(float d);
	void AddFogDensity(float d);
	float GetFogDensity();
	void SetFogColor(glm::vec3 c);
	glm::vec3 GetFogColor();
	void SetFogMaxAltitude(float a);
	void AddFogMaxAltitude(float a);
	float GetFogMaxAltitude();
	void SetFogPrecipitation(float p);
	void AddFogPrecipitation(float p);
	float GetFogPrecipitation();

	//Time
	void SetTimeSpeed(float t);
	float GetTimeSpeed();
	void ToggleTimeLapse();
	void SetTimeStop(bool bStop);
	bool IsTimeStop();

	float* GetShaderParas();

	LightManager* GetLightManager() { return m_lightManager; }

private:
	void updateAtmosphere();
	void updateCloud();

	LightManager* m_lightManager;

	glm::vec4 getSunColor();
	glm::vec4 getZenithColor();

	bool m_isTimeStop;

	Model m_skySphereMesh;
	float m_latitude;
	int m_day;
	float m_hour;
	float m_turbidity;
	float m_exposure;
	float m_timeSpeed;
	float m_lightR;
	bool m_updateSunPos;

	float m_thetaS;
	glm::vec4 m_wsSunPos;
	glm::vec3 m_zenith; // xyY

	glm::vec4 m_windDir;
	glm::vec3 m_cloudBias;

	float m_shaderParas[10];

	unsigned int m_sunLightID;

	std::string m_weatherMapPath;
	gl::Texture* m_weatherMapId;
	float m_cloudMaxAltitude;
	float m_cloudMinAltitude;
	float m_cloudCoverage;
	float m_cloudPrecipitation;
	//GLuint m_cloudNoises[3];//PerlinWorley, Worley, Curl
	bool m_updateCloudPos;

	float m_fogDensity;
	glm::vec3 m_fogColor;
	float m_fogMaxAltitude;
	float m_fogPrecipitation;
};