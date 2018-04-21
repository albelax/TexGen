#include "include/MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
	m_ui -> setupUi(this);
	m_gl = new GLWindow(this);
	m_ui -> s_mainWindowGridLayout -> addWidget(m_gl,0,0,3,5);
	connect(m_ui->m_selectImage, SIGNAL(currentIndexChanged(int)), m_gl, SLOT(selectImage(int)));

	createActions();
	createMenus();
}

//------------------------------------------------------------------------

MainWindow::~MainWindow()
{
	delete m_ui;
}

//------------------------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
    case Qt::Key_Escape : QApplication::exit(EXIT_SUCCESS); break;
    default : break;
  }
}

//------------------------------------------------------------------------

void MainWindow::mouseMoveEvent(QMouseEvent * _event)
{
  m_gl->mouseMove(_event);
}

//------------------------------------------------------------------------

void MainWindow::mousePressEvent(QMouseEvent * _event)
{
  m_gl->mouseClick(_event);
}

//------------------------------------------------------------------------

void MainWindow::mouseReleaseEvent(QMouseEvent * _event)
{
  m_gl->mouseClick(_event);
}

//------------------------------------------------------------------------

void MainWindow::createMenus()
{
  m_fileMenu = new QMenu(tr("&File"), this);
  m_editMenu = new QMenu(tr("&Edit"), this);

  m_ui->menuFiles->addAction(openAct);
  m_ui->menuedit->addAction(calculateIntensityAct);
  m_ui->menuedit->addAction(calculateSeparationAct);
  m_ui->menuedit->addAction(calculateNormalsAct);
  m_ui->menuedit->addAction(calculateSpecularAct);
}

//------------------------------------------------------------------------

void MainWindow::createActions()
{
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  calculateIntensityAct = new QAction(tr("&Calculate Intensity"), this);
  connect(calculateIntensityAct, SIGNAL(triggered()), m_gl, SLOT(calculateIntensity()));

  calculateSeparationAct = new QAction(tr("&Calculate Separation"), this);
  connect(calculateSeparationAct, SIGNAL(triggered()), m_gl, SLOT(calculateSeparation()));

  calculateNormalsAct = new QAction(tr("&Calculate Normals"), this);
  connect(calculateNormalsAct, SIGNAL(triggered()), m_gl, SLOT(calculateNormals()));

  calculateSpecularAct = new QAction(tr("&Calculate Specular"), this);
  connect(calculateSpecularAct, SIGNAL(triggered()), m_gl, SLOT(calculateSpecular()));
}

//------------------------------------------------------------------------

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath() );
  if (!fileName.isEmpty())
  {
    m_gl->setImagePath( fileName.toLatin1().data() );
    m_gl->loadImage();
  }
}

