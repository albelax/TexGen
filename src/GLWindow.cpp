#include "GLWindow.h"

#include <iostream>
#include <QColorDialog>
#include <QGLWidget>
#include <QImage>
#include <QColor>
#include <math.h>
#include "Image.h"

//----------------------------------------------------------------------------------------------------------------------

GLWindow::GLWindow( QWidget *_parent ) : QOpenGLWidget( _parent )
{
  m_plane = Mesh( "models/plane.obj", "plane" );
  this->resize(_parent->size());
  m_camera.setInitialMousePos(0,0);
  m_camera.setTarget(0.0f, 0.0f, -2.0f);
  m_camera.setEye(0.0f, 0.0f, 0.0f);
  m_originalImage = /*"images/sky_xneg.png"; //*/ "images/bricksSmall.jpeg";
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
  glViewport( 0, 0, devicePixelRatio(), devicePixelRatio() );
  init();
}

//----------------------------------------------------------------------------------------------------------------------

GLWindow::~GLWindow()
{

}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseMove( QMouseEvent * _event )
{
  m_camera.handleMouseMove( _event->pos().x(), _event->pos().y() );

  if ( _event->buttons() == Qt::LeftButton )
  {
    //    std::cout << "x " << _event->pos().x() << " y: " <<  _event->pos().y() << '\n';
    glm::vec2 tmp( _event->pos().x(), _event->pos().y() );
    m_stroke.push_back( tmp );
  }

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseClick(QMouseEvent * _event)
{
  if (_event->type() ==  QMouseEvent::MouseButtonRelease)
  {
    QImage strokedImage = m_image;
    strokedImage.fill( Qt::white );

    QPainter newP( &strokedImage );
    drawStroke( newP );

    //    m_editedImage.strokeRefinement(strokedImage);

    strokedImage.save( "images/testy.png", 0, -1 );
    m_stroke.clear();
  }

  m_camera.handleMouseClick( _event->pos().x(), _event->pos().y(), _event );
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
  m_renderShader = Shader( "m_toScreen", shadersAddress + "renderedVert.glsl", shadersAddress + "renderedFrag.glsl" );
  m_normalShader = Shader( "m_toScreen", shadersAddress + "NormalVert.glsl", shadersAddress + "NormalFrag.glsl" );

  glLinkProgram( m_renderShader.getShaderProgram() );
  glLinkProgram( m_normalShader.getShaderProgram() );

  glGenVertexArrays( 1, &m_vao );
  glBindVertexArray( m_vao );
  glGenBuffers( 1, &m_vbo );
  glGenBuffers( 1, &m_tbo );

  int amountVertexData = m_plane.getAmountVertexData();

  m_plane.setBufferIndex( 0 );
  // load vertices
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
  glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER, m_plane.getBufferIndex() * sizeof( float ) , m_plane.getAmountVertexData() * sizeof( float ), &m_plane.getVertexData() );
  // pass vertices to shader

  // load texture coordinates
  glBindBuffer( GL_ARRAY_BUFFER,	m_tbo );
  glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW) ;
  glBufferSubData( GL_ARRAY_BUFFER, m_plane.getBufferIndex()/3*2 * sizeof( float ), m_plane.getAmountVertexData() * sizeof(float), &m_plane.getUVsData() );

  // FRAMEBUFFER FOR THE CALCULATION OF THE NORMALS
  glGenFramebuffers( 1, &m_normalFramebuffer );
  glBindFramebuffer( GL_FRAMEBUFFER, m_normalFramebuffer );

  glGenTextures( 1, &m_normalTexture);
  glBindTexture( GL_TEXTURE_2D, m_normalTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normalTexture, 0 );
  //   end calculation of normals

  //   second framebuffer
  glGenFramebuffers( 1, &m_framebuffer );
  glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer );

  glGenTextures( 1, &m_renderedTexture);
  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderedTexture, 0);
  //   end second framebuffer

  addTexture( m_originalImage );

  m_editedImage = Image( m_image );
  m_editedImage.intensity();
  //  m_editedImage.chroma();
  //  m_editedImage.separation();
  //  m_editedImage.shading();
  //  m_editedImage.save( Image::map::ALBEDO, "images/albedo.jpg" );

  m_editedImage.save( Image::map::INTENSITY, "images/grey.jpg" );
  //  m_editedImage.save( Image::map::SHADING, "images/shading.jpg" );


  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glGenerateMipmap( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );
  int error;

  auto img = cl::ImageGL( m_editedImage.m_CLContext, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, m_textures[m_activeTexture], &error );

  if ( CL_INVALID_CONTEXT == error )
  std::cout << error << " <- error\n";

  if ( CL_SUCCESS == error )
  std::cout << error << " <- success\n";
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::paintGL()
{
  QPainter p( this );

  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glBindTexture( GL_TEXTURE_2D, m_textures[m_activeTexture]);

  //  glBindFramebuffer( GL_FRAMEBUFFER,  m_framebuffer );
  //  renderNormals();
  //  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );

  //  glBindFramebuffer( GL_FRAMEBUFFER,  3 );

  renderTexture();
  //  drawStroke( p );

  //  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::renderNormals()
{
  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


  glUseProgram( m_normalShader.getShaderProgram() );

  //  glActiveTexture( GL_TEXTURE1 );
  //  glBindTexture( GL_TEXTURE_2D, m_renderedTexture );

  //  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  //  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  //  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  //  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  //  glGenerateMipmap( GL_TEXTURE_2D );

  glBindVertexArray( m_vao );
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_normalShader.getShaderProgram(), "VertexPosition" ) );
  glVertexAttribPointer( glGetAttribLocation( m_normalShader.getShaderProgram(), "VertexPosition" ), 3, GL_FLOAT, GL_FALSE, 0, 0 );

  glBindBuffer( GL_ARRAY_BUFFER, m_tbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_normalShader.getShaderProgram(), "TexCoord" ) );
  glVertexAttribPointer( glGetAttribLocation( m_normalShader.getShaderProgram(), "TexCoord" ), 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

  glDrawArrays( GL_TRIANGLES, m_plane.getBufferIndex() / 3, ( m_plane.getAmountVertexData() / 3 ) );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::renderTexture()
{
  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //  glBindTexture( GL_TEXTURE_2D, m_textures[m_activeTexture]);

  glUseProgram( m_renderShader.getShaderProgram() );

  glBindVertexArray( m_vao );
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_renderShader.getShaderProgram(), "VertexPosition" ) );
  glVertexAttribPointer( glGetAttribLocation( m_renderShader.getShaderProgram(), "VertexPosition" ), 3, GL_FLOAT, GL_FALSE, 0, 0 );

  glBindBuffer( GL_ARRAY_BUFFER, m_tbo );
  glEnableVertexAttribArray( glGetAttribLocation( m_renderShader.getShaderProgram(), "TexCoord" ) );
  glVertexAttribPointer( glGetAttribLocation( m_renderShader.getShaderProgram(), "TexCoord" ), 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

  glDrawArrays( GL_TRIANGLES, m_plane.getBufferIndex() / 3, ( m_plane.getAmountVertexData() / 3 ) );
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

void GLWindow::drawStroke( QPainter & _p )
{
  QPoint prevPoint;
  QPoint lastPoint;
  QPen pen( Qt::red );
  pen.setWidth( 5 );

  _p.setPen(pen);
  if ( m_stroke.size() > 0 )
    for( unsigned int i = 0; i< m_stroke.size() - 1; ++i )
    {
      prevPoint.setX(m_stroke[i].x);
      prevPoint.setY(m_stroke[i].y);

      lastPoint.setX(m_stroke[i+1].x);
      lastPoint.setY(m_stroke[i+1].y);

      _p.drawLine(prevPoint, lastPoint);
    }
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showOriginalImage()
{
  static int index = -1;
  if ( index == -1 )
  {
    addTexture( m_originalImage );
    index = m_textures.size() - 1;
  }
  else
  {
    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );
  }
  m_activeTexture = index;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showAlbedoMap()
{
  static int index = -1;
  if ( index == -1 )
  {
    addTexture( "images/albedo.jpg" );
    index = m_textures.size() - 1;
  }
  else
  {
    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );
  }
  m_activeTexture = index;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showGrayscale()
{
  static int index = -1;
  if ( index == -1 )
  {
    addTexture( "images/grey.jpg" );
    index = m_textures.size() - 1;
  }
  else
  {
    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );
  }
  m_activeTexture = index;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::showShadingMap()
{
  static int index = -1;
  if ( index == -1 )
  {
    addTexture( "images/shading.jpg" );
    index = m_textures.size() - 1;
  }
  else
  {
    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_glImage.width(), m_glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_glImage.bits() );
  }
  m_activeTexture = index;
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::selectImage(int _i)
{
  std::cout << _i << "\n";
  switch ( _i )
  {
    case 0: this->showOriginalImage(); break;
    case 1: this->showAlbedoMap(); break;
    case 2: this->showShadingMap(); break;
    case 3: this->showGrayscale(); break;

    default: break;
  }
  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glGenerateMipmap( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::toNormal()
{
  // end
  int width = 512;
  int height = 512;
  const char * p = "pippo.jpg";
  saveActiveTexture( p, width, height );
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::saveActiveTexture( const char * _name, int _width, int _height)
{
  int tot = _width * _height;

  float * pix = (float * ) calloc( _width * _height * 3 , sizeof( float ) );
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pix );
  QImage im = m_image;

  for ( int i = 0; i < tot; ++i)
  {
    int x = i % _width;
    int y = i / tot;
    im.setPixel( x, y, qRgb( pix[i*3],  pix[i*3+1],  pix[i*3+2]) );
  }
  im.save( _name, 0, -1 );
  free( pix );
}

//------------------------------------------------------------------------------------------------------------------------------
