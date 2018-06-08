#ifndef Shader_h
#define Shader_h


#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <string>
#include <fstream>

enum  class ShaderType : char {VERTEX,FRAGMENT,GEOMETRY,TESSCONTROL,TESSEVAL,COMPUTE,NONE};

class Shader
{
public:
	Shader(std::string _name, std::string _vertexPath, std::string _fragmentPath);
	Shader();
	~Shader();
	void addVertex( std::string _path );
	void addFragment( std::string _path );
	void addGeometry( std::string _path );
	void addTessellationControl( std::string _path );
	void addTessellationEvaluation( std::string _path );
	void errorCheck( GLint _shader, ShaderType _type );
	void createProgram();
	GLuint getShaderProgram() { return m_shaderProgram; }
	std::string getName() { return this -> m_name; }

private:
	void setName(std::string _name) { this -> m_name = _name; }
	std::string loadShader(std::string _filename);
	GLuint m_shaderProgram;
	GLint m_vertex;
	GLint m_fragment;
	GLint m_geometry;
	GLint m_tessellationControl;
	GLint m_tessellationEvaluation;
  const GLchar * m_fragmentSource;
  const GLchar * m_vertexSource;
  const GLchar * m_geometrySource;
  const GLchar * m_TessellationControlSource;
  const GLchar * m_TessellationEvaluationSource;
	std::string m_name;

};

#endif /* Shader_h */
