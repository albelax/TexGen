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


class GLWindow : public QOpenGLWidget
{
    Q_OBJECT        // must include this if you use Qt signals/slots
public :
    /// @brief Constructor for GLWindow
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Constructor for GLWindow
    /// @param [in] _parent the parent window to create the GL context in
    //----------------------------------------------------------------------------------------------------------------------
    GLWindow(QWidget *_parent );

    /// @brief dtor
    ~GLWindow();
    void mouseMove(QMouseEvent * _event);
    void mouseClick(QMouseEvent * _event);
    std::vector<std::vector<float>> intensity();
    std::vector< std::vector< std::vector<float> > > chroma(std::vector<std::vector<float> > &_intensity );

protected:

    /// @brief  The following methods must be implimented in the sub class
    /// this is called when the window is created
    void initializeGL();

    /// @brief this is called whenever the window is re-sized
    /// @param[in] _w the width of the resized window
    /// @param[in] _h the height of the resized window
    void resizeGL(int _w , int _h);
    /// @brief this is the main gl drawing routine which is called whenever the window needs to
    // be re-drawn
    void paintGL();
    void addTexture( std::string _image );
    void renderTexture();
    void renderScene();
    void exportCSV( std::string _file );

private :
    //----------------------------------------------------------------------------------------------------------------------
    void init();
    //----------------------------------------------------------------------------------------------------------------------
    Mesh m_plane;
    //----------------------------------------------------------------------------------------------------------------------
    Shader m_renderShader;
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_vao;
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_vbo;
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_tbo;
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_colourTextureAddress;
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<GLuint> m_textures;
    //----------------------------------------------------------------------------------------------------------------------
    TrackballCamera m_camera;
    //----------------------------------------------------------------------------------------------------------------------
    QImage m_image;
    //----------------------------------------------------------------------------------------------------------------------
    QImage m_glImage;
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<std::vector<std::vector<float>>> m_totDiffF0;
    //----------------------------------------------------------------------------------------------------------------------
//    std::unique_ptr<QPainter> p {new QPainter(this)};
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<glm::vec2> m_stroke;
    //----------------------------------------------------------------------------------------------------------------------
    bool clearStroke;

};

#endif
