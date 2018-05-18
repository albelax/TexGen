#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef linux
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

#include <QMainWindow>
#include <QtWidgets>
#include "GLWindow.h"
#include "PBRViewport.h"
#include "Scene.h"
#include "Image.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow( QWidget *parent = 0 );
  ~MainWindow();
  Image m_imageProcessor;
private slots:
  void open();
  void save();
  void changeMesh();
  void changeLayout( int _n );

  void updateSpecular();
  void updateNormal();
  void updateRoughness();
  void resetSpecularSettings();
  void swapView( int _n );

private:
  bool m_pbrCreated = false;
  Ui::MainWindow * m_ui;
  GLWindow * m_gl;
  PBRViewport * m_pbrViewport;
  Scene * m_activeScene;

  QMenu * m_fileMenu;
  QMenu * m_editMenu;

  QAction * openAct;
  QAction * saveAct;
  QAction * loadMeshAct;

  QAction * exitAct;
  QList<QAction *> saveAsActs;
  std::vector<QLayoutItem *> m_originalLayout;
  std::vector<QWidget *> * m_currentMenu;
  std::vector<QWidget *> m_originalMenu;
  std::vector<QWidget *> m_normalMenu;
  std::vector<QWidget *> m_specularMenu;
  std::vector<QWidget *> m_roughnessMenu;
  std::vector<QWidget *> m_metallicMenu;

  void createActions();
  void createMenus();
  void keyPressEvent(QKeyEvent * _event);
  void mouseMoveEvent(QMouseEvent * _event);
  void mousePressEvent(QMouseEvent *_event);
  void mouseReleaseEvent(QMouseEvent *_event);
  void makeSpecularMenu();
  void makeRoughnessMenu();
  void makeNormalMenu();
  void makeMetallicMenu();

};

#endif // MAINWINDOW_H
