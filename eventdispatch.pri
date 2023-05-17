INCLUDEPATH             += $$top_srcdir/eventdispatch/

win32{
QMAKE_CXXFLAGS          += /MP
LIBS                    += $$top_builddir/bin/eventdispatch.lib
}else{
LIBS                    += $$top_builddir/bin/libeventdispatch.so
}
