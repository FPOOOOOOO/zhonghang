/****************************************************************************
** Resource object code
**
** Created by: The Resource Compiler for Qt version 5.14.2
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

static const unsigned char qt_resource_data[] = {
  // C:/Users/FPO/Desktop/zhonghang/Upper/zhonghang-QTWIN/Qt_UDP/qt.conf
  0x0,0x0,0x0,0x36,
  0x5b,
  0x50,0x6c,0x61,0x74,0x66,0x6f,0x72,0x6d,0x73,0x5d,0x5b,0x50,0x6c,0x61,0x74,0x66,
  0x6f,0x72,0x6d,0x73,0x5d,0xa,0x57,0x69,0x6e,0x64,0x6f,0x77,0x73,0x41,0x72,0x67,
  0x75,0x6d,0x65,0x6e,0x74,0x73,0x3d,0x64,0x70,0x69,0x61,0x77,0x61,0x72,0x65,0x6e,
  0x65,0x73,0x73,0x3d,0x30,
  
};

static const unsigned char qt_resource_name[] = {
  // qt
  0x0,0x2,
  0x0,0x0,0x7,0x84,
  0x0,0x71,
  0x0,0x74,
    // etc
  0x0,0x3,
  0x0,0x0,0x6c,0xa3,
  0x0,0x65,
  0x0,0x74,0x0,0x63,
    // qt.conf
  0x0,0x7,
  0x8,0x74,0xa6,0xa6,
  0x0,0x71,
  0x0,0x74,0x0,0x2e,0x0,0x63,0x0,0x6f,0x0,0x6e,0x0,0x66,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/qt
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x2,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/qt/etc
  0x0,0x0,0x0,0xa,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x3,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/qt/etc/qt.conf
  0x0,0x0,0x0,0x16,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x83,0x11,0x6f,0xb8,0xb0,

};

#ifdef QT_NAMESPACE
#  define QT_RCC_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
#  define QT_RCC_MANGLE_NAMESPACE0(x) x
#  define QT_RCC_MANGLE_NAMESPACE1(a, b) a##_##b
#  define QT_RCC_MANGLE_NAMESPACE2(a, b) QT_RCC_MANGLE_NAMESPACE1(a,b)
#  define QT_RCC_MANGLE_NAMESPACE(name) QT_RCC_MANGLE_NAMESPACE2( \
        QT_RCC_MANGLE_NAMESPACE0(name), QT_RCC_MANGLE_NAMESPACE0(QT_NAMESPACE))
#else
#   define QT_RCC_PREPEND_NAMESPACE(name) name
#   define QT_RCC_MANGLE_NAMESPACE(name) name
#endif

#ifdef QT_NAMESPACE
namespace QT_NAMESPACE {
#endif

bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

#ifdef QT_NAMESPACE
}
#endif

int QT_RCC_MANGLE_NAMESPACE(qInitResources_qt)();
int QT_RCC_MANGLE_NAMESPACE(qInitResources_qt)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qRegisterResourceData)
        (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_qt)();
int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_qt)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qUnregisterResourceData)
       (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

namespace {
   struct initializer {
       initializer() { QT_RCC_MANGLE_NAMESPACE(qInitResources_qt)(); }
       ~initializer() { QT_RCC_MANGLE_NAMESPACE(qCleanupResources_qt)(); }
   } dummy;
}
