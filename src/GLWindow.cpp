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

void GLWindow::resizeGL( int _w, int _h )
{

}

//----------------------------------------------------------------------------------------------------------------------

GLWindow::~GLWindow()
{

}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseMove(QMouseEvent * _event)
{
  m_camera.handleMouseMove( _event->pos().x(), _event->pos().y() );
  update();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::mouseClick(QMouseEvent * _event)
{
  m_camera.handleMouseClick( _event->pos().x(), _event->pos().y(), _event->type(), _event, 0 );
  update();
}

//----------------------------------------------------------------------------------------------------------------------

void GLWindow::addTexture( std::string _image )
{
  GLuint tmp;
  m_textures.push_back(tmp);
  glActiveTexture( GL_TEXTURE0 + ( m_textures.size() - 1 ) );
  glGenTextures( 1, &m_textures[ m_textures.size() - 1 ] );

  m_image.load(_image.c_str());

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
  glLinkProgram( m_renderShader.getShaderProgram() );

  glGenVertexArrays( 1, &m_vao );
  glBindVertexArray( m_vao );
  glGenBuffers( 1, &m_vbo );
  glGenBuffers( 1, &m_tbo );

  int amountVertexData = m_plane.getAmountVertexData() ;

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

  addTexture( "images/sky_xneg.png" );

  Image image( m_image );
  image.intensity();
  image.chroma();
  image.separation();
  image.save( Image::map::ALBEDO, "images/albedo.jpg" );

  glUniform1i( m_colourTextureAddress, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glGenerateMipmap( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );

}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::paintGL()
{
  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  renderTexture();

  update();
}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::renderScene()
{

}

//------------------------------------------------------------------------------------------------------------------------------

void GLWindow::renderTexture()
{
  glClearColor( 1, 1, 1, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glBindTexture( GL_TEXTURE_2D, m_textures[0]);

  glUniform1i( m_colourTextureAddress, 0 );

  glActiveTexture( GL_TEXTURE0 );

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
  for(int j =0; j< m_totDiffF0[0].size(); ++j)
  {
    for ( int k = 0; k <  m_totDiffF0[0][0].size(); ++k )
    {
      for(int i = 0; i< 20; ++i)
      {
        out << m_totDiffF0[i][j][k] << ",";
      }
      out << "\n";
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------
