/****************************************************************************
** Meta object code from reading C++ file 'serialcommunication.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../serialcommunication.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serialcommunication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SerialCommunication_t {
    QByteArrayData data[9];
    char stringdata0[106];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SerialCommunication_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SerialCommunication_t qt_meta_stringdata_SerialCommunication = {
    {
QT_MOC_LITERAL(0, 0, 19), // "SerialCommunication"
QT_MOC_LITERAL(1, 20, 20), // "slot_parse_rx_buffer"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 10), // "rx_byteArr"
QT_MOC_LITERAL(4, 53, 15), // "Command_Handler"
QT_MOC_LITERAL(5, 69, 7), // "uint8_t"
QT_MOC_LITERAL(6, 77, 7), // "uiAckId"
QT_MOC_LITERAL(7, 85, 14), // "ByteArray_Data"
QT_MOC_LITERAL(8, 100, 5) // "uiLen"

    },
    "SerialCommunication\0slot_parse_rx_buffer\0"
    "\0rx_byteArr\0Command_Handler\0uint8_t\0"
    "uiAckId\0ByteArray_Data\0uiLen"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SerialCommunication[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x0a /* Public */,
       4,    3,   27,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QByteArray, 0x80000000 | 5,    6,    7,    8,

       0        // eod
};

void SerialCommunication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SerialCommunication *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slot_parse_rx_buffer((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: _t->Command_Handler((*reinterpret_cast< uint8_t(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2])),(*reinterpret_cast< uint8_t(*)>(_a[3]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SerialCommunication::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_SerialCommunication.data,
    qt_meta_data_SerialCommunication,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SerialCommunication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialCommunication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SerialCommunication.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialCommunication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
