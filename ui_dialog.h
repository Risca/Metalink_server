/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created: Wed Jan 6 20:00:12 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *nickLabel;
    QLineEdit *nickLineEdit;
    QLabel *hostnameLabel;
    QLineEdit *hostnameLineEdit;
    QLabel *portLabel;
    QLineEdit *portLineEdit;
    QPlainTextEdit *plainTextEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *sendButton;
    QPushButton *connectButton;
    QPushButton *quitButton;
    QListWidget *listWidget;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(433, 352);
        horizontalLayout_2 = new QHBoxLayout(Dialog);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        nickLabel = new QLabel(Dialog);
        nickLabel->setObjectName(QString::fromUtf8("nickLabel"));

        formLayout->setWidget(0, QFormLayout::LabelRole, nickLabel);

        nickLineEdit = new QLineEdit(Dialog);
        nickLineEdit->setObjectName(QString::fromUtf8("nickLineEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, nickLineEdit);

        hostnameLabel = new QLabel(Dialog);
        hostnameLabel->setObjectName(QString::fromUtf8("hostnameLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, hostnameLabel);

        hostnameLineEdit = new QLineEdit(Dialog);
        hostnameLineEdit->setObjectName(QString::fromUtf8("hostnameLineEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, hostnameLineEdit);

        portLabel = new QLabel(Dialog);
        portLabel->setObjectName(QString::fromUtf8("portLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, portLabel);

        portLineEdit = new QLineEdit(Dialog);
        portLineEdit->setObjectName(QString::fromUtf8("portLineEdit"));

        formLayout->setWidget(2, QFormLayout::FieldRole, portLineEdit);


        verticalLayout->addLayout(formLayout);

        plainTextEdit = new QPlainTextEdit(Dialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));

        verticalLayout->addWidget(plainTextEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        sendButton = new QPushButton(Dialog);
        sendButton->setObjectName(QString::fromUtf8("sendButton"));
        sendButton->setEnabled(false);

        horizontalLayout->addWidget(sendButton);

        connectButton = new QPushButton(Dialog);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));

        horizontalLayout->addWidget(connectButton);

        quitButton = new QPushButton(Dialog);
        quitButton->setObjectName(QString::fromUtf8("quitButton"));

        horizontalLayout->addWidget(quitButton);


        verticalLayout->addLayout(horizontalLayout);


        horizontalLayout_2->addLayout(verticalLayout);

        listWidget = new QListWidget(Dialog);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        horizontalLayout_2->addWidget(listWidget);


        retranslateUi(Dialog);
        QObject::connect(quitButton, SIGNAL(clicked()), Dialog, SLOT(close()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
        nickLabel->setText(QApplication::translate("Dialog", "Nick", 0, QApplication::UnicodeUTF8));
        nickLineEdit->setText(QApplication::translate("Dialog", "Risca", 0, QApplication::UnicodeUTF8));
        hostnameLabel->setText(QApplication::translate("Dialog", "Hostname", 0, QApplication::UnicodeUTF8));
        hostnameLineEdit->setText(QApplication::translate("Dialog", "localhost", 0, QApplication::UnicodeUTF8));
        portLabel->setText(QApplication::translate("Dialog", "Port", 0, QApplication::UnicodeUTF8));
        portLineEdit->setText(QApplication::translate("Dialog", "1337", 0, QApplication::UnicodeUTF8));
        plainTextEdit->setPlainText(QApplication::translate("Dialog", "This is a message from Me", 0, QApplication::UnicodeUTF8));
        sendButton->setText(QApplication::translate("Dialog", "&Send", 0, QApplication::UnicodeUTF8));
        sendButton->setShortcut(QApplication::translate("Dialog", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        connectButton->setText(QApplication::translate("Dialog", "C&onnect", 0, QApplication::UnicodeUTF8));
        connectButton->setShortcut(QApplication::translate("Dialog", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        quitButton->setText(QApplication::translate("Dialog", "&Quit", 0, QApplication::UnicodeUTF8));
        quitButton->setShortcut(QApplication::translate("Dialog", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
