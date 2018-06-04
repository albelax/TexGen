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
    m_pbrCreated = true;
    m_activeScene = m_pbrViewport;
  }
  else
  {
    m_gl = new GLWindow(this, &m_imageProcessor);
    m_activeScene = m_gl;
  }

  m_ui->s_mainWindowGridLayout->addWidget(m_activeScene, 0, 0, 3, 5);

  createActions();
  createMenus();
  makeDiffuseMenu();
  makeSpecularMenu();
  makeNormalMenu();
  makeRoughnessMenu();
  makeMetallicMenu();
  makeAOMenu();
  makeDisplacementMenu();
  m_currentMenu = &m_diffuseMenu;

  // tabs and viewport mode
  connect(m_ui->viewport, SIGNAL(currentIndexChanged(int)), this, SLOT(swapView(int)));
  connect(m_ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(changeLayout(int)));
  connect(m_ui->tilingSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setTiling()));

  // diffuse
  connect((QSlider *)m_diffuseMenu[1], SIGNAL(sliderReleased() ), this, SLOT(updateDiffuse()));
  connect((QSlider *)m_diffuseMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateDiffuse()));
  connect((QSlider *)m_diffuseMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateDiffuse()));
  connect((QPushButton *)m_diffuseMenu[6], SIGNAL(released()), this, SLOT(resetDiffuseSettings()));

  // specular
  connect((QSlider *)m_specularMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QSlider *)m_specularMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QSlider *)m_specularMenu[7], SIGNAL(sliderReleased() ), this, SLOT(updateSpecular()));
  connect((QCheckBox *)m_specularMenu[1], SIGNAL(clicked(bool)), this, SLOT(updateSpecular()));
  connect((QCheckBox *)m_specularMenu[9], SIGNAL(clicked(bool)), this, SLOT(updateSpecular()));
  connect((QPushButton *)m_specularMenu[10], SIGNAL(released()), this, SLOT(resetSpecularSettings()));

  // normal
  connect((QSlider *)m_normalMenu[1], SIGNAL(sliderReleased() ), this, SLOT(updateNormal()));
  connect((QCheckBox *)m_normalMenu[3], SIGNAL(clicked(bool)), this, SLOT(updateNormal()));

  // roughness
  connect((QSlider *)m_roughnessMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateRoughness()));
  connect((QSlider *)m_roughnessMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateRoughness()));
  connect((QSlider *)m_roughnessMenu[7], SIGNAL(sliderReleased() ), this, SLOT(updateRoughness()));
  connect((QCheckBox *)m_roughnessMenu[1], SIGNAL(clicked(bool)), this, SLOT(updateRoughness()));
  connect((QCheckBox *)m_roughnessMenu[9], SIGNAL(clicked(bool)), this, SLOT(updateRoughness()));
  connect((QPushButton *)m_roughnessMenu[10], SIGNAL(released()), this, SLOT(resetRoughnessSettings()));

  //metallic
  connect((QCheckBox *)m_metallicMenu[1], SIGNAL(clicked(bool)), this, SLOT(pickingMetallic()));
  connect((QSlider *)m_metallicMenu[3], SIGNAL(sliderReleased() ), this, SLOT(recalculateMetallic()));
  connect((QSlider *)m_metallicMenu[5], SIGNAL(clicked(bool) ), this, SLOT(toggleMetallic()));

  //AO
  connect((QSlider *)m_AOMenu[1], SIGNAL(sliderReleased() ), this, SLOT(updateAO()));
  connect((QSlider *)m_AOMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateAO()));
  connect((QSlider *)m_AOMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateAO()));
  connect((QCheckBox *)m_AOMenu[7], SIGNAL(clicked(bool)), this, SLOT(toggleAO()));
  connect((QPushButton *)m_AOMenu[8], SIGNAL(released()), this, SLOT(resetAOSettings()));

  // displacement
  connect((QSlider *)m_displacementMenu[3], SIGNAL(sliderReleased() ), this, SLOT(updateDisplacement()));
  connect((QSlider *)m_displacementMenu[5], SIGNAL(sliderReleased() ), this, SLOT(updateDisplacement()));
  connect((QSlider *)m_displacementMenu[7], SIGNAL(sliderReleased() ), this, SLOT(updateDisplacement()));
  connect((QCheckBox *)m_displacementMenu[1], SIGNAL(clicked(bool)), this, SLOT(updateDisplacement()));
  connect((QCheckBox *)m_displacementMenu[9], SIGNAL(clicked(bool)), this, SLOT(updateDisplacement()));
  connect((QPushButton *)m_displacementMenu[10], SIGNAL(released()), this, SLOT(resetDisplacementSettings()));


  // DIFFUSE ------------
  QWidget * diffuseTab = new QWidget;
  QVBoxLayout * diffuseLayout = new QVBoxLayout;

  for ( auto &_widget : m_diffuseMenu)
  {
    _widget->setParent(diffuseTab);
    diffuseLayout->setAlignment( this, Qt::AlignTop );
    diffuseLayout->addWidget( _widget ); // probably should be added only once?
    _widget->show();
  }
  diffuseLayout->addSpacing(150);
  diffuseTab->setLayout(diffuseLayout);

  // NORMAL ------------
  QWidget * normalTab = new QWidget;
  QVBoxLayout * normalLayout = new QVBoxLayout;

  for ( auto &_widget : m_normalMenu )
  {
    _widget->setParent( normalTab );
    normalLayout->setAlignment( this, Qt::AlignTop );
    normalLayout->addWidget( _widget ); // probably should be added only once?
    _widget->show();
  }

  normalLayout->addSpacing(250);
  normalTab->setLayout( normalLayout );

  // ROUGHNESS ------------
  QWidget * roughnessTab = new QWidget;
  QVBoxLayout * roughnessLayout = new QVBoxLayout;

  for ( auto &_widget : m_roughnessMenu)
  {
    _widget->setParent(roughnessTab);
    roughnessLayout->setAlignment( this, Qt::AlignTop );
    roughnessLayout->addWidget( _widget ); // probably should be added only once?
    _widget->show();
  }
  roughnessLayout->addSpacing(100);
  roughnessTab->setLayout(roughnessLayout);

  // METALLIC ------------
  QWidget * metallicTab = new QWidget;
  QVBoxLayout * metallicLayout = new QVBoxLayout;

  for ( auto &_widget : m_metallicMenu )
  {
    _widget->setParent( metallicTab );
    metallicLayout->setAlignment( this, Qt::AlignTop );
    metallicLayout->addWidget( _widget );
    _widget->show();
  }
  metallicLayout->addSpacing(250);
  metallicTab->setLayout(metallicLayout);

  // AO ------------
  QWidget * aoTab = new QWidget;
  QVBoxLayout * aoLayout = new QVBoxLayout;

  for ( auto &_widget : m_AOMenu )
  {
    _widget->setParent( aoTab );
    aoLayout->setAlignment( this, Qt::AlignTop );
    aoLayout->addWidget( _widget );
    _widget->show();
  }
  aoLayout->addSpacing(250);
  aoTab->setLayout(aoLayout);

  // DISPLACEMENT ------------
  QWidget * displacementTab = new QWidget;
  QVBoxLayout * displacementLayout = new QVBoxLayout;

  for ( auto &_widget : m_displacementMenu)
  {
    _widget->setParent(displacementTab);
    displacementLayout->setAlignment( this, Qt::AlignTop );
    displacementLayout->addWidget( _widget ); // probably should be added only once?
    _widget->show();
  }
  displacementLayout->addSpacing(100);
  displacementTab->setLayout(displacementLayout);

  m_ui->tabWidget->setFixedWidth(440);

  m_ui->tabWidget->removeTab(0);
  m_ui->tabWidget->removeTab(0);
  m_ui->tabWidget->addTab(diffuseTab, tr("Diffuse"));
  m_ui->tabWidget->addTab(normalTab, tr("Normal"));
  m_ui->tabWidget->addTab(roughnessTab, tr("Roughness"));
  m_ui->tabWidget->addTab(metallicTab, tr("Metallic"));
  m_ui->tabWidget->addTab(aoTab, tr("AO"));
  m_ui->tabWidget->addTab(displacementTab, tr("Displacement"));

  tabsInitialized = true;

  m_ui->tilingSpinBox->setMinimum(1);


}

//------------------------------------------------------------------------

MainWindow::~MainWindow()
{
  delete m_ui;
  delete m_tabWidget;
}

//------------------------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
    case Qt::Key_Escape : QApplication::exit( EXIT_SUCCESS ); break;
    default : break;
  }
}

//------------------------------------------------------------------------

void MainWindow::mouseMoveEvent(QMouseEvent * _event)
{
  m_activeScene->mouseMove( _event );
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

  if ( static_cast<QCheckBox *>( m_metallicMenu[1] )->isChecked() )
  {
    glm::vec2 tmp( _event->pos().x() - 5, _event->pos().y() - 35 );
    auto ratio = dynamic_cast<GLWindow*>(m_activeScene)->getRatio();

    m_metallicPixel[0] = tmp.x * ratio[0];
    m_metallicPixel[1] = tmp.y * ratio[1];

    dynamic_cast<GLWindow*>(m_activeScene)->calculateMetallic( m_metallicPixel[0], m_metallicPixel[1], static_cast<QSlider *>(m_metallicMenu[3])->value() );
    static_cast<QCheckBox *>( m_metallicMenu[1] )->setChecked( false );
  }
}

//------------------------------------------------------------------------

void MainWindow::createMenus()
{
  m_fileMenu = new QMenu(tr("&File"), this);
  m_editMenu = new QMenu(tr("&Edit"), this);

  m_ui->menuFiles->addAction(openAct);
  m_ui->menuFiles->addAction(saveAct);
  m_ui->menuFiles->addAction(loadMeshAct);
}

//------------------------------------------------------------------------

void MainWindow::createActions()
{
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  loadMeshAct = new QAction(tr("&Load mesh..."), this);
  loadMeshAct->setShortcuts(QKeySequence::Open);
  connect(loadMeshAct, SIGNAL(triggered()), this, SLOT(changeMesh()));

  saveAct = new QAction(tr("&Save..."), this);
  saveAct->setShortcuts(QKeySequence::Save);
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
}

//------------------------------------------------------------------------

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::currentPath() );

  if( !fileName.isEmpty() && dynamic_cast<GLWindow *>( m_activeScene ) ) //w-discrepancy sample i of the total sample set of size
  {
    dynamic_cast<GLWindow *>( m_activeScene )->loadImage( fileName.toLatin1().data() );
  }
}

//------------------------------------------------------------------------

void MainWindow::changeMesh()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::currentPath() );

  if( !fileName.isEmpty() && dynamic_cast<PBRViewport *>( m_activeScene ) )
  {
    dynamic_cast<PBRViewport *>( m_activeScene )->changeMesh(fileName.toStdString() );
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
    dynamic_cast<GLWindow *>( m_activeScene )->save( fileName.toLatin1().data() );
  }
  updateNormal();
  updateRoughness();
  updateDiffuse();
  updateAO();
  updateDisplacement();

}

//------------------------------------------------------------------------

void MainWindow::changeLayout( int _n )
{
  if(tabsInitialized==false) return;

  if ( _n == Image::ORIGINAL && dynamic_cast<GLWindow *>( m_activeScene ) )
  {
    dynamic_cast<GLWindow *>( m_activeScene )->selectImage(_n);
  }

  switch(_n)
  {
    case 0 : updateDiffuse(); break;
    case 1 : updateNormal(); break;
    case 2 : updateRoughness(); break;
    case 3 : recalculateMetallic(); break;
    case 4 : updateAO(); break;
    case 5 : updateDisplacement(); break;
    default: break;
  }


}

//------------------------------------------------------------------------

void MainWindow::swapView( int _n )
{
  if ( _n == 0 )
  {
    delete m_activeScene;
    m_activeScene = new GLWindow(this, &m_imageProcessor);
    int n = m_ui->tabWidget->currentIndex();
    switch(n)
    {
      case 0 : updateDiffuse(); break;
      case 1 : updateNormal(); break;
      case 2 : updateRoughness(); break;
      case 3 : recalculateMetallic(); break;
      case 4 : updateAO(); break;
      case 5 : updateDisplacement(); break;
      default: break;
    }
  }

  else if ( _n == 1 )
  {
    delete m_activeScene;
    m_activeScene = new PBRViewport( this, &m_imageProcessor ); // TODO: change constructor
    setTiling();
    updateNormal();
    updateRoughness();
    updateDiffuse();
    updateAO();
    updateDisplacement();
    updateSkybox();
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
  m_specularMenu.push_back( new QPushButton( "Reset", nullptr ) );
}

//------------------------------------------------------------------------

void MainWindow::makeRoughnessMenu()
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
  sharpness->setMaximum(10);
  sharpness->setValue(5);

  //0-1
  m_roughnessMenu.push_back( new QLabel( "Invert", 0, 0 ) );
  m_roughnessMenu.push_back( new QCheckBox() );

  //2-3
  m_roughnessMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_roughnessMenu.push_back( contrast );

  //4-5
  m_roughnessMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_roughnessMenu.push_back( brightness );

  //6-7
  m_roughnessMenu.push_back( new QLabel( "Blur/Sharpen", 0, 0 ) );
  m_roughnessMenu.push_back( sharpness );

  //8-9
  m_roughnessMenu.push_back( new QLabel( "Histogram equalization", 0, 0 ) );
  m_roughnessMenu.push_back( new QCheckBox() );

  // 10
  m_roughnessMenu.push_back( new QPushButton( "Reset", nullptr ) );
}

void MainWindow::makeDisplacementMenu()
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
  sharpness->setMaximum(10);
  sharpness->setValue(5);

  //0-1
  m_displacementMenu.push_back( new QLabel( "Invert", 0, 0 ) );
  m_displacementMenu.push_back( new QCheckBox() );

  //2-3
  m_displacementMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_displacementMenu.push_back( contrast );

  //4-5
  m_displacementMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_displacementMenu.push_back( brightness );

  //6-7
  m_displacementMenu.push_back( new QLabel( "Blur/Sharpen", 0, 0 ) );
  m_displacementMenu.push_back( sharpness );

  //8-9
  m_displacementMenu.push_back( new QLabel( "Histogram equalization", 0, 0 ) );
  m_displacementMenu.push_back( new QCheckBox() );

  // 10
  m_displacementMenu.push_back( new QPushButton( "Reset", nullptr ) );
}

void MainWindow::makeAOMenu()
{
  QSlider * contrast = new QSlider( Qt::Horizontal, Q_NULLPTR );
  contrast->setMinimum(0);
  contrast->setMaximum(100);
  contrast->setValue(20);

  QSlider * brightness = new QSlider( Qt::Horizontal, Q_NULLPTR );
  brightness->setMinimum(0);
  brightness->setMaximum(100);
  brightness->setValue(50);

  QSlider * depth = new QSlider( Qt::Horizontal, Q_NULLPTR );
  depth->setMinimum(0);
  depth->setMaximum(20);
  depth->setValue(1);

  //0-1
  m_AOMenu.push_back( new QLabel( "Depth", 0, 0 ) );
  m_AOMenu.push_back( depth );

  //2-3
  m_AOMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_AOMenu.push_back( contrast );

  //4-5
  m_AOMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_AOMenu.push_back( brightness );

  //6-7
  m_AOMenu.push_back( new QLabel( "No Map", 0, 0 ) );
  m_AOMenu.push_back( new QCheckBox() );

  //8
  m_AOMenu.push_back( new QPushButton( "Reset", nullptr ) );
}

void MainWindow::makeDiffuseMenu()
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
  sharpness->setMaximum(10);
  sharpness->setValue(5);

  //0-1
  m_diffuseMenu.push_back( new QLabel( "Contrast", 0, 0 ) );
  m_diffuseMenu.push_back( contrast );

  //2-3
  m_diffuseMenu.push_back( new QLabel( "Brightness", 0, 0 ) );
  m_diffuseMenu.push_back( brightness );

  //4-5
  m_diffuseMenu.push_back( new QLabel( "Blur/Sharpen", 0, 0 ) );
  m_diffuseMenu.push_back( sharpness );

  //6
  m_diffuseMenu.push_back( new QPushButton( "Reset", nullptr ) );
}

//------------------------------------------------------------------------

void MainWindow::makeNormalMenu()
{
  QSlider * depth = new QSlider( Qt::Horizontal, Q_NULLPTR );
  depth->setMinimum(0);
  depth->setMaximum(20);
  depth->setValue(1);

  m_normalMenu.push_back( new QLabel( "Depth", 0, 0 ) );
  m_normalMenu.push_back( depth );

  m_normalMenu.push_back( new QLabel( "Invert", 0, 0 ) );
  m_normalMenu.push_back( new QCheckBox() );

}

//------------------------------------------------------------------------

void MainWindow::makeMetallicMenu()
{
  QSlider * range = new QSlider( Qt::Horizontal, Q_NULLPTR );
  range->setMinimum(0);
  range->setMaximum(255);
  range->setValue(125);

  m_metallicMenu.push_back( new QLabel( "Picking Color", 0, 0 ) );
  m_metallicMenu.push_back( new QCheckBox() );

  m_metallicMenu.push_back( new QLabel( "Range", 0, 0 ) );
  m_metallicMenu.push_back(range);

  m_metallicMenu.push_back( new QLabel( "None", 0, 0 ) );
  m_metallicMenu.push_back( new QCheckBox() );

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
  m_activeScene->calculateNormals( static_cast<QSlider *>( m_normalMenu[1])->value(), static_cast<QCheckBox *>( m_normalMenu[3])->isChecked() );
}

//------------------------------------------------------------------------

void MainWindow::updateRoughness()
{
  m_activeScene->calculateRoughness(static_cast<QSlider *>(m_roughnessMenu[5])->value(),
      static_cast<QSlider *>(m_roughnessMenu[3])->value(),
      static_cast<QCheckBox *>(m_roughnessMenu[1])->isChecked(),
      static_cast<QSlider *>(m_roughnessMenu[7])->value(),
      static_cast<QCheckBox *>(m_roughnessMenu[9])->isChecked());
}

void MainWindow::updateDisplacement()
{
  m_activeScene->calculateDisplacement(static_cast<QSlider *>(m_displacementMenu[5])->value(),
      static_cast<QSlider *>(m_displacementMenu[3])->value(),
      static_cast<QCheckBox *>(m_displacementMenu[1])->isChecked(),
      static_cast<QSlider *>(m_displacementMenu[7])->value(),
      static_cast<QCheckBox *>(m_displacementMenu[9])->isChecked());
}

void MainWindow::updateDiffuse()
{
  m_activeScene->calculateDiffuse(static_cast<QSlider *>(m_diffuseMenu[3])->value(), // BRIGHTNESS
      static_cast<QSlider *>(m_diffuseMenu[1])->value(), // CONTRAST
      static_cast<QSlider *>(m_diffuseMenu[5])->value()); // SHARPNESS
}

void MainWindow::updateAO()
{
  m_activeScene->calculateAO(static_cast<QSlider *>(m_AOMenu[1])->value(), // DEPTH
      static_cast<QSlider *>(m_AOMenu[3])->value(), // CONTRAST
      static_cast<QSlider *>(m_AOMenu[5])->value() // BRIGHTNESS
      );
}

//------------------------------------------------------------------------

void MainWindow::resetSpecularSettings()
{
  static_cast<QSlider *>(m_specularMenu[3])->setValue(20);
  static_cast<QSlider *>(m_specularMenu[5])->setValue(50);
  static_cast<QCheckBox *>(m_specularMenu[1])->setChecked(false);
  static_cast<QCheckBox *>(m_specularMenu[9])->setChecked(false);

  updateSpecular();
}

//------------------------------------------------------------------------

void MainWindow::resetRoughnessSettings()
{
  static_cast<QSlider *>(m_roughnessMenu[3])->setValue(20);
  static_cast<QSlider *>(m_roughnessMenu[5])->setValue(50);
  static_cast<QSlider *>(m_roughnessMenu[7])->setValue(5);
  static_cast<QCheckBox *>(m_roughnessMenu[1])->setChecked(false);
  static_cast<QCheckBox *>(m_roughnessMenu[9])->setChecked(false);

  updateRoughness();
}

void MainWindow::resetDisplacementSettings()
{
  static_cast<QSlider *>(m_displacementMenu[3])->setValue(20);
  static_cast<QSlider *>(m_displacementMenu[5])->setValue(50);
  static_cast<QSlider *>(m_displacementMenu[7])->setValue(5);
  static_cast<QCheckBox *>(m_displacementMenu[1])->setChecked(false);
  static_cast<QCheckBox *>(m_displacementMenu[9])->setChecked(false);

  updateDisplacement();
}

void MainWindow::resetDiffuseSettings()
{
  static_cast<QSlider *>(m_diffuseMenu[1])->setValue(20);
  static_cast<QSlider *>(m_diffuseMenu[3])->setValue(50);
  static_cast<QSlider *>(m_diffuseMenu[5])->setValue(5);

  updateDiffuse();
}

void MainWindow::resetAOSettings()
{
  static_cast<QSlider *>(m_AOMenu[1])->setValue(1);
  static_cast<QSlider *>(m_AOMenu[3])->setValue(20);
  static_cast<QSlider *>(m_AOMenu[5])->setValue(50);
  static_cast<QCheckBox *>(m_AOMenu[7])->setChecked(false);

  updateAO();
}

//------------------------------------------------------------------------

void MainWindow::pickingMetallic()
{
  dynamic_cast<GLWindow *>( m_activeScene )->showOriginalImage();
}

//------------------------------------------------------------------------

void MainWindow::recalculateMetallic()
{
  m_activeScene->calculateMetallic( m_metallicPixel[0], m_metallicPixel[1], static_cast<QSlider *>(m_metallicMenu[3])->value() );
  static_cast<QCheckBox *>( m_metallicMenu[1] )->setChecked( false );
}

//------------------------------------------------------------------------

void MainWindow::toggleMetallic()
{
  m_activeScene->toggleMetallic(static_cast<QCheckBox *>(m_metallicMenu[5])->isChecked());
  m_activeScene->calculateMetallic( m_metallicPixel[0], m_metallicPixel[1], static_cast<QSlider *>(m_metallicMenu[3])->value() );
}

void MainWindow::toggleAO()
{
  m_activeScene->toggleAO(static_cast<QCheckBox *>(m_AOMenu[7])->isChecked());
  updateAO();
}

void MainWindow::setTiling()
{
  if(dynamic_cast<PBRViewport *>(m_activeScene))
  {
    dynamic_cast<PBRViewport *>(m_activeScene)->setTiling(m_ui->tilingSpinBox->value());
  }
}

void MainWindow::updateSkybox()
{
  if(dynamic_cast<PBRViewport *>(m_activeScene))
  {
    dynamic_cast<PBRViewport *>(m_activeScene)->setSkybox(m_ui->skyboxCheckbox->isChecked());
  }
}
