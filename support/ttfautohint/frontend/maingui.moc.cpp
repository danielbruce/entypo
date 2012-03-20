/****************************************************************************
** Meta object code from reading C++ file 'maingui.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "maingui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'maingui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Main_GUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      18,    9,    9,    9, 0x08,
      33,    9,    9,    9, 0x08,
      49,    9,    9,    9, 0x08,
      61,    9,    9,    9, 0x08,
      73,    9,    9,    9, 0x08,
      90,    9,    9,    9, 0x08,
     108,    9,    9,    9, 0x08,
     120,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Main_GUI[] = {
    "Main_GUI\0\0about()\0browse_input()\0"
    "browse_output()\0check_min()\0check_max()\0"
    "absolute_input()\0absolute_output()\0"
    "check_run()\0run()\0"
};

void Main_GUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Main_GUI *_t = static_cast<Main_GUI *>(_o);
        switch (_id) {
        case 0: _t->about(); break;
        case 1: _t->browse_input(); break;
        case 2: _t->browse_output(); break;
        case 3: _t->check_min(); break;
        case 4: _t->check_max(); break;
        case 5: _t->absolute_input(); break;
        case 6: _t->absolute_output(); break;
        case 7: _t->check_run(); break;
        case 8: _t->run(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Main_GUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Main_GUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Main_GUI,
      qt_meta_data_Main_GUI, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Main_GUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Main_GUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Main_GUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Main_GUI))
        return static_cast<void*>(const_cast< Main_GUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Main_GUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
