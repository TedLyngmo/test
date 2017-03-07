/*
**
**  XmlParser - (c) 2004 Ted Lyngmo
**
*/

#include <fstream>
#include "XmlParser.hpp"

string trim( string in ) {
    size_t spos;
    int epos;
    size_t in_len;

    in_len = in.length();

    for( spos=0; spos<in_len; spos++ ) {
        if( (unsigned char) in[spos] > ' ' ) break;
    }
    if( spos < in_len ) {
	for( epos=(in_len-1); epos>=0; epos-- ) {
	    if( (unsigned char) in[epos] > ' ' ) break;
	}
        epos++;
    } else {
        spos=0;
        epos=0;
    }
//    cout << ">" << endl << in << endl << "<.substr(" << spos << "," << (epos-spos) << ")=[" << in.substr(spos,epos-spos) << "]" << endl;
    return in.substr(spos,epos-spos);
}

// XmlExcepions
// ****************************************************************************

XmlException::XmlException( int ErrorCode, const string Desc ) {
    Errno = ErrorCode;
    Description = Desc;
}

/*
XmlException::XmlException( const XmlException &xe ) {
    Errno	= xe.Errno;
    Description	= xe.Description;
}

XmlException &XmlException::operator=( const XmlException &cpy ) {
    Errno = cpy.Errno;
    Description = cpy.Description;
    return *this;
}
*/

XmlException::~XmlException() throw() {
}

XmlQueryException::XmlQueryException( const XmlQueryException &qe ) : XmlException( qe.Errno, qe.Description ) {
    Query = new XmlElement( *qe.Query );
}

XmlQueryException::XmlQueryException( int ErrorCode, XmlElement &query ) : XmlException( ErrorCode, query.Name+"="+query.Value ) {
    Query = new XmlElement( query );
}

XmlQueryException::~XmlQueryException() throw() {
    //cout << "del XmlQueryException query " << endl;
    delete Query;
    //cout << "after XmlQueryException query " << endl;
}

// XmlElement
// ****************************************************************************
int XmlElement::InstanceCounter=0;

XmlElement::XmlElement() {
    InstanceCounter++;
    Parent	= NULL;
    Name	= "";
    Value	= "";
    isNull	= true;
    isParameter	= false;
    TagType	= '/';
    Current	= elements.begin();
}

#include <stdio.h>
//static FILE *fp=NULL;
//static int dep=0;
XmlElement::XmlElement( const XmlElement &cpy ) {
    InstanceCounter++;
    XmlElement &tmp = (XmlElement&) cpy;
    std::list<XmlElement*>::iterator ci;
    ci=tmp.elements.begin();

//    cout << "Copy Ctor Parent=" << cpy.Parent->Name << " this=" << cpy.Name << endl;
    Parent	= cpy.Parent;
    Name	= cpy.Name;
    Value	= cpy.Value;
    isNull	= cpy.isNull;
    isParameter	= cpy.isParameter;
    TagType	= cpy.TagType;
    Current	= elements.begin();
//    if( fp ) fprintf( fp, "%*scpy 1 %s=%s\n", ++dep,"", tmp.Name.c_str(), tmp.Value.c_str() );
    while( ci != tmp.elements.end() ) {
//        if( fp ) fprintf( fp, "%*scpy 2 %s=%s\n", dep,"", (*ci)->Name.c_str(), (*ci)->Value.c_str() );
        elements.push_back( new XmlElement( **ci ) );
//        if( fp ) fprintf( fp, "%*scpy 3\n", dep,"" );
        ci++;
    }
//    if( fp ) fprintf( fp, "%*scpy 4\n", dep--,"" );
}

XmlElement::XmlElement( XmlElement *parent, string name, string value, bool isnull ) {
    InstanceCounter++;
    Parent	= parent;
    Name	= name;
    Value	= value;
    isNull	= isnull;
    isParameter	= false;
    TagType	= '/';
}

const char* XmlElement::c_str() {
    return Value.c_str();
}

size_t XmlElement::length() {
    return Value.length();
}

void XmlElement::Clear() {
    std::list<XmlElement*>::iterator ci=elements.begin();
    XmlElement *tmp;

    //cout << "Clearing: " << Name << "=" << Value << endl;
    while( ci != elements.end() ) {
        tmp = *ci;
        ci++;
        delete tmp;
    }
    elements.clear();
}

XmlElement::~XmlElement() {
    InstanceCounter--;
    Clear();
}

inline void XmlElement::setParent( XmlElement *parent ) {
    Parent = parent;
}

XmlElement& XmlElement::getCurrent() throw( XmlException ) {
    return **Current;
}

void XmlElement::setCurrent( const XmlElement *cur ) throw( XmlException ) {
    Current = elements.begin();
    while( Current != elements.end() ) {
        if( *Current == cur ) break;
	Current++;
    }
//    if( Current == elements.end() ) throw( XmlException(0,string("Not my child")) );
}


XmlElement& XmlElement::getFirst( unsigned int step ) throw( XmlException ) {
    Current = elements.begin();

    while( --step && Current != elements.end() ) {
        Current++;
    }

    if( Current == elements.end() ) return *this; // throw( XmlException(0,string("No elements")) );

    return **Current;
}

XmlElement& XmlElement::getNext( unsigned int step ) throw( XmlException ) {
    while( step-- && Current != elements.end() ) Current++;
    if( Current == elements.end() ) return *this; // throw( XmlException(0,string("No more elements")) );
    return **Current;
}

bool XmlElement::atEnd() {
    return (Current == elements.end());
}

XmlElement& XmlElement::getParent( unsigned int step ) throw( XmlException ) {
    XmlElement *ret=this, *last;
    while( step-- && ret ) {
        last=ret;
	ret=ret->Parent;
	ret->setCurrent( last );
    }
    if( !ret ) throw XmlException(0,Name) ;
    return *ret;
}

XmlElement &XmlElement::copy( const XmlElement &cpy ) {
    Clear();
    XmlElement &tmp = (XmlElement&) cpy;		// hide the const
    std::list<XmlElement*>::iterator ci;
    ci=tmp.elements.begin();

    Parent	= cpy.Parent;
    Name	= cpy.Name;
    Value	= cpy.Value;
    isNull	= cpy.isNull;

    while( ci != tmp.elements.end() ) {
        elements.push_back( new XmlElement( **ci ) );
        ci++;
    }
    return *this;
}
XmlElement &XmlElement::operator=( const XmlElement &cpy ) {
//    if( fp ) fprintf( fp, "operator= 1\n" );
    XmlElement tmpcpy;
    tmpcpy.copy( cpy );  // needed if one element sets itself to one of its subelements
//    if( fp ) fprintf( fp, "operator= 1.5\n" );
    copy(tmpcpy);

    return *this;

/*
    XmlElement &tmp = tmpcpy;		// hide the const

    std::list<XmlElement*>::iterator ci;
    ci=tmp.elements.begin();

//    cout << "operator= Parent=" << cpy.Parent->Name << " this=" << cpy.Name << endl;
    Parent	= cpy.Parent;
    Name	= cpy.Name;
    Value	= cpy.Value;
    isNull	= cpy.isNull;
//    if( fp ) fprintf( fp, "operator= 2\n" );
    while( ci != tmp.elements.end() ) {
//        if( fp ) fprintf( fp, "operator= 3 %s=%s\n", (*ci)->Name.c_str(), (*ci)->Value.c_str() );
        elements.push_back( new XmlElement( **ci ) );
//        if( fp ) fprintf( fp, "operator= 3.5\n" );
        ci++;
    }
//    if( fp ) fprintf( fp, "operator= 4\n" ); fflush( fp );
    return *this;
    */
}

XmlElement &XmlElement::operator=( string value ) {
    Value = value;
    isNull = false;
    return *this;
}

bool XmlElement::operator==( XmlElement &rval ) {
    return this == &rval;
}

bool XmlElement::operator!=( XmlElement &rval ) {
    return this != &rval;
}

bool XmlElement::operator==( string value ) {
    return Value == value;
}

bool XmlElement::operator!=( string value ) {
    return Value != value;
}

XmlElement& XmlElement::addParameters( string parameters ) throw( XmlException ) {
    string name, value;
    size_t spos, epos;
    while( parameters.length() ) {
        spos = parameters.find( "=", 0 );
	if( spos == string::npos )
	    throw( XmlException( 0, string("Invalid XML document, no '=' found.") ) );

	name = trim(parameters.substr( 0, spos));
	if( !name.length() )
	    throw( XmlException( 0, string("Invalid XML document, no parameter name.") ) );

        spos = parameters.find( "\"", spos );
	if( spos == string::npos ) {
	    spos = parameters.find( "'", spos );
	    if( spos == string::npos )
		throw( XmlException( 0, string("Invalid XML document, start '\"' missing.") ) );
	}

        epos = parameters.find( "\"", spos+1 );
	if( epos == string::npos ) {
	    epos = parameters.find( "'", spos+1 );
	    if( epos == string::npos )
		throw( XmlException( 0, string("Invalid XML document, end '\"' missing.") ) );
	}

	value = parameters.substr( spos+1, epos-spos-1 );
	parameters = parameters.substr( epos+1, parameters.length()-epos+1 );

	addElement(name,value).isParameter = true;
    }
    return *this;
}

void Replace( string& in, string Find, string with ) {
    size_t pos=0;
    size_t Len = Find.length();
    while( (pos = in.find(Find,pos)) != std::string::npos ) {
	in.replace(pos,Len,with);
    }
}

XmlElement& XmlElement::parse( string xmldata ) throw( XmlException ) {
    XmlElement *cur=this, *tmp;
    string element_name, value, parameters;
    int ch;
    size_t pos=0, spos, epos, ppos, len=xmldata.length();
    bool hasendtag;

    Clear();
    while( pos<len && cur ) {
        spos=pos;
        while( pos<len && xmldata.at(pos)!='<' ) pos++;
	if( pos<len ) {
	    cur->Value = trim(xmldata.substr(spos,pos-spos)); // set the value of the current XmlElement
	    Replace(cur->Value,"&amp;","&");
	    Replace(cur->Value,"&lt;","<");
	    Replace(cur->Value,"&gt;",">");
       	    Replace(cur->Value,"&apos;","'");
       	    Replace(cur->Value,"&quot;","\"");            

	    // cout << "cur=" << cur->Name << "=" << cur->Value << "." <<endl;
	    epos=pos;
	    while( epos<len && xmldata.at(epos)!='>' ) epos++;


	    if( epos<len ) {
		element_name = trim(xmldata.substr(pos+1,epos-pos-1));
		if( element_name.length()==0 ) throw( XmlException( 0, string("Invalid XML document") ) );

		ch = element_name.at(element_name.length()-1);
		if( ch=='/' || ch=='?' || ch=='!' ) {
		    hasendtag=true;
		    element_name = trim(element_name.substr(0,element_name.length()-1));
		} else hasendtag=false;

		if( element_name.length()==0 ) throw( XmlException( 0, string("Invalid XML document") ) );

		ppos = element_name.find( " ", 0 );
		if( ppos != string::npos ) {
		    parameters = trim(element_name.substr(ppos+1,element_name.length()-ppos));
		    element_name = element_name.substr(0,ppos);
		} else parameters="";

		if( element_name.at(0)=='?' || element_name.at(0)=='!' ) {	// special elements
		    if( element_name.at(0)!='!' ) {
			tmp = &cur->addElement(element_name);		// Does not have an end tag
			tmp->TagType = element_name.at(0);
			tmp->addParameters( parameters );
		    } else {
			// tmp->Value = parameters;			// just remove comments for now
		    }
		} else if( element_name.at(0)=='/' ) {			// end tag
		    cur = &cur->getParent();
		} else if( hasendtag ) {				// start+end tag
		    tmp = &cur->addElement(element_name);
		    tmp->addParameters( parameters );
		} else { // new tag
		    cur = &cur->addElement(element_name);		// Add value or new tag container
		    cur->addParameters( parameters );
		}
	    }
	    pos=epos+1;
	}
    }
    // this check does not work
    if( pos<len || !cur ) throw( XmlException( 0, string("Invalid XML document") ) );

    return *this;
}

XmlElement& XmlElement::load( string file ) throw( XmlException ) {
    string doc;
    ifstream fs( file.c_str(), ios::in | ios::binary );
    char ch;
    if( fs ) {
	while( !fs.eof() ) {
	    fs.get( ch );
	    doc = doc + ch;
	}
	fs.close();
	parse( doc );
    } else throw( XmlException(0, string("Could not open file") ) );

    return *this;
}

XmlElement &XmlElement::addElement( string element_name, string element_value ) {
    // cout << Name << "=" << Value << ".add:" << element_name << "=" << element_value << "." << endl;
    elements.push_back( new XmlElement( this, trim(element_name), trim(element_value) ) );
    setCurrent( elements.back() );
    return( *elements.back() );
}

XmlElement *XmlElement::query_r( XmlElement &Query, int depth, int &max_depth_found,
                                 XmlElement *best_so_far ) {
    XmlElement *retval;
    std::list<XmlElement*>::iterator cu, qi=Query.elements.begin();

    depth++;

    // cout << depth << " " << Query.Name << endl;

    while( qi != Query.elements.end() && max_depth_found != -1 ) {		// query iterator loop start
        cu=elements.begin();
	while( cu != elements.end() && max_depth_found != -1 ) {		// loop on doc being queried

	    /*
	    cout << "qi " << (*qi)->Name << "= \"" << (*qi)->Value << "\", ";
	    cout << "cu " << (*cu)->Name << "= \"" << (*cu)->Value << "\"." << endl;;
	    */

	    // match
	    if( (*cu)->Name == (*qi)->Name && ((*cu)->Value == (*qi)->Value || (*qi)->isNull) ) {
	        if( (*qi)->elements.size() ) {					// has subquery
		    retval = (*cu)->query_r( **qi, depth, max_depth_found, best_so_far );
		    if( retval ) {						// subquery succeeded
			Current = cu;
			//cout << "this: " << this->Name <<  "Current: " << (*Current)->Name << endl;
			best_so_far = retval;
			break;
		    } else {							// subquery failed
		    								// continue on this level
		    }
		} else {							// found a leaf
		    if( depth>max_depth_found ) {				// a new hi score?
			max_depth_found = depth;
			best_so_far = *cu;
			Current = cu;
			//cout << "this: " << *this <<  "Current: " << **Current << endl;
		    }
		    break;
		}
	    }
	    cu++;
	}							// end of loop on doc
	if( cu == elements.end() ) {				// qi not found, at least not in this subquery
	    best_so_far=NULL;
	    break;
	}
	qi++;
    }								// query iterator loop end
    return best_so_far;
}

XmlElement& XmlElement::operator()( XmlElement &Query ) throw( XmlQueryException ) {
    int		depth=0,
		max_depth_found=0;
    XmlElement*	retval = query_r(Query,depth,max_depth_found,NULL);

    // cout << "max_depth_found: " << max_depth_found << endl;
    if( retval ) return *retval;
    else throw( XmlQueryException( 0, Query ) );
}

XmlElement& XmlElement::operator()( string query_string ) throw( XmlQueryException ) {
    if( query_string[0] == '<' ) {	// depth search
	XmlElement query;
	query.parse( query_string );
	return operator()( query );
    } else {				// same as operator[] - but inserts missing elements instead of throwing
	try {
	    return operator[]( query_string );
	} catch( ... ) {
	    return addElement( query_string );
	}
    }
}

XmlElement& XmlElement::operator[]( string element_name ) throw( XmlException ) {
    Current=elements.begin();

    while( Current != elements.end() ) {
        if( (*Current)->Name == element_name ) {
	    return **Current;
	}
        Current++;
    }
    throw XmlException( 0, "<"+element_name+"> not in <"+Name+">" );
}

static ostream& outstream( ostream &os, const XmlElement &tag, int indent ) {
    std::list<XmlElement*>::const_iterator cu=tag.elements.begin();
    bool hasParameter;
    bool wasParameter=false;
    int ind=indent;

    if( tag.isParameter ) {
	os << " " << tag.Name << "=\"" << tag.Value << "\"";
    } else {
	bool isRealTag = (tag.Name.length()!=0);

	if( isRealTag ) {
	    //os << "*";
	    while( ind-- ) os << " ";
	    os << "<" << tag.Name;
	}
	/* else {
	    os << "^";
	}*/

	bool bNeedsClosing=true;

	if( cu != tag.elements.end() && (*cu)->isParameter ) {
	    hasParameter=true;
	} else {
	    hasParameter=false;
	    if( isRealTag ) {
		if( tag.Value.length() || cu != tag.elements.end() ) {
		    os << ">" << tag.Value;
		} else {
		    os << " />";
		    bNeedsClosing = false;
		}
	    }
	}

	if( tag.elements.size() ) {
	    if( isRealTag && !hasParameter ) os << endl;
	    //wasParameter = true;
	    while( cu != tag.elements.end() ) {
		if( !(*cu)->isParameter ) {
		    if( wasParameter ) {
			wasParameter = false;
			os << ">" << tag.Value << endl;
		    }
		} else wasParameter=true;

		outstream( os, **cu, indent+(isRealTag?2:0) );
		cu++;
	    }
	    ind=indent;
	    if( wasParameter ) {
	        if( !tag.Value.length() ) os << " " << char(tag.TagType);
		os << ">" << tag.Value;
		if( !tag.Value.length() ) os << endl;
	    } else if( isRealTag ) {
	       	while( ind-- ) os << " "; 
	    }
	}
	if( !wasParameter || tag.Value.length() ) {
	    if( isRealTag ) {
		if( bNeedsClosing ) {
		    os << "<" << "/" << tag.Name << ">";
		}
		os << endl;
	    }
	}
    }
    return os;
}

ostream& operator<<( ostream &os, const XmlElement &Element ) {
    return outstream( os, Element, 0 );
}
