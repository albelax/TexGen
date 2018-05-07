#ifndef NGLSCENE_H_
#define NGLSCENE_H_


#include "Shader.h"
#include "TrackballCamera.h"
#include "Mesh.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <glm.hpp>
#include <QOpenGLWidget>
#include <QResizeEvent>
#include <QEvent>
#include <memory>
#include <QImage>
#include <QPainter>
#include <string>
#include "Image.h"


class GLWindow : public QOpenGLWidget
{
  Q_OBJECT // must include this if you use Qt signals/slots
public :
  /// @brief Constructor for GLWindow
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Constructor for GLWindow
  /// @param [in] _parent the parent window to create the GL context in
  //----------------------------------------------------------------------------------------------------------------------
  GLWindow(QWidget *_parent );
  void loadImage();
  void setImagePath( char * _path );
  void save(const char * _name);

  /// @brief dtor
  ~GLWindow();
  void mouseMove(QMouseEvent * _event);
  void mouseClick(QMouseEvent * _event);
  std::vector<std::vector<float>> intensity();
  std::vector< std::vector< std::vector<float> > > chroma(std::vector<std::vector<float> > &_intensity );

public slots:
  void selectImage( int _i );
  void calculateIntensity();
  void calculateNormals();
  void calculateSeparation();
  void calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness );

protected:
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL();
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  void paintGL();
  void addTexture( std::string _image );
  void renderTexture();
  void renderNormals();
  void exportCSV( std::string _file );
  void drawStroke( QPainter & _p );

  void showOriginalImage();
  void showAlbedoMap();
  void showGrayscale();
  void showShadingMap();
  void showSpecular();
  void showNormalMap();


private :
  void init();
  Mesh m_plane;
  Shader m_renderShader;
  Shader m_normalShader;
  GLuint m_vao;
  GLuint m_vbo;
  GLuint m_tbo;
  GLuint m_colourTextureAddress;
  GLuint m_normalFramebuffer;
  GLuint m_framebuffer;
  GLuint m_normalTexture;
  GLuint m_renderedTexture;
  std::vector<GLuint> m_textures;
  TrackballCamera m_camera;
  QImage m_image;
  QImage m_preview;
  QImage m_glImage;
  std::vector<std::vector<std::vector<float>>> m_totDiffF0;
  std::vector<glm::vec2> m_stroke;
  bool clearStroke;
  Image m_editedImage;
  char * m_originalImage;
  unsigned int m_activeTexture = 0;
  bool m_textureLoaded = false;
};

#endif
