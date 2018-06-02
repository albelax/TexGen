#include <stdio.h>
#include <iostream>
#include "Shader.h"

Shader::Shader()
{

}

//----------------------------------------------------------------------------------------------------------------------

Shader::~Shader()
{

}

//----------------------------------------------------------------------------------------------------------------------

Shader::Shader(std::string _name, std::string _vertex, std::string _fragment)
{
	m_shaderProgram = glCreateProgram();
	m_name = _name;
	GLint tmp_shader;

	// creation of the vertex shader
	tmp_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string source = loadShader( _vertex );
	const GLchar * tmp_shaderSource = (GLchar *)source.c_str();

	glShaderSource(tmp_shader, 1, &tmp_shaderSource, NULL);
	glCompileShader(tmp_shader);
	glAttachShader(m_shaderProgram, tmp_shader);

	GLint status;
	glGetShaderiv(tmp_shader, GL_COMPILE_STATUS, &status);

	switch( status )
	{
		case(0): std::cout << "vertex shader did not compile\n"; break;
		case(1): std::cout << "vertex shader compiled\n"; break;
	}

	// creation of the fragment shader
	tmp_shader = glCreateShader( GL_FRAGMENT_SHADER );
	source = loadShader(_fragment);
	tmp_shaderSource = (GLchar *)source.c_str();

	glShaderSource(tmp_shader, 1, &tmp_shaderSource, NULL);
	glCompileShader(tmp_shader);
	glAttachShader(m_shaderProgram, tmp_shader);

	glGetShaderiv(tmp_shader, GL_COMPILE_STATUS, &status);

	switch( status )
	{
		case(0): std::cout << "vertex shader did not compile\n"; break;
		case(1): std::cout << "vertex shader compiled\n"; break;
	}

	GLchar buffer[500];
	glGetShaderInfoLog( tmp_shader, 500, nullptr, buffer );
	glDeleteShader( tmp_shader );
}

//----------------------------------------------------------------------------------------------------------------------

std::string Shader::loadShader(std::string _filename)
{
	std::ifstream shaderFile(_filename);
	std::string tmp_source;
	// iteration from beginning to end of the file
	// The default-constructed std::istreambuf_iterator is known as the end-of-stream iterator
	tmp_source = std::string(std::istreambuf_iterator<char>(shaderFile), std::istreambuf_iterator<char>());
	shaderFile.close();
	tmp_source += '\0';
	return tmp_source;
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::addVertex( std::string _path )
{
	auto tmp = loadShader( _path );
	m_vertexSource = (GLchar *)tmp.c_str();
	m_vertex = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( m_vertex, 1, &m_vertexSource, NULL );
	glCompileShader( m_vertex );
	glAttachShader( m_shaderProgram, m_vertex );
	errorCheck( m_vertex, ShaderType::VERTEX );

	GLchar buffer[500];
	glGetShaderInfoLog( m_vertex, 500, nullptr, buffer );
	glDeleteShader( m_vertex );
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::addFragment( std::string _path )
{
	auto tmp = loadShader( _path );
	m_fragmentSource = (GLchar *)tmp.c_str();
	m_fragment = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( m_fragment, 1, &m_fragmentSource, NULL );
	glCompileShader( m_fragment );
	glAttachShader( m_shaderProgram, m_fragment );
	errorCheck( m_fragment, ShaderType::FRAGMENT );
	GLchar buffer[500];
	glGetShaderInfoLog( m_fragment, 500, nullptr, buffer );
	glDeleteShader( m_fragment );
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::addGeometry( std::string _path )
{
	auto tmp = loadShader( _path );
	m_geometrySource = (GLchar *)tmp.c_str();
	m_geometry = glCreateShader( GL_GEOMETRY_SHADER );
	glShaderSource( m_geometry, 1, &m_geometrySource, NULL );
	glCompileShader( m_geometry );
	glAttachShader( m_shaderProgram, m_geometry );
	errorCheck( m_geometry, ShaderType::GEOMETRY );
	GLchar buffer[500];
	glGetShaderInfoLog( m_geometry, 500, nullptr, buffer );
	glDeleteShader( m_geometry );
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::addTessellationControl( std::string _path )
{
	auto tmp = loadShader( _path );
	m_TessellationControlSource = (GLchar *)tmp.c_str();

	m_tessellationControl = glCreateShader( GL_TESS_CONTROL_SHADER );
	glShaderSource( m_tessellationControl, 1, &m_TessellationControlSource, NULL );
	glCompileShader( m_tessellationControl );
	glAttachShader( m_shaderProgram, m_tessellationControl );
	errorCheck( m_tessellationControl, ShaderType::TESSCONTROL );
	GLchar buffer[500];
	glGetShaderInfoLog( m_tessellationControl, 500, nullptr, buffer );
	glDeleteShader( m_tessellationControl );
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::addTessellationEvaluation( std::string _path )
{
	auto tmp = loadShader( _path );
	m_TessellationEvaluationSource = (GLchar *)tmp.c_str();

	m_tessellationEvaluation = glCreateShader( GL_TESS_EVALUATION_SHADER );
	glShaderSource( m_tessellationEvaluation, 1, &m_TessellationEvaluationSource, NULL );
	glCompileShader( m_tessellationEvaluation );
	glAttachShader( m_shaderProgram, m_tessellationEvaluation );
	errorCheck( m_tessellationEvaluation, ShaderType::TESSEVAL );
	GLchar buffer[500];
	glGetShaderInfoLog( m_tessellationEvaluation, 500, nullptr, buffer );
	glDeleteShader( m_tessellationEvaluation );
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::errorCheck( GLint _shader, ShaderType _type )
{
	std::string shader;
	GLint status;
		GLchar buffer[500];
	glGetShaderiv( _shader, GL_COMPILE_STATUS, &status );
	glGetShaderInfoLog( _shader, 500, nullptr, buffer );

	switch ( _type )
	{
		case ShaderType::VERTEX: shader = "vertex"; break;
		case ShaderType::FRAGMENT: shader = "fragment"; break;
		case ShaderType::GEOMETRY: shader = "geometry"; break;
		case ShaderType::TESSCONTROL: shader = "tessellation control"; break;
		case ShaderType::TESSEVAL: shader = "tessellation evaluation"; break;
		case ShaderType::COMPUTE: shader = "compute"; break;
		default: shader = "none"; break;
	}
	switch( status )
	{
		case(0): std::cout << shader << buffer << " shader did not compile\n"; break;
		case(1): std::cout << shader << buffer << " shader compiled\n"; break;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void Shader::createProgram()
{
	m_shaderProgram = glCreateProgram();
}
