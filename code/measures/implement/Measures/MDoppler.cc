//# MDoppler.cc: A Measure: Doppler shift
//# Copyright (C) 1995,1996,1997,1998
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//# Includes
#ifdef __GNUG__
#include <aips/Quanta/Quantum.h>
typedef Quantum<Double> gpp_mdoppler_bug1;
#endif
#include <aips/Utilities/Assert.h>
#include <aips/RTTI/Register.h>
#include <aips/Measures/MDoppler.h>

//# Constructors
MDoppler::MDoppler() :
  MeasBase<MVDoppler,MDoppler::Ref>() {}

MDoppler::MDoppler(const MVDoppler &dt) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,MDoppler::DEFAULT) {}

MDoppler::MDoppler(const MVDoppler &dt, const MDoppler::Ref &rf) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,rf) {}

MDoppler::MDoppler(const MVDoppler &dt, uInt rf) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,rf) {}

MDoppler::MDoppler(const Quantity &dt) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,MDoppler::DEFAULT) {}

MDoppler::MDoppler(const Quantity &dt, const MDoppler::Ref &rf) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,rf) {}

MDoppler::MDoppler(const Quantity &dt, uInt rf) : 
  MeasBase<MVDoppler,MDoppler::Ref>(dt,rf) {}

MDoppler::MDoppler(const Measure *dt) :
  MeasBase<MVDoppler,MDoppler::Ref>(dt) {}

MDoppler::MDoppler(const MeasValue *dt) :
  MeasBase<MVDoppler,MDoppler::Ref>(*(MVDoppler*)dt,
				MDoppler::DEFAULT) {}

//# Destructor
MDoppler::~MDoppler() {}

//# Operators

//# Member functions
const String &MDoppler::tellMe() const {
    return MDoppler::showMe();
}

const String &MDoppler::showMe() {
    static const String name("Doppler");
    return name;
}

uInt MDoppler::type() const {
  return Register((MDoppler *)0);
}

void MDoppler::assert(const Measure &in) {
  if (in.type() != Register((MDoppler *)0)) {
    throw(AipsError("Illegal Measure type argument: " +
		    MDoppler::showMe()));
  };
}

const String &MDoppler::showType(uInt tp) {
    static const String tname[MDoppler::N_Types] = {
	"RADIO", 
	"OPTICAL",
	"RATIO",
	"TRUE",
	"GAMMA"};

    DebugAssert(tp < MDoppler::N_Types, AipsError);
    return tname[tp];
}

const String *const MDoppler::allMyTypes(Int &nall, Int &nextra,
					  const uInt *&typ) {
  static const Int N_name  = 8;
  static const Int N_extra = 0;
  static const String tname[N_name] = {
    "RADIO", 
    "Z",
    "RATIO",
    "BETA",
    "GAMMA",
    "OPTICAL",
    "TRUE",
    "RELATIVISTIC" };
  
  static const uInt oname[N_name] = {
    MDoppler::RADIO, 
    MDoppler::Z,
    MDoppler::RATIO,
    MDoppler::BETA,
    MDoppler::GAMMA,
    MDoppler::Z,
    MDoppler::BETA,
    MDoppler::BETA };

  nall   = N_name;
  nextra = N_extra;
  typ    = oname;
  return tname;
}

const String *const MDoppler::allTypes(Int &nall, Int &nextra,
					const uInt *&typ) const {
  return MDoppler::allMyTypes(nall, nextra, typ);
}

Bool MDoppler::getType(MDoppler::Types &tp, const String &in) {
  const uInt *oname;
  Int nall, nex;
  const String *tname = MDoppler::allMyTypes(nall, nex, oname);
  
  Int i = Measure::giveMe(in, nall, tname);
  
  if (i>=nall) {
    return False;
  } else {
    tp = (MDoppler::Types) oname[i];
  };
  return True;
}

Bool MDoppler::giveMe(MDoppler::Ref &mr, const String &in) {
  MDoppler::Types tp;
  if (MDoppler::getType(tp, in)) {
    mr = MDoppler::Ref(tp);
  } else {
    mr = MDoppler::Ref();
    return False;
  };
  return True;
};

Bool MDoppler::giveMe(const String &in, MDoppler::Ref &mr) {
  return MDoppler::giveMe(mr, in);
}

Bool MDoppler::setOffset(const Measure &in) {
  if (in.type() != Register((MDoppler *)0)) return False;
  ref.set(in);
  return True;
}

Bool MDoppler::setRefString(const String &in) {
  MDoppler::Types tp;
  if (MDoppler::getType(tp, in)) {
    ref.setType(tp);
    return True;
  };
  ref.setType(MDoppler::DEFAULT);
  return False;
}

const String &MDoppler::getDefaultType() const {
  return MDoppler::showType(MDoppler::DEFAULT);
}

String MDoppler::getRefString() const {
  return MDoppler::showType(ref.getType());
}
uInt MDoppler::myType() {
  return Register((MDoppler *)0);
}

Quantity MDoppler::get(const Unit &un) const {
    return data.get(un);
}
    
Measure *MDoppler::clone() const {
  return (new MDoppler(*this));
}
