#ifndef PBRVIEWPORT_H_
#define PBRVIEWPORT_H_

#include "Shader.h"
#include "Scene.h"
#include "TrackballCamera.h"
#include "Mesh.h"
#include <glm.hpp>

#include <QOpenGLWidget>
#include <QGLWidget>

class PBRViewport : public Scene
{
  Q_OBJECT
public:
  PBRViewport( QWidget *_parent );
  PBRViewport( QWidget *_parent, Image * _image );
  ~PBRViewport();
  void mouseMove( QMouseEvent * _event ) override;
  void mouseClick( QMouseEvent * _event ) override;
  void calculateNormals( int _depth ) override;
  void calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override {}

protected:
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL();
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  void paintGL();
  void resizeGL( int _w , int _h );
  void renderScene();
  void addTexture( QImage _image );
  void addTexture( QImage _image, GLuint * _texture, unsigned int _offset );

private :
  void init();
  GLint m_colorAddress;
  GLint m_specularTextureAddress;
  GLint m_normalTextureAddress;
  GLint m_colourTextureAddress;
  GLuint m_depthTexture;

  GLuint m_diffuseTexture;
  GLuint m_normalTexture;
  GLuint m_specularTexture;
  QImage m_normal;
};

#endif
