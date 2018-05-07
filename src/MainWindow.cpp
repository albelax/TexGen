#include "include/MainWindow.h"
#include "ui_MainWindow.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
	m_ui -> setupUi(this);
	m_gl = new GLWindow(this);
	m_ui -> s_mainWindowGridLayout -> addWidget(m_gl,0,0,3,5);


	createActions();
	createMenus();
	makeSpecularMenu();
	m_currentMenu = &m_originalMenu;
	connect(m_ui->m_selectImage, SIGNAL(currentIndexChanged(int)), m_gl, SLOT(selectImage(int)));
	connect(m_ui->m_selectImage, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLayout(int)));
	connect((QSlider *)m_specularMenu[3], SIGNAL(sliderReleased() ), this, SLOT(changeContrast()));
	connect((QSlider *)m_specularMenu[5], SIGNAL(sliderReleased() ), this, SLOT(changeBrightness()));
	connect((QCheckBox *)m_specularMenu[1], SIGNAL(clicked(bool)), this, SLOT(invert()));

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
  m_ui->menuFiles->addAction(saveAct);

  m_ui->menuedit->addAction(calculateIntensityAct);
  m_ui->menuedit->addAction(calculateSeparationAct);
  m_ui->menuedit->addAction(calculateNormalsAct);
}

//------------------------------------------------------------------------

void MainWindow::createActions()
{
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(tr("&Save..."), this);
  saveAct->setShortcuts(QKeySequence::Save);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  calculateIntensityAct = new QAction(tr("&Calculate Intensity"), this);
  connect(calculateIntensityAct, SIGNAL(triggered()), m_gl, SLOT(calculateIntensity()));

  calculateSeparationAct = new QAction(tr("&Calculate Separation"), this);
  connect(calculateSeparationAct, SIGNAL(triggered()), m_gl, SLOT(calculateSeparation()));

  calculateNormalsAct = new QAction(tr("&Calculate Normals"), this);
  connect(calculateNormalsAct, SIGNAL(triggered()), m_gl, SLOT(calculateNormals()));
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

//------------------------------------------------------------------------

void MainWindow::save()
{
  QString fileFormat = "jpg";
  QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                  initialPath,tr("%1 Files (*.%2);;All Files (*)"));
  if (!fileName.isEmpty())
  {
    m_gl->save( fileName.toLatin1().data() );
    std::cout << fileName.toLatin1().data() << "\n";
  }
}
//------------------------------------------------------------------------

void MainWindow::changeLayout( int _n )
{
  QLayout * layout = m_ui->s_drawGB->layout();

  for( auto &_widget : *m_currentMenu )
  {
    _widget->hide();
  }

//    if ( _n == 3 )
//    {
//      m_currentMenu.push_back( new QPushButton( "button" ) );
//      layout->addWidget( m_currentMenu[ m_currentMenu.size() - 1] );
//    }

  if ( _n == Image::SPECULAR )
  {
    m_currentMenu = &m_specularMenu;

    for ( auto &_widget : m_specularMenu)
    {
      layout->addWidget( _widget ); // probably should be added only once?
      _widget->show();
    }
  }
}

//------------------------------------------------------------------------

void MainWindow::makeSpecularMenu()
{
  QSlider * contrast = new QSlider( Qt::Horizontal, Q_NULLPTR );
  contrast->setMinimum(0);
  contrast->setMaximum(100);
//  contrast->value();


  QSlider * brightness = new QSlider( Qt::Horizontal, Q_NULLPTR );
  brightness->setMinimum(0);
  brightness->setMaximum(100);
//  brightness->value();
  brightness->setValue( 100 );

  m_specularMenu.push_back( new QLabel( "Invert", 0, 0 ) );
  m_specularMenu.push_back( new QCheckBox() );

  m_specularMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_specularMenu.push_back( contrast );

  m_specularMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_specularMenu.push_back( brightness );

//  m_specularMenu.push_back( new QPushButton( "Calculate"));
}

//------------------------------------------------------------------------

void MainWindow::changeContrast()
{
  m_gl->calculateSpecular(static_cast<QSlider *>(m_specularMenu[5])->value(),static_cast<QSlider *>(m_specularMenu[3])->value(),static_cast<QCheckBox *>(m_specularMenu[1])->isChecked());

}

//------------------------------------------------------------------------

void MainWindow::changeBrightness()
{
  m_gl->calculateSpecular(static_cast<QSlider *>(m_specularMenu[5])->value(),static_cast<QSlider *>(m_specularMenu[3])->value(),static_cast<QCheckBox *>(m_specularMenu[1])->isChecked());
}

//------------------------------------------------------------------------

void MainWindow::invert( )
{
  m_gl->calculateSpecular(static_cast<QSlider *>(m_specularMenu[5])->value(),static_cast<QSlider *>(m_specularMenu[3])->value(),static_cast<QCheckBox *>(m_specularMenu[1])->isChecked());
}
