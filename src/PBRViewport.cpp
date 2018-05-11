#include "PBRViewport.h"

PBRViewport::PBRViewport(QWidget *_parent) : Scene( _parent )
{
  m_plane = Mesh("models/plane.obj","tile" );
  this->resize(_parent->size());
  m_camera.setInitialMousePos(0,0);
  m_camera.setTarget(0.0f, 0.0f, -2.0f);
  m_camera.setEye(0.0f, 0.0f, 0.0f);
}

PBRViewport::PBRViewport( QWidget *_parent, Image * _image ) : Scene( _parent )
{
  m_editedImage = _image;
  m_plane = Mesh("models/plane.obj","tile" );
  this->resize(_parent->size());
  m_camera.setInitialMousePos(0,0);
  m_camera.setTarget(0.0f, 0.0f, -2.0f);
  m_camera.setEye(0.0f, 0.0f, 0.0f);
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::initializeGL()
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
	m_MV = glm::translate( m_MV, glm::vec3(0.0f, 0.0f, -2.0f) );

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::resizeGL( int _w, int _h )
{

}

//----------------------------------------------------------------------------------------------------------------------

PBRViewport::~PBRViewport()
{

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::mouseMove(QMouseEvent * _event)
{
	m_camera.handleMouseMove( _event->pos().x(), _event->pos().y() );
	update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::mouseClick(QMouseEvent * _event)
{
	m_camera.handleMouseClick(_event->pos().x(), _event->pos().y(), _event);
	update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::addTexture( QImage _image )
{
	GLuint tmp;
	m_textures.push_back(tmp);
	glActiveTexture(GL_TEXTURE0 + (m_textures.size() - 1));
	glGenTextures(1, &m_textures[m_textures.size() - 1]);

//  QImage image = _image;
//	image.load(_image.c_str());
  QImage image = QGLWidget::convertToGLFormat( _image );
  if(_image.isNull())
		qWarning("IMAGE IS NULL");
	glBindTexture( GL_TEXTURE_2D, m_textures[m_textures.size()-1] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits() );
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::init()
{
	//	std::cerr << "OpenGL Version :" << glGetString(GL_VERSION) << std::endl;
	std::string shadersAddress = "shaders/";
	m_shader = Shader( "m_shader", shadersAddress + "phong_vert.glsl", shadersAddress + "phong_frag.glsl" );
	glLinkProgram( m_shader.getShaderProgram() );
	glUseProgram( m_shader.getShaderProgram() );

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );
	glGenBuffers( 1, &m_vbo );
	glGenBuffers( 1, &m_nbo );
	glGenBuffers( 1, &m_tbo );

	int amountVertexData = m_plane.getAmountVertexData();

	m_plane.setBufferIndex( 0 );

	// load vertices
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, m_plane.getAmountVertexData() * sizeof(float), &m_plane.getVertexData() );

	// pass vertices to shader
	GLint pos = glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" );
	glEnableVertexAttribArray( pos );
	glVertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	// load normals
	glBindBuffer( GL_ARRAY_BUFFER,	m_nbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, m_plane.getAmountVertexData() * sizeof(float), &m_plane.getNormalsData() );

	// pass normals to shader
	GLint n = glGetAttribLocation( m_shader.getShaderProgram(), "VertexNormal" );
	glEnableVertexAttribArray( n );
	glVertexAttribPointer( n, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	// load texture coordinates
	glBindBuffer( GL_ARRAY_BUFFER,	m_tbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float), 0, GL_STATIC_DRAW) ;
	glBufferSubData( GL_ARRAY_BUFFER, 0, m_plane.getAmountVertexData() * sizeof(float), &m_plane.getUVsData() );

	// pass texture coords to shader
	GLint t = glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" );
	glEnableVertexAttribArray( t );
	glVertexAttribPointer( t, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

	// link matrices with shader locations
	m_MVAddress = glGetUniformLocation( m_shader.getShaderProgram(), "MV" );
	m_MVPAddress = glGetUniformLocation( m_shader.getShaderProgram(), "MVP" );
	m_NAddress = glGetUniformLocation( m_shader.getShaderProgram(), "N" );
	m_colorAddress = glGetUniformLocation( m_shader.getShaderProgram(), "baseColor" );

	// textures --------------------
	m_colourTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "ColourTexture" );
	m_normalTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "NormalTexture" );
	m_specularTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "SpecularTexture" );

	// load color texture
  addTexture( m_editedImage->getDiffuse() ); // TODO
  glUniform1i( m_colourTextureAddress, 0 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glGenerateMipmap( GL_TEXTURE_2D );

//	// load normal texture
  auto tmp = m_editedImage->getDiffuse();
  addTexture( m_editedImage->calculateNormalMap( tmp, 2 ) ); // TODO
  glUniform1i( m_normalTextureAddress, 1 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glGenerateMipmap( GL_TEXTURE_2D);

//	// load specular texture
//  addTexture( m_editedImage->getSpecular() ); // TODO
//	glUniform1i( m_specularTextureAddress, 1 );

//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
//	glGenerateMipmap( GL_TEXTURE_2D);
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::paintGL()
{
	glViewport( 0, 0, width(), height() );
	glClearColor( 1, 1, 1, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	renderScene();
	update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::renderScene()
{
	glViewport( 0, 0, width(), height() );
	glClearColor( 1, 1, 1, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( m_shader.getShaderProgram() );
	m_camera.update();
	m_projection = glm::perspective( glm::radians( 60.0f ),
																		static_cast<float>( width() ) / static_cast<float>( height() ), 0.1f, 100.0f );
	m_view = glm::lookAt( glm::vec3( 0.0f, 0.0f, 5.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	m_MVP = m_projection * m_camera.viewMatrix() * m_MV;
	glm::mat3 N = glm::mat3( glm::inverse( glm::transpose( m_MV ) ) );

  glUniformMatrix4fv( m_MVPAddress, 1, GL_FALSE, glm::value_ptr( m_MVP ) );
  glUniformMatrix4fv( m_MVAddress, 1, GL_FALSE, glm::value_ptr( m_MV ) );

  glUniformMatrix3fv( m_NAddress, 1, GL_FALSE, glm::value_ptr( N ) );

//	glBindVertexArray( m_vao );
//	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
//	glEnableVertexAttribArray( glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" ) );
//	glVertexAttribPointer( glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" ), 3, GL_FLOAT, GL_FALSE, 0, 0 );

//	glBindBuffer( GL_ARRAY_BUFFER, m_tbo );
//	glEnableVertexAttribArray( glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" ) );
//	glVertexAttribPointer( glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" ), 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

  glDrawArrays( GL_TRIANGLES, m_plane.getBufferIndex() / 3, ( m_plane.getAmountVertexData() / 3 ) );
}
