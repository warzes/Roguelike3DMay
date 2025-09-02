#include "stdafx.h"
#include "SceneManager.h"
#include "GameMeshObject.h"
//=============================================================================
bool SceneManager::Init()
{
	if (!m_weatherSystem.Init())
		return false;

	m_curSceneName = "New Scene";
	m_sceneRoot.SetName("Root");
	return true;
}
//=============================================================================
void SceneManager::Close()
{
	m_weatherSystem.Close();
}
//=============================================================================
void SceneManager::LoadScene(const std::string& sceneName)
{
	// TODO:
}
//=============================================================================
void SceneManager::LoadDefaultScene()
{
	// TODO:
}
//=============================================================================
void SceneManager::WriteObj2XMLNode(rapidxml::xml_document<>* sceneDoc, rapidxml::xml_node<>* parent, GameObject* obj)
{
	using namespace rapidxml; // TODO: delete

	if (obj == nullptr)
		return;

	while (true)
	{
		xml_node<>* l1Node = sceneDoc->allocate_node(node_element);
		xml_attribute<>* attrib;
		switch (obj->GetType())
		{
		case GameObjectType::Model:
		{
			GameMeshObject* meshObj = (GameMeshObject*)obj;
			l1Node->name(sceneDoc->allocate_string("StaticMeshObject"));
			attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("Name"), sceneDoc->allocate_string(meshObj->GetName().c_str()));
			l1Node->append_attribute(attrib);
			//attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("Path"), sceneDoc->allocate_string(meshObj->GetMeshData()->GetPath().c_str()));
			l1Node->append_attribute(attrib);
			//attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("SubMesh"), sceneDoc->allocate_string(meshObj->GetMeshData()->GetSubMeshName().c_str()));
			l1Node->append_attribute(attrib);
			attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("Visible"), sceneDoc->allocate_string(meshObj->IsVisible() ? "1" : "0"));
			l1Node->append_attribute(attrib);
			parent->append_node(l1Node);

			//write material 
			//xml_node<>* l2Node = sceneDoc->allocate_node(node_element, sceneDoc->allocate_string("MaterialID"), sceneDoc->allocate_string(std::to_string(meshObj->GetMaterial()->GetID()).c_str()));
			//l1Node->append_node(l2Node);
		}
		break;
		case GameObjectType::Null:
		{
			l1Node->name(sceneDoc->allocate_string("Object"));
			attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("Name"), sceneDoc->allocate_string(obj->GetName().c_str()));
			l1Node->append_attribute(attrib);
			attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("Visible"), sceneDoc->allocate_string(obj->IsVisible() ? "1" : "0"));
			l1Node->append_attribute(attrib);
			parent->append_node(l1Node);
		}
		break;
		default:
			break;
		}

		//write position
		glm::vec3 info = obj->GetPosition();
		xml_node<>* l2Node = sceneDoc->allocate_node(node_element, sceneDoc->allocate_string("Position"));
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("x"), sceneDoc->allocate_string(std::to_string(info.x).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("y"), sceneDoc->allocate_string(std::to_string(info.y).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("z"), sceneDoc->allocate_string(std::to_string(info.z).c_str()));
		l2Node->append_attribute(attrib);
		l1Node->append_node(l2Node);

		//write rotation
		info = obj->GetRotation();
		l2Node = sceneDoc->allocate_node(node_element, sceneDoc->allocate_string("Rotation"));
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("x"), sceneDoc->allocate_string(std::to_string(info.x).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("y"), sceneDoc->allocate_string(std::to_string(info.y).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("z"), sceneDoc->allocate_string(std::to_string(info.z).c_str()));
		l2Node->append_attribute(attrib);
		l1Node->append_node(l2Node);

		//write scale
		info = obj->GetScale();
		l2Node = sceneDoc->allocate_node(node_element, sceneDoc->allocate_string("Scale"));
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("x"), sceneDoc->allocate_string(std::to_string(info.x).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("y"), sceneDoc->allocate_string(std::to_string(info.y).c_str()));
		l2Node->append_attribute(attrib);
		attrib = sceneDoc->allocate_attribute(sceneDoc->allocate_string("z"), sceneDoc->allocate_string(std::to_string(info.z).c_str()));
		l2Node->append_attribute(attrib);
		l1Node->append_node(l2Node);

		//write child objects
		if (obj->GetFirstChild() != nullptr)
		{
			l2Node = sceneDoc->allocate_node(node_element, sceneDoc->allocate_string("Children"));
			l1Node->append_node(l2Node);
			WriteObj2XMLNode(sceneDoc, l2Node, obj->GetFirstChild());
		}

		obj = obj->GetNext();
		if (obj == nullptr)
			break;
	}
}
//=============================================================================
void SceneManager::ReadObjFromXMLNode(rapidxml::xml_node<>* xmlNode, GameObject* sceneNodeParent)
{
	using namespace rapidxml; // TODO: delete

	if (strcmp(xmlNode->name(), "Object") == 0)
	{
		GameObject* newObj = new GameObject();
		sceneNodeParent->AddChild(newObj);
		newObj->SetVisible((int)atof(xmlNode->first_attribute("Visible")->value()));
	}
	else if (strcmp(xmlNode->name(), "StaticMeshObject") == 0)
	{
		GameMeshObject* newMeshObj = new GameMeshObject();
		sceneNodeParent->AddChild(newMeshObj);

		//MeshData* data = FbxImportManager::Instance()->ImportFbxModel(xmlNode->first_attribute("Path")->value(), xmlNode->first_attribute("SubMesh")->value());
		//newMeshObj->SetMeshData(data);
		newMeshObj->SetVisible((bool)atoi(xmlNode->first_attribute("Visible")->value()));

		unsigned int matID = (unsigned int)atoi(xmlNode->first_node("MaterialID")->value());
		//newMeshObj->SetMaterial(matID);
	}

	//read transform info
	GameObject* newObj = sceneNodeParent->GetLastChild();

	newObj->SetName(xmlNode->first_attribute("Name")->value());

	glm::vec3 transInfo;
	xml_node<>* l0Node = xmlNode->first_node("Position");
	transInfo.x = atof(l0Node->first_attribute("x")->value());
	transInfo.y = atof(l0Node->first_attribute("y")->value());
	transInfo.z = atof(l0Node->first_attribute("z")->value());
	newObj->SetPosition(transInfo);

	l0Node = xmlNode->first_node("Rotation");
	transInfo.x = atof(l0Node->first_attribute("x")->value());
	transInfo.y = atof(l0Node->first_attribute("y")->value());
	transInfo.z = atof(l0Node->first_attribute("z")->value());
	newObj->SetRotation(transInfo);

	l0Node = xmlNode->first_node("Scale");
	transInfo.x = atof(l0Node->first_attribute("x")->value());
	transInfo.y = atof(l0Node->first_attribute("y")->value());
	transInfo.z = atof(l0Node->first_attribute("z")->value());
	newObj->SetScale(transInfo);

	//read child
	xml_node<>* childRoot = xmlNode->first_node("Children");
	if (childRoot != nullptr)
	{
		xml_node<>* child = childRoot->first_node();
		ReadObjFromXMLNode(child, newObj);
	}

	//read next
	if (xmlNode->next_sibling() != nullptr)
		ReadObjFromXMLNode(xmlNode->next_sibling(), sceneNodeParent);
}
//=============================================================================
void SceneManager::SaveScene(const std::string& sceneName)
{
	// TODO:
}
//=============================================================================
void SceneManager::Draw(GameObject* node)
{
	//draw node if the type is Mesh
	if (node->GetType() == GameObjectType::Model)
	{
		GameMeshObject* meshObj = (GameMeshObject*)node;
		meshObj->Draw({});
	}

	//draw children
	GameObject* child = node->GetFirstChild();
	if (child == nullptr) return;
	while (true)
	{
		Draw(child);

		if (child->GetNext() == nullptr)
			break;

		child = child->GetNext();
	}
}
//=============================================================================
void SceneManager::DrawScene()
{
	Draw(&m_sceneRoot);
	//if (TerrainManager::Instance()->UseTerrain())
	//	TerrainManager::Instance()->GetTerrainObject()->Render(shaderProgram);

	//if (TerrainManager::Instance()->UseOcean())
	//	TerrainManager::Instance()->GetOceanObject()->Render(shaderProgram);
}
//=============================================================================
GameObject* SceneManager::GetRootNode()
{
	return &m_sceneRoot;
}
//=============================================================================
void SceneManager::DeleteSceneNode(GameObject* obj)
{
	if (obj == nullptr)return;

	GameObject* root = obj;
	if (obj == obj->GetParent()->GetFirstChild())
	{
		obj->GetParent()->SetFirstChild(obj->GetNext());
	}
	if (obj->GetPrevious() != nullptr)
	{
		obj->GetPrevious()->SetNext(obj->GetNext());
	}
	if (obj->GetNext() != nullptr)
	{
		obj->GetNext()->SetPrevious(obj->GetPrevious());
	}

	if (root->GetChildCount() > 0)
	{
		GameObject* curPtr = root->GetFirstChild();
		while (true)
		{
			if (curPtr == root)
			{
				deleteObject(curPtr);
				break;
			}

			if (curPtr->GetChildCount() > 0)
			{
				curPtr = curPtr->GetFirstChild();
				continue;
			}
			else
			{
				GameObject* temp = curPtr;
				if (curPtr->GetNext() != nullptr)
					curPtr = curPtr->GetNext();
				else
					curPtr = curPtr->GetParent();
				deleteObject(temp);
				continue;
			}
		}
	}
	else
	{
		deleteObject(root);
	}
}
//=============================================================================
void SceneManager::ClearCurScene()
{
	GameObject* child = m_sceneRoot.GetFirstChild();
	while (child != nullptr)
	{
		DeleteSceneNode(child);
		child = m_sceneRoot.GetFirstChild();
	}
}
//=============================================================================
void SceneManager::SetCurSceneName(const std::string& name)
{
	m_curSceneName = name;
}
//=============================================================================
std::string SceneManager::GetCurSceneName() const
{
	return m_curSceneName;
}
//=============================================================================
void SceneManager::deleteObject(GameObject* obj)
{
	if (obj->GetParent() != nullptr)
		obj->GetParent()->SetChildCount(obj->GetParent()->GetChildCount() - 1);

	if (obj->GetType() == GameObjectType::Model)
	{
		GameMeshObject* meshObj = (GameMeshObject*)obj;
		meshObj->Free();
		delete meshObj;
	}
	else
	{
		delete obj;
	}
}
//=============================================================================