#ifndef PBRVIEWPORT_H_
#define PBRVIEWPORT_H_

#include "Shader.h"
#include "Scene.h"
#include "TrackballCamera.h"
#include "Mesh.h"
#include <glm.hpp>

#include <QOpenGLWidget>
#include <QGLWidget>
#include <QEvent>

class PBRViewport : public Scene
{
  Q_OBJECT
public:
  PBRViewport( QWidget *_parent );
  PBRViewport( QWidget *_parent, Image * _image );
  ~PBRViewport();
  void mouseMove( QMouseEvent * _event ) override;
  void mouseClick( QMouseEvent * _event ) override;
  void calculateNormals( int _depth, bool _invert ) override;
  void calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override;
  void calculateRoughness( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override;
  void calculateMetallic(int _x, int _y, float _range) override;
  void calculateDiffuse(int _brightness, int _contrast, int _sharpness ) override;
  void calculateAO(int _depth, int _contrast, int _brightness) override;

  void calculateDisplacement( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override;
  void setTiling(int _t);


  void changeMesh( std::string _filename );
  void init( bool _pbr );
  unsigned int loadCubemap(std::vector<std::string> faces);
  void setSkybox(bool _b);

protected:
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL() override;
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  void paintGL() override;
  void resizeGL( int _w , int _h ) override;
  void renderScene();
  void addTexture( QImage _image );
  void addTexture( QImage _image, GLuint * _texture, unsigned int _offset );
  void makeGrid( GLfloat _size, size_t _steps );

private :
  GLint m_specularTextureAddress;
  GLint m_normalTextureAddress;
  GLint m_colourTextureAddress;
  GLuint m_depthTexture;
  GLuint m_metallicTextureAddress;
  GLuint m_roughnessTextureAddress;
  GLuint m_aoTextureAddress;
  GLuint m_displacementTextureAddress;


  GLuint m_diffuseTexture;
  GLuint m_normalTexture;
  GLuint m_specularTexture;
  GLuint m_roughnessTexture;
  GLuint m_metallicTexture;
  GLuint m_aoTexture;
  GLuint m_displacementTexture;
  std::vector<float> m_grid;
  Shader m_gradient;
  Shader m_skybox;
  Shader m_tess;

  Mesh m_plane;
  QImage m_normal;

  GLuint m_skyboxVAO;
  GLuint m_skyboxVBO;
  GLuint m_cubemapTexture;

  // Which shader to use
  bool m_pbr;
  bool m_isSkybox;

  float skyboxSize = 10.0f;

  float skyboxVertices[108] = {
      // positions
      -skyboxSize,  skyboxSize, -skyboxSize,
      -skyboxSize, -skyboxSize, -skyboxSize,
       skyboxSize, -skyboxSize, -skyboxSize,
       skyboxSize, -skyboxSize, -skyboxSize,
       skyboxSize,  skyboxSize, -skyboxSize,
      -skyboxSize,  skyboxSize, -skyboxSize,

      -skyboxSize, -skyboxSize,  skyboxSize,
      -skyboxSize, -skyboxSize, -skyboxSize,
      -skyboxSize,  skyboxSize, -skyboxSize,
      -skyboxSize,  skyboxSize, -skyboxSize,
      -skyboxSize,  skyboxSize,  skyboxSize,
      -skyboxSize, -skyboxSize,  skyboxSize,

       skyboxSize, -skyboxSize, -skyboxSize,
       skyboxSize, -skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize, -skyboxSize,
       skyboxSize, -skyboxSize, -skyboxSize,

      -skyboxSize, -skyboxSize,  skyboxSize,
      -skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize, -skyboxSize,  skyboxSize,
      -skyboxSize, -skyboxSize,  skyboxSize,

      -skyboxSize,  skyboxSize, -skyboxSize,
       skyboxSize,  skyboxSize, -skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
       skyboxSize,  skyboxSize,  skyboxSize,
      -skyboxSize,  skyboxSize,  skyboxSize,
      -skyboxSize,  skyboxSize, -skyboxSize,

      -skyboxSize, -skyboxSize, -skyboxSize,
      -skyboxSize, -skyboxSize,  skyboxSize,
       skyboxSize, -skyboxSize, -skyboxSize,
       skyboxSize, -skyboxSize, -skyboxSize,
      -skyboxSize, -skyboxSize,  skyboxSize,
       skyboxSize, -skyboxSize,  skyboxSize
  };

};

#endif
