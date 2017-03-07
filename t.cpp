#include <stdio.h>
#include <iostream>
#include "XmlParser.hpp"
//#include "t.hpp"

int main( int argc, char *argv[] ) {
    {
    XmlElement x, q, qr, c, y, tmp;


    try {
	x.load( argc>1?argv[1]:"doc.xml" );		// l�gg documentet i 'x'
	q.load( argc>2?argv[2]:"query.xml" );		// l�gg fr�gan i 'q'
	// while( 1 ) {
	    tmp = x;
	    tmp = tmp["svsxml"];
	    tmp = tmp["get_live_service_update_response"];
	    tmp = tmp["live_header"];
	    tmp = tmp["prel_result"];
	    cout << "tmp:" << endl;
	    cout << tmp << endl;
	// }
    }
    catch( XmlException &xe ) {
        cout << xe << endl;
    }

    cout << "--- Document" << endl;

    cout << x;		// accessa en tag direkt med []
    					// finns inte taggen s� kastas
					// XmlException
    /*
       �r du s�ker p� flera niv�er i dokumentet kan du skriva
       cout << x["tag1"]["tag2"]["tag3"].Value;

       F�r att f� ut "Hejsan" ur detta:
       <tag1>
	   <tag2>
	       <tag3>Hejsan</tag3>
	   </tag2>
       </tag1>
    */

    try {
	cout << "--- Query" << endl;
	cout << q;

	qr = x( q ).getParent();	// fr�ga med (). kastar
					// XmlQueryException om fr�gan
					// misslyckas
	cout << "  --- Fr�gan st�lld" << endl;
	cout << qr;
	cout << qr.getParent().getFirst(15);
	cout << "  --- matchnummer ute" << endl;

	/*
	cout << match["hemmalag"].Value << "(" << match["hemmaresultat"].Value
	     << ") mot " << match["bortalag"].Value << "("
	     << match["bortaresultat"].Value << ")" << endl;
	*/

    }
    catch( XmlQueryException &xqe ) {
        cout << "XmlQueryException:" << xqe << endl;
    }
    catch( XmlException &xe ) {
        cout << "XmlException:" << xe << endl;
    }
    cout << "endast dtor:s kvar" << endl;
    }

    return 0;
}
