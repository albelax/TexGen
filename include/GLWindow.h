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
#include "Scene.h"
#include <array>

class GLWindow : public Scene
{
  Q_OBJECT // must include this if you use Qt signals/slots
public :
  /// @brief Constructor for GLWindow
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Constructor for GLWindow
  /// @param [in] _parent the parent window to create the GL context in
  //----------------------------------------------------------------------------------------------------------------------

  GLWindow( QWidget * _parent );
  GLWindow( QWidget * _parent, Image * _image );

  void loadImage( char * _path ); // override
  void save( const char * _name );

  /// @brief dtor
  ~GLWindow();
  void mouseMove( QMouseEvent * _event ) override;
  void mouseClick( QMouseEvent * _event ) override;
  std::vector<std::vector<float>> intensity();
  std::vector< std::vector< std::vector<float> > > chroma(std::vector<std::vector<float> > &_intensity );
  std::array<float, 2> getRatio() const { return m_ratio; }
  void showOriginalImage();

public slots:
  void selectImage( int _i );
  void calculateIntensity();
  void calculateNormals( int _depth, bool _invert ) override;
  void calculateSeparation();
  void calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override;
  void calculateRoughness( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize ) override;
  void calculateMetallic( int _x, int _y, float _range );
  void calculateDiffuse(int _brightness, int _contrast, int _sharpness, bool _equalize);

protected:
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  void initializeGL() override;
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  void paintGL() override;
  void addTexture( std::string _image );
  void renderTexture();
  void renderNormals();
  void exportCSV( std::string _file );
  void drawStroke( QPainter & _p , std::array<float, 2> & _ratio );

  void showAlbedoMap();
  void showGrayscale();
  void showShadingMap();
  void showSpecular();
  void showNormalMap();

private :
  void init();
  bool clearStroke;
  bool m_textureLoaded = false;
  GLuint m_colourTextureAddress;
  GLuint m_renderedTexture;
  QImage m_image;
  QImage m_preview;
  QImage m_glImage;
  std::vector<std::vector<std::vector<float>>> m_totDiffF0;
  std::vector<glm::vec2> m_stroke;
  std::array<float, 2> m_ratio;
};

#endif
