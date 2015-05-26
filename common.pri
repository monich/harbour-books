WARNINGS = -Wall -Wno-unused-parameter -Wno-deprecated-declarations
QMAKE_CXXFLAGS += $$WARNINGS -Wno-psabi
QMAKE_CFLAGS += $$WARNINGS

CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS_DEBUG *= -O0
  QMAKE_CFLAGS_DEBUG *= -O0
}
