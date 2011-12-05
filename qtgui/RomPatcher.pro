SOURCES = RomPatcher.cpp main.cpp
HEADERS = RomPatcher.h

CONFIG += qt

LIBS += ../lib/libmacrom.a

win32 {
    QMAKE_PRE_LINK += "windres --input ../qtgui/RomPatcher.rc --output RomPatcher.res --output-format=coff"
    LIBS += -lws2_32 RomPatcher.res
}
