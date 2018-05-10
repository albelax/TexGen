#ifndef SCENE_H
#define SCENE_H

#include "Shader.h"
#include "Mesh.h"
#include "TrackballCamera.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <ext.hpp>
#include <glm.hpp>
#include <QOpenGLWidget>

class Scene : public QOpenGLWidget
{
  Q_OBJECT
public:
//  Scene();
  Scene(QWidget *_parent );

  ~Scene();
  virtual void mouseMove(QMouseEvent * _event ) = 0;
  virtual void mouseClick(QMouseEvent * _event ) = 0;

protected:
  Mesh m_plane;
  Shader m_shader;
  GLuint m_vao;
  GLuint m_nbo;
  GLuint m_vbo;
  GLuint m_tbo;
  glm::mat4 m_view;
  glm::mat4 m_projection;
  glm::mat4 m_MV;
  glm::mat4 m_MVP;

  GLint m_MVAddress;
  GLint m_MVPAddress;
  GLint m_NAddress;
  std::vector<GLuint> m_textures;

  TrackballCamera m_camera;
};

#endif // SCENE_H
