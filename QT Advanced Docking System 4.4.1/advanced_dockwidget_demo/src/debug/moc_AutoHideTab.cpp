/****************************************************************************
** Meta object code from reading C++ file 'AutoHideTab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../AutoHideTab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AutoHideTab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ads__CAutoHideTab_t {
    QByteArrayData data[12];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ads__CAutoHideTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ads__CAutoHideTab_t qt_meta_stringdata_ads__CAutoHideTab = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ads::CAutoHideTab"
QT_MOC_LITERAL(1, 18, 25), // "onAutoHideToActionClicked"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 23), // "onDragHoverDelayExpired"
QT_MOC_LITERAL(4, 69, 21), // "setDockWidgetFloating"
QT_MOC_LITERAL(5, 91, 15), // "unpinDockWidget"
QT_MOC_LITERAL(6, 107, 22), // "requestCloseDockWidget"
QT_MOC_LITERAL(7, 130, 15), // "sideBarLocation"
QT_MOC_LITERAL(8, 146, 11), // "orientation"
QT_MOC_LITERAL(9, 158, 15), // "Qt::Orientation"
QT_MOC_LITERAL(10, 174, 9), // "activeTab"
QT_MOC_LITERAL(11, 184, 8) // "iconOnly"

    },
    "ads::CAutoHideTab\0onAutoHideToActionClicked\0"
    "\0onDragHoverDelayExpired\0setDockWidgetFloating\0"
    "unpinDockWidget\0requestCloseDockWidget\0"
    "sideBarLocation\0orientation\0Qt::Orientation\0"
    "activeTab\0iconOnly"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ads__CAutoHideTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       4,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x0a /* Public */,
       5,    0,   42,    2, 0x0a /* Public */,
       6,    0,   43,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       7, QMetaType::Int, 0x00095001,
       8, 0x80000000 | 9, 0x00095009,
      10, QMetaType::Bool, 0x00095001,
      11, QMetaType::Bool, 0x00095001,

       0        // eod
};

void ads::CAutoHideTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CAutoHideTab *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onAutoHideToActionClicked(); break;
        case 1: _t->onDragHoverDelayExpired(); break;
        case 2: _t->setDockWidgetFloating(); break;
        case 3: _t->unpinDockWidget(); break;
        case 4: _t->requestCloseDockWidget(); break;
        default: ;
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<CAutoHideTab *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->sideBarLocation(); break;
        case 1: *reinterpret_cast< Qt::Orientation*>(_v) = _t->orientation(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->isActiveTab(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->iconOnly(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject ads::CAutoHideTab::staticMetaObject = { {
    &CPushButton::staticMetaObject,
    qt_meta_stringdata_ads__CAutoHideTab.data,
    qt_meta_data_ads__CAutoHideTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ads::CAutoHideTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ads::CAutoHideTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ads__CAutoHideTab.stringdata0))
        return static_cast<void*>(this);
    return CPushButton::qt_metacast(_clname);
}

int ads::CAutoHideTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
