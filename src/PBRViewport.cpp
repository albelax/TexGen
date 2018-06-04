#include "PBRViewport.h"

PBRViewport::PBRViewport(QWidget *_parent) : Scene( _parent )
{
  m_mesh = Mesh( "models/plane.obj","tile" );
  m_plane = Mesh( "models/plane.obj", "Rendering Target" );

  this->resize(_parent->size());
  m_camera.setMousePos(0,0);
  m_camera.setTarget(0.0f, 0.0f, -2.0f);
  m_camera.setOrigin(0.0f, 0.0f, 0.0f);
}

PBRViewport::PBRViewport( QWidget *_parent, Image * _image ) : Scene( _parent )
{
  m_editedImage = _image;
  m_mesh = Mesh("models/plane.obj","tile" );
  m_plane = Mesh( "models/plane.obj", "Rendering Target" );

  this->resize(_parent->size());
  m_camera.setMousePos(0,0);
  m_camera.setTarget(0.0f, 0.0f, -2.0f);
  m_camera.setOrigin(0.0f, 0.0f, 0.0f);
}

void PBRViewport::changeMesh(std::string _filename)
{
  makeCurrent();
  m_mesh = Mesh(_filename, "newMesh");
  init( m_pbr );
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
	init( true );
	m_MV = glm::mat4(1.0f);
	m_MV = glm::translate( m_MV, glm::vec3(0.0f, 0.0f, -2.0f) );

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::resizeGL( int _w, int _h )
{
	makeCurrent();
	m_camera.resize(_w, _h);
	addTexture( m_editedImage->getDiffuse(), &m_diffuseTexture, 0 );
	glUniform1i( m_colourTextureAddress, 0 );
}

//----------------------------------------------------------------------------------------------------------------------

PBRViewport::~PBRViewport()
{

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::mouseMove(QMouseEvent * _event)
{
	m_camera.handleMouseMove( glm::vec2(_event->pos().x(), _event->pos().y()) );
	update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::mouseClick(QMouseEvent * _event)
{
	m_camera.handleMouseClick(*_event);
	update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::addTexture( QImage _image )
{
	GLuint tmp;
	m_textures.push_back( tmp );
	glActiveTexture(GL_TEXTURE0 + (m_textures.size() - 1));
	glGenTextures(1, &m_textures[m_textures.size() - 1]);

	QImage image = QGLWidget::convertToGLFormat( _image );
	if( _image.isNull() )
		qWarning( "IMAGE IS NULLZ" );
	glBindTexture( GL_TEXTURE_2D, m_textures[m_textures.size() - 1] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits() );
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::addTexture( QImage _image, GLuint *_texture, unsigned int _offset )
{
	glActiveTexture( GL_TEXTURE0 + _offset );
	glGenTextures( 1, _texture );

	QImage image = QGLWidget::convertToGLFormat( _image );
	if( _image.isNull() )
		qWarning( "IMAGE IS NULL" );
	glBindTexture( GL_TEXTURE_2D, *_texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits() );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::init(bool _pbr)
{
	m_pbr = _pbr;
	std::string shadersAddress = "shaders/";

	if(_pbr)
	{
		//		m_shader = Shader( "m_pbrShader", shadersAddress + "pbr_vert.glsl", shadersAddress + "pbr_frag.glsl" );
		m_shader.createProgram();
		m_shader.addVertex( shadersAddress + "pbr_vert.glsl" );
		m_shader.addFragment( shadersAddress + "pbr_frag.glsl" );
	}
	else
	{
		m_shader = Shader( "m_shader", shadersAddress + "phong_vert.glsl", shadersAddress + "phong_frag.glsl" );
	}
	m_gradient = Shader( "m_gradient", shadersAddress + "gradientVert.glsl", shadersAddress + "gradientFrag.glsl" );
	m_skybox = Shader( "m_skybox", shadersAddress + "skyboxVert.glsl", shadersAddress + "skyboxFrag.glsl" );
	m_tess.createProgram();
	m_tess.addVertex(shadersAddress + "tessvert.glsl");
	m_tess.addFragment( shadersAddress + "tessfrag.glsl" );
	m_tess.addTessellationControl( shadersAddress + "tesscontrol.glsl");
	m_tess.addTessellationEvaluation( shadersAddress + "tesseval.glsl" );
	m_tess.addGeometry( shadersAddress + "tessgeom.glsl" );

	glLinkProgram( m_tess.getShaderProgram() );


	glLinkProgram( m_skybox.getShaderProgram() );
	glLinkProgram( m_gradient.getShaderProgram() );
//	glLinkProgram( m_shader.getShaderProgram() );
//	glUseProgram( m_shader.getShaderProgram() );
		glUseProgram( m_tess.getShaderProgram() );

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );
	glGenBuffers( 1, &m_vbo );
	glGenBuffers( 1, &m_nbo );
	glGenBuffers( 1, &m_tbo );

	int amountVertexData = m_plane.getAmountVertexData() + m_mesh.getAmountVertexData();
	m_plane.setBufferIndex( 0 );
	m_mesh.setBufferIndex( m_plane.getAmountVertexData() );

	// load vertices
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float) * 3.f, 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, m_plane.getBufferIndex() * sizeof(float), m_plane.getAmountVertexData() * sizeof(float), &m_plane.getVertexData() );
	glBufferSubData( GL_ARRAY_BUFFER, m_mesh.getBufferIndex() * sizeof(float), m_mesh.getAmountVertexData() * sizeof(float), &m_mesh.getVertexData() );

	// pass vertices to shader
//	GLint pos = glGetAttribLocation( m_shader.getShaderProgram(), "VertexPosition" );
	GLint pos = glGetAttribLocation( m_tess.getShaderProgram(), "VertexPosition" );

	glEnableVertexAttribArray( pos );
	glVertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	// load normals
	glBindBuffer( GL_ARRAY_BUFFER,	m_nbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float) * 3.f, 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, m_plane.getBufferIndex() * sizeof(float), m_plane.getAmountVertexData() * sizeof(float), &m_plane.getNormalsData() );
	glBufferSubData( GL_ARRAY_BUFFER, m_mesh.getBufferIndex() * sizeof(float), m_mesh.getAmountVertexData() * sizeof(float), &m_mesh.getNormalsData() );

	// pass normals to shader
//	GLint n = glGetAttribLocation( m_shader.getShaderProgram(), "VertexNormal" );
	GLint n = glGetAttribLocation( m_tess.getShaderProgram(), "VertexNormal" );

	glEnableVertexAttribArray( n );
	glVertexAttribPointer( n, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	// load texture coordinates
	glBindBuffer( GL_ARRAY_BUFFER,	m_tbo );
	glBufferData( GL_ARRAY_BUFFER, amountVertexData * sizeof(float) * 2.f, 0, GL_STATIC_DRAW) ;
	glBufferSubData( GL_ARRAY_BUFFER, m_plane.getBufferIndex() * sizeof(float), m_plane.getAmountVertexData() * sizeof(float), &m_plane.getUVsData() );
	glBufferSubData( GL_ARRAY_BUFFER, m_mesh.getBufferIndex()/3*2 * sizeof(float), m_mesh.getAmountVertexData() * sizeof(float), &m_mesh.getUVsData() );

	// pass texture coords to shader
//	GLint t = glGetAttribLocation( m_shader.getShaderProgram(), "TexCoord" );
	GLint t = glGetAttribLocation( m_tess.getShaderProgram(), "TexCoord" );

	glEnableVertexAttribArray( t );
	glVertexAttribPointer( t, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

	// link matrices with shader locations
	if(_pbr)
	{
//		m_roughnessTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "RoughnessTexture" );
//		m_metallicTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "MetallicTexture" );
		m_roughnessTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "RoughnessTexture" );
		m_metallicTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "MetallicTexture" );
	}

	else
	{
//		m_specularTextureAddress = glGetUniformLocation( m_shader.getShaderProgram(), "SpecularTexture" );
		m_specularTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "SpecularTexture" );

	}

//	m_MVAddress = glGetUniformLocation( m_shader.getShaderProgram(), "MV" );
//	m_MVPAddress = glGetUniformLocation( m_shader.getShaderProgram(), "MVP" );
//	m_NAddress = glGetUniformLocation( m_shader.getShaderProgram(), "N" );


	m_MVAddress = glGetUniformLocation( m_tess.getShaderProgram(), "MV" );
	m_MVPAddress = glGetUniformLocation( m_tess.getShaderProgram(), "MVP" );
	m_NAddress = glGetUniformLocation( m_tess.getShaderProgram(), "N" );

	//	 textures --------------------
	m_colourTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "ColourTexture" );
	m_normalTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "NormalTexture" );
	m_aoTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "AOTexture" );
	m_displacementTextureAddress = glGetUniformLocation( m_tess.getShaderProgram(), "DisplacementTexture" );

	//// load color texture
	addTexture( m_editedImage->getDiffuse(), &m_diffuseTexture, 0 );
	glUniform1i( m_colourTextureAddress, 0 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glGenerateMipmap( GL_TEXTURE_2D );

	//// load normal texture
	auto tmp = m_editedImage->getDiffuse();
	addTexture( m_editedImage->getNormal(), &m_normalTexture, 1 );

	glUniform1i( m_normalTextureAddress, 1 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glGenerateMipmap( GL_TEXTURE_2D);

	//// load specular texture

	if(!_pbr)
	{
		addTexture( m_editedImage->getSpecular(), &m_specularTexture, 2 );
		glUniform1i( m_specularTextureAddress, 2 );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D);
	}

	else
	{
		// TODO: load
		addTexture( m_editedImage->getRoughness(), &m_roughnessTexture, 3 );
		glUniform1i( m_roughnessTextureAddress, 3 );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D);

		addTexture( m_editedImage->getMetallic(), &m_metallicTexture, 4 );
		glUniform1i( m_metallicTextureAddress, 4 );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D );

		addTexture( m_editedImage->getAO(), &m_aoTexture, 5 );
		glUniform1i( m_aoTextureAddress, 5 );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D );


		addTexture( m_editedImage->getDisplacement(), &m_displacementTexture, 6 );
		glUniform1i( m_displacementTextureAddress, 6 );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	GLuint tmpTexture;
	addTexture( m_editedImage->getDiffuse(), &tmpTexture, 25 ); // void one, for some reason is needed ....
	auto camPos = m_camera.getCameraEye();
	glUniform3f( glGetUniformLocation( m_tess.getShaderProgram(), "camPos" ), camPos.x, camPos.y, camPos.z);

	if(_pbr)
	{
		glUniform1f( glGetUniformLocation( m_tess.getShaderProgram(), "ao"), 1.0f );
	}

	// SKYBOX

	glUseProgram( m_skybox.getShaderProgram() );
	glGenVertexArrays(1,&m_skyboxVAO);
	glGenBuffers(1, &m_skyboxVBO);
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"images/sky_xpos.png",
		"images/sky_xneg.png",
		"images/sky_yneg.png",
		"images/sky_ypos.png",
		"images/sky_zpos.png",
		"images/sky_zneg.png"
	};

	m_cubemapTexture = loadCubemap(faces);

	glUniform1i(glGetUniformLocation( m_skybox.getShaderProgram(), "skybox" ), 0);

	glUniform1i(glGetUniformLocation( m_tess.getShaderProgram(), "skybox" ), 6);

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::paintGL()
{
	makeCurrent();
	glViewport( 0, 0, width(), height() );
	glClearColor( 1, 1, 1, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable(GL_CULL_FACE);


	if( !m_isSkybox )
	{
		glBindVertexArray(m_vao);
		glUseProgram( m_gradient.getShaderProgram() );
		glDrawArrays( GL_TRIANGLES, m_plane.getBufferIndex()/3, m_plane.getAmountVertexData()/3 );
	}

	else
	{
		m_camera.update();
		m_MVP = m_camera.projMatrix() * m_camera.viewMatrix() * m_MV;
		glBindVertexArray( m_skyboxVAO );
		glUseProgram( m_skybox.getShaderProgram() );
		glUniformMatrix4fv( glGetUniformLocation( m_skybox.getShaderProgram(), "MVP" ), 1, GL_FALSE, glm::value_ptr( m_MVP ) );
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture );
		glDrawArrays( GL_TRIANGLES, 0, 36 );
	}

	renderScene();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::renderScene()
{
	glViewport( 0, 0, width(), height() );
	glClear( GL_DEPTH_BUFFER_BIT );
	glUseProgram( m_tess.getShaderProgram() );
	glBindVertexArray(m_vao);
//	glUseProgram( m_shader.getShaderProgram() );
	auto camPos = m_camera.getCameraEye();
//	glUniform3f( glGetUniformLocation( m_shader.getShaderProgram(), "camPos" ), camPos.x, camPos.y, camPos.z );

//	glUniform3f( glGetUniformLocation( m_shader.getShaderProgram(), "camPos" ), camPos.x, camPos.y, camPos.z );
	glUniform3f( glGetUniformLocation( m_tess.getShaderProgram(), "camPos" ), camPos.x, camPos.y, camPos.z );

	m_camera.update();

	m_MVP = m_camera.projMatrix() * m_camera.viewMatrix() * m_MV;
	glm::mat3 N = glm::mat3 (glm::inverse( glm::transpose( m_MV ) ));

	glUniformMatrix4fv( m_MVPAddress, 1, GL_FALSE, glm::value_ptr( m_MVP ) );
	glUniformMatrix4fv( m_MVAddress, 1, GL_FALSE, glm::value_ptr( m_MV ) );
	glUniformMatrix3fv( m_NAddress, 1, GL_FALSE, glm::value_ptr( N ) );

	glPatchParameteri(GL_PATCH_VERTICES, 3);       // tell OpenGL that every patch has 3 verts
	glDrawArrays( GL_PATCHES, m_mesh.getBufferIndex()/3, ( m_mesh.getAmountVertexData() / 3 ) );
//	glDrawArrays( GL_TRIANGLES, m_mesh.getBufferIndex()/3, ( m_mesh.getAmountVertexData() / 3 ) );

}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateNormals(int _depth , bool _invert )
{
  auto tmp = m_editedImage->getDiffuse();
  tmp = m_editedImage->calculateNormalMap( tmp, _depth, _invert, Image::NORMAL );
  QImage glImage = QGLWidget::convertToGLFormat( tmp );

  if( glImage.isNull() )
    qWarning( " Normal map IS NULL" );

  glBindTexture( GL_TEXTURE_2D, m_normalTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_normalTextureAddress, 1 );


  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

void PBRViewport::calculateAO(int _depth, int _contrast, int _brightness)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;

  auto tmp = m_editedImage->getDiffuse();
  m_editedImage->calculateNormalMap( tmp, _depth, false, Image::AO);
  m_editedImage->specular( tmpBrightness, tmpContrast, false, 0, false, Image::AO );

  tmp = m_editedImage->getAO();

  QImage glImage = QGLWidget::convertToGLFormat( tmp );

  if( glImage.isNull() )
    qWarning( "AO IS NULL" );

  glBindTexture( GL_TEXTURE_2D, m_aoTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_aoTextureAddress, 5 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateSpecular( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize )
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;

  m_editedImage->specular( tmpBrightness, tmpContrast, _invert, _sharpness, _equalize, Image::SPECULAR );
  auto tmp = m_editedImage->getSpecular();
  QImage glImage = QGLWidget::convertToGLFormat( tmp );
  if(glImage.isNull())
    qWarning("Spec IS NULL");

  glBindTexture( GL_TEXTURE_2D, m_metallicTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_metallicTextureAddress, 4 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateDiffuse(int _brightness, int _contrast, int _sharpness)
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;
  float tmpSharpnessBlur = _sharpness-5;

  m_editedImage->diffuse( tmpBrightness, tmpContrast, tmpSharpnessBlur);
  auto tmp = m_editedImage->getDiffuse();
  QImage glImage = QGLWidget::convertToGLFormat( tmp );

  if( glImage.isNull() )
    qWarning( "diffuse IS NULL" );

  glBindTexture( GL_TEXTURE_2D, m_diffuseTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_colourTextureAddress, 0 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateDisplacement( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize )
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;
  float tmpSharpnessBlur = _sharpness-5;

  m_editedImage->specular( tmpBrightness, tmpContrast, _invert, tmpSharpnessBlur, _equalize, Image::DISPLACEMENT );
  auto tmp = m_editedImage->getDisplacement();
  QImage glImage = QGLWidget::convertToGLFormat( tmp );

  if( glImage.isNull() )
    qWarning( "rough IS NULL" );

  glBindTexture( GL_TEXTURE_2D, m_displacementTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_displacementTextureAddress, 6 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateRoughness( int _brightness, int _contrast, bool _invert, int _sharpness, bool _equalize )
{
  float tmpBrightness = static_cast<float>( _brightness ) / 100.0f;
  float tmpContrast = static_cast<float>( _contrast ) / 100.0f;
  float tmpSharpnessBlur = _sharpness-5;

  m_editedImage->specular( tmpBrightness, tmpContrast, _invert, tmpSharpnessBlur, _equalize, Image::ROUGHNESS );
  auto tmp = m_editedImage->getRoughness();
  QImage glImage = QGLWidget::convertToGLFormat( tmp );

  if( glImage.isNull() )
    qWarning( "rough IS NULL" );

  glBindTexture( GL_TEXTURE_2D, m_roughnessTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_roughnessTextureAddress, 3 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::calculateMetallic(int _x, int _y, float _range)
{
  m_editedImage->metallic( _x, _y, _range );
  auto tmp = m_editedImage->getMetallic();
  QImage glImage = QGLWidget::convertToGLFormat( tmp );
  if(glImage.isNull())
    qWarning("metallic IS NULL");
  glBindTexture( GL_TEXTURE_2D, m_metallicTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );

  glUniform1i( m_metallicTextureAddress, 4 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  update();
}

//----------------------------------------------------------------------------------------------------------------------

unsigned int PBRViewport::loadCubemap(std::vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  for (unsigned int i = 0; i < faces.size(); i++)
  {
    QImage image = QImage(QString::fromStdString(faces[i]));
    QImage image2 = QGLWidget::convertToGLFormat( image );
    image2 = image2.mirrored(false,true);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0, GL_RGBA, image2.width(), image2.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.bits()
                 );
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  GLfloat anisotropy;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
  return textureID;
}

//----------------------------------------------------------------------------------------------------------------------

void PBRViewport::makeGrid( GLfloat _size, size_t _steps )
{
	// allocate enough space for our verts
	// as we are doing lines it will be 2 verts per line
	// and we need to add 1 to each of them for the <= loop
	// and finally muliply by 12 as we have 12 values per line pair
	int vboSize = ( _steps + 2 ) * 12;
	m_grid.reserve( vboSize );

	// claculate the step size for each grid value
	float step = _size/static_cast<float>(_steps);
	// pre-calc the offset for speed
	float s2 = _size/2.0f;
	// assign v as our value to change each vertex pair
	float v = -s2;
	// loop for our grid values

	for( size_t i = 0; i <= _steps; ++i )
	{
		// vertex 1 x,y,z
		m_grid.push_back( -s2 ); // x
		m_grid.push_back( v ); // y
		m_grid.push_back( 0.0f ); // z

		// vertex 2 x,y,z
		m_grid.push_back( s2 ); // x
		m_grid.push_back( v ); // y
		m_grid.push_back( 0.0f ); // z

		// vertex 3 x,y,z
		m_grid.push_back( v );
		m_grid.push_back( s2 );
		m_grid.push_back( 0.0f );

		// vertex 4 x,y,z
		m_grid.push_back( v );
		m_grid.push_back( -s2 );
		m_grid.push_back( 0.0f );
		// now change our step value

		v += step;
	}
}

//----------------------------------------------------------------------------------------------------------------------

