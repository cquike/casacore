//# tTable_4.cc: Interactive test program for adding/removing columns
//# Copyright (C) 2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <aips/Tables/TableDesc.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/ScaColDesc.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/StManAipsIO.h>
#include <aips/Tables/IncrementalStMan.h>
#include <aips/Tables/StandardStMan.h>
#include <aips/Tables/TiledShapeStMan.h>
#include <aips/Tables/TiledColumnStMan.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/ArrayLogical.h>
#include <aips/Arrays/ArrayUtil.h>
#include <aips/Arrays/ArrayIO.h>
#include <aips/Containers/Record.h>
#include <aips/Utilities/Regex.h>
#include <aips/Utilities/Assert.h>
#include <aips/Exceptions/Error.h>
#include <iostream.h>

// <summary>
// Interactive test program for adding/removing columns.
// </summary>


// First build a description.
TableDesc makeDesc (Bool ask)
{
  // Build the table description.
  TableDesc td("", "1", TableDesc::Scratch);
  String stman, stmanname;
  Int op;
  while (True) {
    try {
      if (ask) {
	cout << "0=end 1=scalar 2=dirarr 3=fixindarr 4=varindarr: ";
      }
      cin >> op;
      if (op >= 1  &&  op <= 4) {
	if (ask) {
	  cout << "Column name,stman (a=aipsio s=ssm i=ism t=tsm),stmanname: ";
	}
	String str;
	cin >> str;
	Vector<String> strs = stringToVector (str);
	AlwaysAssert (strs.nelements() >= 1  &&  strs.nelements() <= 3,
		      AipsError);
	stman = "StandardStMan";
	stmanname = "";
	if (strs.nelements() > 1) {
	  strs(1).downcase();
	  if (strs(1) == 'a') {
	    stman = "StManAipsIO";
	  } else if (strs(1) == 'i') {
	    stman = "IncrementalStMan";
	  } else if (strs(1) == 't') {
	    stman = "TiledShapeStMan";
	  } else if (strs(1) != 's') {
	    throw AipsError ("Invalid StMan given");
	  }
	  stmanname = stman;
	  if (strs.nelements() > 2) {
	    stmanname = strs(2);
	  }
	}

	if (op == 1) {
	    td.addColumn (ScalarColumnDesc<uInt>(strs(0), "", stman,
						 stmanname));
	  } else if (op == 2) {
	    td.addColumn (ArrayColumnDesc<uInt>(strs(0), "", stman, stmanname,
					      IPosition(1,10),
					      ColumnDesc::Direct));
	} else if (op == 3) {
	  td.addColumn (ArrayColumnDesc<uInt>(strs(0), "", stman, stmanname,
					      IPosition(1,10),
					      ColumnDesc::FixedShape));
	} else if (op == 4) {
	  td.addColumn (ArrayColumnDesc<uInt>(strs(0), "", stman, stmanname));
	}
      } else {
	break;
      }
    } catch (AipsError x) {
      cout << x.getMesg() << endl;
    }
  }
  // Create the hypercolumn descriptions for all tiled columns.
  SimpleOrderedMap<String,String> map("");
  for (uInt i=0; i<td.ncolumn(); i++) {
    const ColumnDesc& cd = td.columnDesc(i);
    if (cd.dataManagerType() == "TiledShapeStMan") {
      map(cd.dataManagerGroup()) += ("," + cd.name());
    }
  }
  for (uInt i=0; i<map.ndefined(); i++) {
    String cols = map.getVal(i);
    Vector<String> vec = stringToVector(cols.from(1));
    uInt ndim = 2;
    if (td.columnDesc(vec(0)).isScalar()) {
      ndim = 1;
    }
    td.defineHypercolumn (map.getKey(i), ndim, vec);
  }
  td.show (cout);
  return td;
}

void putData (Table& tab, const TableDesc& td,
	      uInt startrow, uInt nrow)
{
  for (uInt i=0; i<td.ncolumn(); i++) {
    const ColumnDesc& cdesc = td.columnDesc(i);
    if (cdesc.isScalar()) {
      ScalarColumn<uInt> col (tab, cdesc.name());
      for (uInt i=0; i<nrow; i++) {
	col.put (startrow+i, startrow+i);
      }
    } else {
      ArrayColumn<uInt> col (tab, cdesc.name());
      Vector<uInt> vec(10);
      for (uInt i=0; i<nrow; i++) {
	vec = startrow+i;
	col.put (startrow+i, vec);
      }
    }
  }
}

void checkData (const Table& tab, const TableDesc& td,
		uInt startrow, uInt nrow)
{
  for (uInt i=0; i<td.ncolumn(); i++) {
    const ColumnDesc& cdesc = td.columnDesc(i);
    if (cdesc.isScalar()) {
      ROScalarColumn<uInt> col (tab, cdesc.name());
      for (uInt i=0; i<nrow; i++) {
	AlwaysAssert (col(startrow+i) == startrow+i, AipsError);
      }
    } else {
      ROArrayColumn<uInt> col (tab, cdesc.name());
      Vector<uInt> vec(10);
      for (uInt i=0; i<nrow; i++) {
	vec = startrow+i;
	AlwaysAssert (allEQ(col(startrow+i), vec), AipsError);
      }
    }
  }
}

void addCols (Bool ask, Table& tab)
{
  TableDesc tdn = makeDesc(ask);
  AlwaysAssert (tdn.ncolumn() > 0, AipsError);
  const ColumnDesc& cdesc = tdn.columnDesc(0);
  if (tdn.ncolumn() == 1) {
    if (cdesc.dataManagerType() == cdesc.dataManagerGroup()) {
      tab.addColumn (cdesc, cdesc.dataManagerType(), False);
    } else {
      tab.addColumn (cdesc, cdesc.dataManagerGroup(), True);
    }
  } else {
    if (cdesc.dataManagerType() == "StManAipsIO") {
      tab.addColumn (tdn, StManAipsIO(cdesc.dataManagerGroup()));
    } else if (cdesc.dataManagerType() == "IncrementalStMan") {
      tab.addColumn (tdn, IncrementalStMan(cdesc.dataManagerGroup()));
    } else if (cdesc.dataManagerType() == "StandardStMan") {
      tab.addColumn (tdn, StandardStMan(cdesc.dataManagerGroup()));
    } else if (cdesc.dataManagerType() == "TiledColumnStMan") {
      tab.addColumn (tdn, TiledColumnStMan(cdesc.dataManagerGroup(),
					   IPosition(2,10,2)));
    } else {
      tab.addColumn (tdn, TiledShapeStMan(cdesc.dataManagerGroup(),
					  IPosition(2,10,2)));
    }
  }
  putData (tab, tdn, 0, tab.nrow());
  cout << " Added and initialized " << tdn.ncolumn() << " columns" << endl;
}

void doTable (Bool ask, const TableDesc& td)
{
  // Now create a new table from the description.
  // Use copy constructor to test if it works fine.
  // (newtab and newtabcp have the same underlying object).
  SetupNewTable newtab("tTable_4_tmp.data", td, Table::New);
  Table tab(newtab);

  Int op;
  while (True) {
    try {
      if (ask) {
	cout << "0=end 1=reopen 2=addcols 3=removecols 4=addrow 5=show "
	        "6=check, 7=refcol: ";
      }
      cin >> op;
      if (op == 1) {
	tab = Table();
	tab = Table("tTable_4_tmp.data", Table::Update);
	cout << " Reopened table" << endl;
      } else if (op == 2) {
	addCols (ask, tab);
      } else if (op == 3) {
	String str;
	if (ask) {
	  cout << "Column names: ";
	}
	cin >> str;
	tab.removeColumn (stringToVector(str));
	cout << " Removed columns " << str << endl;
      } else if (op == 4) {
	uInt n = tab.nrow();
	tab.addRow();
	putData (tab, tab.tableDesc(), n, 1);
	cout << " Added and initialized 1 row" << endl;
      } else if (op == 5  ||  op == 7) {
	Table reftab(tab);
	if (op == 7) {
	  String str;
	  if (ask) {
	    cout << "Column names: ";
	  }
	  cin >> str;
	  Block<String> cols;
	  stringToVector(str).toBlock (cols);
	  reftab = tab.project (cols);
	}
	reftab.actualTableDesc().show (cout);
	Record rec = reftab.dataManagerInfo();
	cout << "Data Managers:" << endl;
	for (uInt i=0; i<rec.nfields(); i++) {
	  const Record& subrec = rec.subRecord(i);
	  cout << " Type=" << subrec.asString("TYPE");
	  cout << " Name=" << subrec.asString("NAME");
	  cout << " Columns=" << subrec.asArrayString("COLUMNS");
	  cout << endl;
	}
	cout << "Table has " << tab.nrow() << " rows" << endl << endl;
      } else if (op == 6) {
	checkData (tab, tab.tableDesc(), 0, tab.nrow());
	cout << " Checked all data" << endl;
      } else {
	break;
      }
    } catch (AipsError x) {
      cout << x.getMesg() << endl;
    }
  }
}

int main (int argc)
{
  try {
    cout << "tTable_4 is for interactive playing with tables" << endl;
    cout << "-----------------------------------------------" << endl;
    Bool ask = argc < 2;
    doTable (ask, makeDesc(ask));
  } catch (AipsError x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}
