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
  void updateDiffuse();
  void updateAO();
  void pickingMetallic();
  void recalculateMetallic();
  void resetSpecularSettings();
  void resetRoughnessSettings();
  void resetDiffuseSettings();
  void resetAOSettings();
  void swapView( int _n );

  void toggleMetallic();
  void toggleAO();

private:
  bool m_pbrCreated = false;
  bool m_pickingColor = false;

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
  std::array<int, 2> m_metallicPixel;
  std::vector<QLayoutItem *> m_originalLayout;
  std::vector<QWidget *> * m_currentMenu;
  std::vector<QWidget *> m_diffuseMenu;
  std::vector<QWidget *> m_normalMenu;
  std::vector<QWidget *> m_specularMenu;
  std::vector<QWidget *> m_roughnessMenu;
  std::vector<QWidget *> m_metallicMenu;
  std::vector<QWidget *> m_AOMenu;

  QTabWidget * m_tabWidget;

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
  void makeDiffuseMenu();
  void makeAOMenu();

  bool tabsInitialized = false;

};

#endif // MAINWINDOW_H
