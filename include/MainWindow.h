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


namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void open();
	void save();
	void changeLayout( int _n );

	void updateSpecular();
	void updateNormal();
	void resetSpecularSettings();
	void swapView( int _n );

private:
	Ui::MainWindow * m_ui;
	GLWindow * m_gl;
	PBRViewport * m_pbrViewport;
	Scene * m_activeScene;

	QMenu * m_fileMenu;
	QMenu * m_editMenu;

  QAction * openAct;
  QAction * saveAct;
  QAction * calculateIntensityAct;
  QAction * calculateSeparationAct;
  QAction * calculateNormalsAct;
  QAction * calculateSpecularAct;

  QList<QAction *> saveAsActs;
  QAction *exitAct;
  std::vector<QLayoutItem *> m_originalLayout;
  std::vector<QWidget *> m_originalMenu;
  std::vector<QWidget *> * m_currentMenu;
  std::vector<QWidget *> m_normalMenu;
  std::vector<QWidget *> m_specularMenu;

	void createActions();
	void createMenus();
	void keyPressEvent(QKeyEvent * _event);
	void mouseMoveEvent(QMouseEvent * _event);
	void mousePressEvent(QMouseEvent *_event);
	void mouseReleaseEvent(QMouseEvent *_event);
	void makeSpecularMenu();
	void makeNormalMenu();

};

#endif // MAINWINDOW_H
