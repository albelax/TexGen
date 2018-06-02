#include "GLWindow.h"

#include <iostream>
#include <QColorDialog>
#include <QGLWidget>
#include <QImage>
#include <QColor>
#include <math.h>
#include "Image.h"


//----------------------------------------------------------------------------------------------------------------------

GLWindow::GLWindow( QWidget *_parent ) : Scene( _parent )
{
  m_mesh = Mesh( "models/plane.obj", "plane" );
  this->resize( _parent->size() );
  m_camera.setMousePos( 0, 0 );
  m_camera.setTarget( 0.0f, 0.0f, -2.0f );
  m_camera.setOrigin( 0.0f, 0.0f, 0.0f );
}

//----------------------------------------------------------------------------------------------------------------------

GLWindow::GLWindow(QWidget * _parent, Image * _image ) : Scene( _parent )
{
  m_mesh = Mesh( "models/plane.obj", "plane" );
  this->resize( _parent->size() );
  m_camera.setMousePos( 0, 0 );
  m_camera.setTarget( 0.0f, 0.0f, -2.0f );
  m_camera.setOrigin( 0.0f, 0.0f, 0.0f );

  m_editedImage = _image;
  m_preview = _image->getDiffuse();
  m_image = _image->getDiffuse();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::initializeGL()
{
#ifdef linux
  // this needs to be after the context creation, otherwise it GLEW will crash
  //std::cout <<"linux \n";
  glewExperimental = GL_TRUE;
  glewInit();
  //	GLenum error = glGetError();
#endif
  glEnable( GL_DEPTH_TEST );

  glEnable( GL_MULTISAMPLE );
  glEnable( GL_TEXTURE_2D );
  glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
  glViewport( 0, 0, devicePixelRatio(), devicePixelRatio() ); // FIX
  init();

  if( m_preview.isNull() == 0 )
  {
    m_glImage = QGLWidget::convertToGLFormat( m_preview );

    glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

    glUniform1i( m_colourTextureAddress, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    m_textureLoaded = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------

GLWindow::~GLWindow()
{

}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseMove( QMouseEvent * _event )
{
  m_camera.handleMouseMove( glm::vec2(_event->pos().x(), _event->pos().y()) );

  if ( _event->buttons() == Qt::LeftButton )
  {
    m_ratio[0] = m_image.width() / width();
    m_ratio[1] = m_image.height() / height();


    glm::vec2 tmp( _event->pos().x() - 5 , _event->pos().y() - 35 );
    m_stroke.push_back( tmp );
  }

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseClick(QMouseEvent * _event)
{
  if ( _event->type() == QMouseEvent::MouseButtonRelease )
  {
    m_ratio[0] = static_cast<float>( m_image.width() ) / static_cast<float>( width() ) ;
    m_ratio[1] = static_cast<float>( m_image.height() ) / static_cast<float>(  height() );

    QImage strokedImage = m_image;
    strokedImage.fill( Qt::white );

    QPainter newP( &strokedImage );
    drawStroke( newP, m_ratio );

    strokedImage.save( "images/testy.png", 0, -1 );
    m_stroke.clear();
    glm::vec2 tmp( _event->pos().x() - 5, _event->pos().y() - 35 );

  }
  m_camera.handleMouseClick( *_event );
  update();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::addTexture( std::string _image )
{
  GLuint tmp;
  m_textures.push_back(tmp);
  glActiveTexture( GL_TEXTURE0 + ( m_textures.size() - 1 ) );
  glGenTextures( 1, &m_textures[ m_textures.size() - 1 ] );

  m_image.load( _image.c_str() );

  m_glImage = QGLWidget::convertToGLFormat( m_image );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_textures[m_textures.size()-1] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::init()
{
  //	std::cerr << "OpenGL Version :" << glGetString(GL_VERSION) << std::endl;
  std::string shadersAddress = "shaders/";
  m_shader = Shader( "m_toScreen", shadersAddress + "renderedVert.glsl", shadersAddress + "renderedFrag.glsl" );

  glLinkProgram( m_shader.getShaderProgram() );

  glGenVertexArrays( 1, &m_vao );
  glBindVertexArray( m_vao );
  glGenBuffers( 1, &m_vbo );
  glGenBuffers( 1, &m_tbo );

  int amountVertexData = m_mesh.getAmountVertexData();

  m_mesh.setBufferIndex( 0 );
  // load vertices
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
  glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER, m_mesh.getBufferIndex() * sizeof( float ) , m_mesh.getAmountVertexData() * sizeof( float ), &m_mesh.getVertexData() );
  // pass vertices to shader

  // load texture coordinates
  glBindBuffer( GL_ARRAY_BUFFER,	m_tbo );
  glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW) ;
  glBufferSubData( GL_ARRAY_BUFFER, m_mesh.getBufferIndex()/3*2 * sizeof( float ), m_mesh.getAmountVertexData() * sizeof(float), &m_mesh.getUVsData() );

  glGenTextures( 1, &m_renderedTexture );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::paintGL()
{
  QPainter p( this );

  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if ( m_textureLoaded )
  {
    glBindTexture( GL_TEXTURE_2D, m_renderedTexture);
    renderTexture();
  }
  std::array<float, 2> ratio = { 1, 1 };
  drawStroke( p, ratio );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::renderTexture()
{
  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //  glBindTexture( GL_TEXTURE_2D, m_textures[m_activeTexture]);

  glUseProgram( m_shader.getShaderProgram() );

  glBindVertexArray( m_vao );
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" ) );
  glVertexAttribPointer( glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" ), 3, GL_FLOAT, GL_FALSE, 0, 0 );

  glBindBuffer( GL_ARRAY_BUFFER, m_tbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" ) );
  glVertexAttribPointer( glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" ), 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

  glDrawArrays( GL_TRIANGLES, m_mesh.getBufferIndex() / 3, ( m_mesh.getAmountVertexData() / 3 ) );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::exportCSV( std::string _file )
{
  std::ofstream out;
  out.open( _file );
  out.clear();
  for( unsigned int j = 0; j < m_totDiffF0[0].size(); ++j )
  {
    for ( unsigned int k = 0; k <  m_totDiffF0[0][0].size(); ++k )
    {
      for( int i = 0; i< 20; ++i )
      {
        out << m_totDiffF0[i][j][k] << ",";
      }
      out << "\n";
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::drawStroke( QPainter & _p, std::array<float, 2> & _ratio )
{
  QPoint prevPoint;
  QPoint lastPoint;
  QPen pen( Qt::red );
  pen.setWidth( 5 );

  _p.setPen(pen);
  if ( m_stroke.size() > 0 )
    for( unsigned int i = 0; i< m_stroke.size() - 1; ++i )
    {
      prevPoint.setX(m_stroke[i].x * _ratio[0]);
      prevPoint.setY(m_stroke[i].y * _ratio[1]);

      lastPoint.setX(m_stroke[i+1].x * _ratio[0]);
      lastPoint.setY(m_stroke[i+1].y * _ratio[1]);

      _p.drawLine(prevPoint, lastPoint);
    }
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showOriginalImage()
{
  glActiveTexture( GL_TEXTURE0 );
  m_preview = m_image.copy();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glActiveTexture( GL_TEXTURE0 );
  m_textureLoaded = true;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showAlbedoMap()
{

}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showGrayscale()
{
  glActiveTexture( GL_TEXTURE0 );
  m_preview = m_editedImage->getIntensity();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  m_textureLoaded = true;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showShadingMap()
{

}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showSpecular()
{
  glActiveTexture( GL_TEXTURE0 );
  m_preview = m_editedImage->getSpecular();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning( "IMAGE IS NULL" );
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glActiveTexture( GL_TEXTURE0 );
  m_textureLoaded = true;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showNormalMap()
{
  glActiveTexture( GL_TEXTURE0 );
  m_preview = m_editedImage->calculateNormalMap( m_image, 1, false, Image::NORMAL );
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glActiveTexture( GL_TEXTURE0 );
  m_textureLoaded = true;
}


//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::selectImage( int _i )
{
  switch ( _i )
  {
    case Image::ORIGINAL: this->showOriginalImage(); break;
    case Image::INTENSITY: this->showGrayscale(); break;
    case Image::NORMAL: this->showNormalMap(); break;
    case Image::ALBEDO: this->showAlbedoMap(); break;
    case Image::SHADING: this->showShadingMap(); break;
    case Image::SPECULAR: this->showSpecular(); break;

    default: break;
  }
  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glActiveTexture( GL_TEXTURE0 );
  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::save( const char * _name )
{
  m_preview.save( _name, 0, -1 );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::loadImage( char *_path )
{
  glActiveTexture( GL_TEXTURE0 );
  m_image.load( _path );
  m_preview = m_image.copy();

  m_editedImage->loadImage( m_image );

  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glActiveTexture( GL_TEXTURE0 );
  m_textureLoaded = true;
  m_ratio[0] = static_cast<float>( m_image.width() ) / static_cast<float>( width() ) ;
  m_ratio[1] = static_cast<float>( m_image.height() ) / static_cast<float>(  height() );
  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateIntensity()
{
  m_editedImage->intensity();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateNormals( int _depth , bool _invert )
{

  if(m_image.isNull()) return;

  m_preview = m_editedImage->calculateNormalMap( m_image, _depth, _invert, Image::NORMAL );

  m_glImage = QGLWidget::convertToGLFormat( m_preview );

  if( m_glImage.isNull() )
    qWarning( "IMAGE IS NULL" );
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateSeparation()
{
  m_editedImage->intensity();
  m_editedImage->chroma();
  m_editedImage->separation();
  m_editedImage->shading();
}

void GLWindow::calculateAO(int _depth, int _contrast, int _brightness)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;

  m_editedImage->calculateNormalMap(m_image, _depth, false, Image::AO);
  m_editedImage->specular( tmpBrightness, tmpContrast, false, 0, false, Image::AO );

  m_preview = m_editedImage->getAO();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;

  m_editedImage->specular( tmpBrightness, tmpContrast, _invert, _sharpness, _equalize, Image::SPECULAR );
  m_preview = m_editedImage->getSpecular();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

void GLWindow::calculateDiffuse(int _brightness, int _contrast, int _sharpness, bool _equalize)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;
  float tmpSharpnessBlur = _sharpness-5;

  if( m_editedImage->isNull() ) return;

  m_editedImage->diffuse( tmpBrightness, tmpContrast, tmpSharpnessBlur, _equalize );
  m_preview = m_editedImage->getDiffuse();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );

  if( m_glImage.isNull() )
    qWarning("IMAGE IS NULL");

  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateRoughness( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;
  float tmpSharpnessBlur = _sharpness-5;

  if( m_editedImage->isNull() ) return;

  m_editedImage->specular( tmpBrightness, tmpContrast, _invert, tmpSharpnessBlur, _equalize, Image::ROUGHNESS );
  m_preview = m_editedImage->getRoughness();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );

  if( m_glImage.isNull() )
    qWarning("IMAGE IS NULL");

  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::calculateMetallic(int _x, int _y, float _range )
{
  m_editedImage->metallic( _x, _y, _range );
  m_preview = m_editedImage->getMetallic();
  m_glImage = QGLWidget::convertToGLFormat( m_preview );
  if(m_glImage.isNull())
    qWarning("IMAGE IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//------------------------------------------------------------------------------------------------------------------------------


