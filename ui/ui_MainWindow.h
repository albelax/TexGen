/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *s_mainWindowGridLayout;
    QGroupBox *s_transformGB;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QPushButton *m_dotsColor;
    QSlider *m_tilesSlider;
    QLabel *label_3;
    QPushButton *m_baseColor;
    QLabel *label;
    QSlider *m_cellsSlider;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *s_drawGB;
    QGridLayout *gridLayout_2;
    QCheckBox *m_rotating;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        s_mainWindowGridLayout = new QGridLayout(centralwidget);
        s_mainWindowGridLayout->setObjectName(QStringLiteral("s_mainWindowGridLayout"));
        s_transformGB = new QGroupBox(centralwidget);
        s_transformGB->setObjectName(QStringLiteral("s_transformGB"));
        gridLayout = new QGridLayout(s_transformGB);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_2 = new QLabel(s_transformGB);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 4, 2, 1, 1);

        m_dotsColor = new QPushButton(s_transformGB);
        m_dotsColor->setObjectName(QStringLiteral("m_dotsColor"));

        gridLayout->addWidget(m_dotsColor, 6, 2, 1, 1);

        m_tilesSlider = new QSlider(s_transformGB);
        m_tilesSlider->setObjectName(QStringLiteral("m_tilesSlider"));
        m_tilesSlider->setMaximum(500);
        m_tilesSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(m_tilesSlider, 3, 2, 1, 1);

        label_3 = new QLabel(s_transformGB);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 2, 1, 1);

        m_baseColor = new QPushButton(s_transformGB);
        m_baseColor->setObjectName(QStringLiteral("m_baseColor"));

        gridLayout->addWidget(m_baseColor, 5, 2, 1, 1);

        label = new QLabel(s_transformGB);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 2, 1, 1);

        m_cellsSlider = new QSlider(s_transformGB);
        m_cellsSlider->setObjectName(QStringLiteral("m_cellsSlider"));
        m_cellsSlider->setBaseSize(QSize(0, 0));
        m_cellsSlider->setMaximum(500);
        m_cellsSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(m_cellsSlider, 1, 2, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 1, 3, 1, 1);

        label->raise();
        m_baseColor->raise();
        m_dotsColor->raise();
        label_2->raise();
        label_3->raise();
        m_cellsSlider->raise();
        m_tilesSlider->raise();

        s_mainWindowGridLayout->addWidget(s_transformGB, 0, 5, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        s_mainWindowGridLayout->addItem(horizontalSpacer_5, 0, 3, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        s_mainWindowGridLayout->addItem(horizontalSpacer_6, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        s_mainWindowGridLayout->addItem(horizontalSpacer_2, 0, 2, 1, 1);

        s_drawGB = new QGroupBox(centralwidget);
        s_drawGB->setObjectName(QStringLiteral("s_drawGB"));
        gridLayout_2 = new QGridLayout(s_drawGB);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        m_rotating = new QCheckBox(s_drawGB);
        m_rotating->setObjectName(QStringLiteral("m_rotating"));
        m_rotating->setEnabled(true);
        m_rotating->setTristate(false);

        gridLayout_2->addWidget(m_rotating, 1, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 2, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 0, 2, 1, 1);


        s_mainWindowGridLayout->addWidget(s_drawGB, 2, 5, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        s_mainWindowGridLayout->addItem(horizontalSpacer, 0, 4, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 26));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "TexSketch", Q_NULLPTR));
        s_transformGB->setTitle(QString());
        label_2->setText(QApplication::translate("MainWindow", "Color", Q_NULLPTR));
        m_dotsColor->setText(QApplication::translate("MainWindow", "Dots Color", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "Noise Tiles", Q_NULLPTR));
        m_baseColor->setText(QApplication::translate("MainWindow", "Base Color", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "Noise Cells", Q_NULLPTR));
        s_drawGB->setTitle(QString());
        m_rotating->setText(QApplication::translate("MainWindow", "Rotating", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
