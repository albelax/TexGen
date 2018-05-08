#ifndef PBRVIEWPORT_H_
#define PBRVIEWPORT_H_

#include "Shader.h"
#include "TrackballCamera.h"
#include "Mesh.h"
#include <glm.hpp>

#include <QOpenGLWidget>
#include <QGLWidget>

class PBRViewport : public QOpenGLWidget
{
public:
  PBRViewport(QWidget *_parent);
  ~PBRViewport();

  void mouseMove(QMouseEvent * _event);
  void mouseClick( QMouseEvent * _event );

protected:
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL();
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  void paintGL();

  void resizeGL(int _w , int _h);

  void renderScene();

  void addTexture(std::string _image);

private :
  void init();
  Mesh m_tile;
  Shader m_shader;

  GLuint m_vao;
  GLuint m_vbo;
  GLuint m_nbo;
  GLuint m_tbo;

  glm::mat4 m_MV;
  glm::mat4 m_MVP;
  glm::mat4 m_tableMV;

  std::vector<GLuint> m_textures;

  GLint m_MVAddress;
  GLint m_MVPAddress;
  GLint m_NAddress;
  GLint m_colorAddress;
  GLint m_specularTextureAddress;
  GLint m_normalTextureAddress;
  GLint m_colourTextureAddress;

  GLuint m_depthTexture;

  TrackballCamera m_camera;

};

#endif
