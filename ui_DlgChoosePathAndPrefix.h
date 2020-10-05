/********************************************************************************
** Form generated from reading UI file 'DlgChoosePathAndPrefix.ui'
**
** Created by: Qt User Interface Compiler version 5.15.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGCHOOSEPATHANDPREFIX_H
#define UI_DLGCHOOSEPATHANDPREFIX_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgChoosePathAndPrefix
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *widget;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lineEditPath;
    QPushButton *pushButtonChoosePath;
    QLabel *label_2;
    QLineEdit *lineEditPrefix;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *DlgChoosePathAndPrefix)
    {
        if (DlgChoosePathAndPrefix->objectName().isEmpty())
            DlgChoosePathAndPrefix->setObjectName(QString::fromUtf8("DlgChoosePathAndPrefix"));
        DlgChoosePathAndPrefix->resize(400, 121);
        buttonBox = new QDialogButtonBox(DlgChoosePathAndPrefix);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 70, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        widget = new QWidget(DlgChoosePathAndPrefix);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 10, 351, 51));
        gridLayout = new QGridLayout(widget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEditPath = new QLineEdit(widget);
        lineEditPath->setObjectName(QString::fromUtf8("lineEditPath"));

        gridLayout->addWidget(lineEditPath, 0, 1, 1, 1);

        pushButtonChoosePath = new QPushButton(widget);
        pushButtonChoosePath->setObjectName(QString::fromUtf8("pushButtonChoosePath"));

        gridLayout->addWidget(pushButtonChoosePath, 0, 2, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        lineEditPrefix = new QLineEdit(widget);
        lineEditPrefix->setObjectName(QString::fromUtf8("lineEditPrefix"));

        gridLayout->addWidget(lineEditPrefix, 1, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(68, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 2, 1, 1);

        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(1, 3);
        gridLayout->setColumnStretch(2, 2);

        retranslateUi(DlgChoosePathAndPrefix);
        QObject::connect(buttonBox, SIGNAL(accepted()), DlgChoosePathAndPrefix, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DlgChoosePathAndPrefix, SLOT(reject()));

        QMetaObject::connectSlotsByName(DlgChoosePathAndPrefix);
    } // setupUi

    void retranslateUi(QDialog *DlgChoosePathAndPrefix)
    {
        DlgChoosePathAndPrefix->setWindowTitle(QCoreApplication::translate("DlgChoosePathAndPrefix", "Choose Path & Prefix", nullptr));
        label->setText(QCoreApplication::translate("DlgChoosePathAndPrefix", "path", nullptr));
        pushButtonChoosePath->setText(QCoreApplication::translate("DlgChoosePathAndPrefix", "choose...", nullptr));
        label_2->setText(QCoreApplication::translate("DlgChoosePathAndPrefix", "prefix", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgChoosePathAndPrefix: public Ui_DlgChoosePathAndPrefix {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGCHOOSEPATHANDPREFIX_H
