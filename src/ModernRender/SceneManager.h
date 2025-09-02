#pragma once

#include "GameObject.h"
#include "WeatherSystem.h"

class SceneManager final
{
public:
	bool Init();
	void Close();

	void LoadScene(const std::string& sceneName);
	void LoadDefaultScene();
	void WriteObj2XMLNode(rapidxml::xml_document<>* sceneDoc, rapidxml::xml_node<>* parent, GameObject* obj);
	void ReadObjFromXMLNode(rapidxml::xml_node<>* xmlNode, GameObject* sceneNodeParent);
	void SaveScene(const std::string& sceneName);
	void Draw(GameObject* node);
	void DrawScene();

	GameObject* GetRootNode();
	void DeleteSceneNode(GameObject* obj);
	void ClearCurScene();

	void SetCurSceneName(const std::string& name);
	std::string GetCurSceneName() const;

private:
	void deleteObject(GameObject* obj);

	std::string m_curSceneName;
	GameObject  m_sceneRoot;

	WeatherSystem m_weatherSystem;
	Camera        m_camera;
};