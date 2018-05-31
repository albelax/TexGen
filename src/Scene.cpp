#include "Scene.h"

//-----------------------------------------------------

Scene::Scene( QWidget *_parent ) : QOpenGLWidget( _parent )
{

}

//-----------------------------------------------------

Scene::~Scene()
{

}

//-----------------------------------------------------

void Scene::toggleMetallic(bool _b)
{
  m_editedImage->toggleMetallic(_b);
}
