/****************************************************************************
** Meta object code from reading C++ file 'dialog.h'
**
** Created: Wed Dec 30 07:09:54 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dialog[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      16,    8,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,    7,    7,    7, 0x08,
      69,    7,    7,    7, 0x08,
      85,    7,    7,    7, 0x08,
     104,    7,    7,    7, 0x08,
     115,    7,    7,    7, 0x08,
     138,  126,    7,    7, 0x08,
     181,    7,    7,    7, 0x08,
     195,    8,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Dialog[] = {
    "Dialog\0\0rawList\0receivedNewContactList(QString)\0"
    "activateSendButton()\0connectToHost()\0"
    "processReadyRead()\0sendPing()\0sendNick()\0"
    "socketError\0displayError(QAbstractSocket::SocketError)\0"
    "sendMessage()\0parseList(QString)\0"
};

const QMetaObject Dialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Dialog,
      qt_meta_data_Dialog, 0 }
};

const QMetaObject *Dialog::metaObject() const
{
    return &staticMetaObject;
}

void *Dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dialog))
        return static_cast<void*>(const_cast< Dialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: receivedNewContactList((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: activateSendButton(); break;
        case 2: connectToHost(); break;
        case 3: processReadyRead(); break;
        case 4: sendPing(); break;
        case 5: sendNick(); break;
        case 6: displayError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 7: sendMessage(); break;
        case 8: parseList((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Dialog::receivedNewContactList(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
