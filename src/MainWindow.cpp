#include "include/MainWindow.h"
#include "ui_MainWindow.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
  bool pbr = false;

  m_ui -> setupUi(this);

  if ( pbr )
  {
    m_pbrViewport = new PBRViewport( this );
    m_activeScene = m_pbrViewport;
  }
  else
  {
    m_gl = new GLWindow(this, & m_imageProcessor);
    m_activeScene = m_gl;
  }

  m_ui -> s_mainWindowGridLayout -> addWidget(m_activeScene, 0, 0, 3, 5);
//  m_imageProcessor = Image();/
  createActions();
  createMenus();
  makeSpecularMenu();
  makeNormalMenu();
  m_currentMenu = &m_originalMenu;

  // specular
  //	connect(m_ui->m_selectImage, SIGNAL(currentIndexChanged(int)), m_gl, SLOT(selectImage(int)));
  connect(m_ui->m_selectImage, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLayout(int)));
  connect((QSlider *)m_specularMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QSlider *)m_specularMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QSlider *)m_specularMenu[7], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QCheckBox *)m_specularMenu[1], SIGNAL(clicked(bool)), this, SLOT(updateSpecular()));
  // normal
  connect((QSlider *)m_normalMenu[1], SIGNAL(sliderReleased() ), this, SLOT(updateNormal()));
  connect((QCheckBox *)m_specularMenu[9], SIGNAL(clicked(bool)), this, SLOT(updateSpecular()));
  connect((QPushButton *)m_specularMenu[10], SIGNAL(released()), this, SLOT(resetSpecularSettings()));
  connect(m_ui->viewport, SIGNAL(currentIndexChanged(int)), this, SLOT(swapView(int)));

//  std::cout << "m_gl: " << sizeof( GLWindow ) << " pbr: " << sizeof( PBRViewport ) <<
//               " Qimage: " << sizeof( QImage ) << " Image processor " << sizeof( Image ) << "\n";
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
  m_activeScene->mouseMove(_event);
}

//------------------------------------------------------------------------

void MainWindow::mousePressEvent(QMouseEvent * _event)
{
  m_activeScene->mouseClick(_event);
}

//------------------------------------------------------------------------

void MainWindow::mouseReleaseEvent(QMouseEvent * _event)
{
  m_activeScene->mouseClick(_event);
}

//------------------------------------------------------------------------

void MainWindow::createMenus()
{
  m_fileMenu = new QMenu(tr("&File"), this);
  m_editMenu = new QMenu(tr("&Edit"), this);

  m_ui->menuFiles->addAction(openAct);
  m_ui->menuFiles->addAction(saveAct);
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
}

//------------------------------------------------------------------------

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::currentPath() );

  if( !fileName.isEmpty() && dynamic_cast<GLWindow *>( m_activeScene ) )
  {
    // this needs to change, otherwise it won't work with the pbr scene
//    dynamic_cast<GLWindow *>( m_activeScene )->setImagePath( fileName.toLatin1().data() );
    dynamic_cast<GLWindow *>( m_activeScene )->loadImage( fileName.toLatin1().data() );
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

  if ( _n == Image::SPECULAR )
  {
    m_currentMenu = &m_specularMenu;

    for ( auto &_widget : m_specularMenu)
    {
      layout->setAlignment( this, Qt::AlignTop );
      layout->addWidget( _widget ); // probably should be added only once?
      _widget->show();
    }
    updateSpecular();
  }

  if ( _n == Image::NORMAL )
  {
    m_currentMenu = &m_normalMenu;

    for ( auto &_widget : m_normalMenu )
    {
      layout->setAlignment( this, Qt::AlignTop );
      layout->addWidget( _widget ); // probably should be added only once?
      _widget->show();
    }
    updateNormal();
  }
}

//------------------------------------------------------------------------

void MainWindow::swapView( int _n )
{
  if ( _n == 0 )
  {
    delete m_activeScene;
    m_activeScene = new GLWindow(this, &m_imageProcessor);
  }

  else if ( _n == 1 )
  {
    delete m_activeScene;
    m_activeScene = new PBRViewport( this, &m_imageProcessor ); // TODO: change constructor
  }
  m_ui -> s_mainWindowGridLayout -> addWidget(m_activeScene, 0, 0, 3, 5);

}

//------------------------------------------------------------------------

void MainWindow::makeSpecularMenu()
{
  QSlider * contrast = new QSlider( Qt::Horizontal, Q_NULLPTR );
  contrast->setMinimum(0);
  contrast->setMaximum(100);
  contrast->setValue(20);

  QSlider * brightness = new QSlider( Qt::Horizontal, Q_NULLPTR );
  brightness->setMinimum(0);
  brightness->setMaximum(100);
  brightness->setValue(50);

  QSlider * sharpness = new QSlider( Qt::Horizontal, Q_NULLPTR );
  sharpness->setMinimum(0);
  sharpness->setMaximum(5);
  sharpness->setValue(1);

  m_specularMenu.push_back( new QLabel( "Invert", 0, 0 ) );
  m_specularMenu.push_back( new QCheckBox() );

  m_specularMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_specularMenu.push_back( contrast );

  m_specularMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_specularMenu.push_back( brightness );

  m_specularMenu.push_back( new QLabel( "Sharpness", 0, 0 ) );
  m_specularMenu.push_back( sharpness );

  m_specularMenu.push_back( new QLabel( "Histogram equalization", 0, 0 ) );
  m_specularMenu.push_back( new QCheckBox() );

  // 10
  m_specularMenu.push_back( new QPushButton("Reset",nullptr));
}

//------------------------------------------------------------------------

void MainWindow::makeNormalMenu()
{
  QSlider * depth = new QSlider( Qt::Horizontal, Q_NULLPTR );
  depth->setMinimum(1);
  depth->setMaximum(20);
  depth->setValue(1);

  m_normalMenu.push_back( new QLabel( "Depth", 0, 0 ) );
  m_normalMenu.push_back( depth );
}

//------------------------------------------------------------------------

void MainWindow::updateSpecular()
{
  m_activeScene->calculateSpecular(static_cast<QSlider *>(m_specularMenu[5])->value(),
      static_cast<QSlider *>(m_specularMenu[3])->value(),
      static_cast<QCheckBox *>(m_specularMenu[1])->isChecked(),
      static_cast<QSlider *>(m_specularMenu[7])->value(),
      static_cast<QCheckBox *>(m_specularMenu[9])->isChecked());
}

//------------------------------------------------------------------------

void MainWindow::updateNormal()
{
  m_activeScene->calculateNormals( static_cast<QSlider *>(m_normalMenu[1])->value() );
}

//------------------------------------------------------------------------

void MainWindow::resetSpecularSettings()
{
  static_cast<QSlider *>(m_specularMenu[3])->setValue(20);
  static_cast<QSlider *>(m_specularMenu[5])->setValue(50);
  static_cast<QCheckBox *>(m_specularMenu[1])->setTristate(false);
  static_cast<QCheckBox *>(m_specularMenu[9])->setTristate(false);

  m_gl->calculateSpecular(static_cast<QSlider *>(m_specularMenu[5])->value(),
      static_cast<QSlider *>(m_specularMenu[3])->value(),
      static_cast<QCheckBox *>(m_specularMenu[1])->isChecked(),
      static_cast<QSlider *>(m_specularMenu[7])->value(),
      static_cast<QCheckBox *>(m_specularMenu[9])->isChecked());
}

//------------------------------------------------------------------------
