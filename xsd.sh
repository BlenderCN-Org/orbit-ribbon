#!/bin/sh
rm -f cpp/autoxsd/*.cpp
rm -f cpp/autoxsd/*.h

# Generate C++/Tree code for the ORE package description schema
xsdcxx cxx-tree --type-naming ucc --guard-prefix ORBIT_RIBBON_AUTOXSD --hxx-suffix ".h" --cxx-suffix ".cpp" --generate-doxygen --generate-serialization --output-dir cpp/autoxsd/ xml/orepkgdesc.xsd

# Don't want to use 'long long' since ISO C++ doesn't support it
sed 's/long long/long/;' cpp/autoxsd/orepkgdesc.h > cpp/autoxsd/orepkgdesc.h2
mv -f cpp/autoxsd/orepkgdesc.h2 cpp/autoxsd/orepkgdesc.h

# Generate C++/Parser pskels for loading models and animations from ORE files
xsdcxx cxx-parser --guard-prefix ORBIT_RIBBON_AUTOXSD --hxx-suffix ".h" --cxx-suffix ".cpp" --output-dir cpp/autoxsd xml/oreanimation.xsd xml/oremeshtype.xsd xml/oremesh.xsd