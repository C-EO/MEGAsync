#ifndef QTMETAENUMUTILS_H
#define QTMETAENUMUTILS_H

#include <QtGlobal>

#include <type_traits>

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
// Convert enum to quint64 while preserving negative values (sign-extend to 64 bits)
// Required for QMetaEnum::valueToKey() in Qt 6.9+
template<typename Enum>
quint64 toQtMetaEnumValue(Enum e)
{
    using UT = std::underlying_type_t<Enum>;
    return static_cast<quint64>(static_cast<qint64>(static_cast<UT>(e)));
}
#else
template<typename Enum>
int toQtMetaEnumValue(Enum e)
{
    using UT = std::underlying_type_t<Enum>;
    return static_cast<int>(static_cast<UT>(e));
}
#endif

#endif // QTMETAENUMUTILS_H
