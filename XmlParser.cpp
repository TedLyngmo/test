/*
**
**  XmlParser - (c) 2004 Ted Lyngmo
**
*/

#include <fstream>
#include <sstream>
#include <cstdlib>
#include "XmlParser.hpp"

std::string trim(const std::string& in) {
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
XmlException::XmlException( const std::string& what_arg ) :
    std::runtime_error(what_arg) {}

 // for C++11 and later the below should be a delegating constructor to XmlException(std::string)
XmlException::XmlException( const char* what_arg ) :
     std::runtime_error(std::string(what_arg)) {}

XmlException::~XmlException() throw() {}
//-----------------------------------------------------------------------------
XmlQueryException::XmlQueryException(const XmlQueryException& qe) :
    XmlException(qe.what()), Query(qe.Query) {}

//-----------------------------------------------------------------------------
XmlQueryException::XmlQueryException(const XmlElement& doc, const XmlElement& query) :
    XmlException(std::string("Query on \"")+doc.Name+std::string("\" failed")), Query(query) {
}
//-----------------------------------------------------------------------------
XmlQueryException::~XmlQueryException() throw() {}
//-----------------------------------------------------------------------------
#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x0540) // Borland C++ Builder 4
# define BCBCONST
#else
# define BCBCONST const
#endif
static struct {
    BCBCONST char character;
    BCBCONST char* string;
} entities[] = {
    {'&',  "amp"},
    {'<',  "lt"},
    {'>',  "gt"},
    {'"',  "quot"},
    {'\'', "apos"}
};
#undef BCBCONST
//-----------------------------------------------------------------------------
std::string& fromxml(std::string& inout) {
    size_t pos_start=std::string::npos, pos_end;

    while( (pos_start=inout.find_last_of('&', pos_start)) != std::string::npos ) {
        pos_end=inout.find_first_of(';', pos_start);
        if(pos_end==std::string::npos || (pos_start+1)>=(pos_end-1)) throw XmlException("Invalid XML string");

        std::string entity = inout.substr(pos_start+1, pos_end-pos_start-1);

        for(size_t ent=0; ent<sizeof(entities); ++ent) {
            if( entity == entities[ent].string ) {
                inout.replace(pos_start, pos_end-pos_start+1, &entities[ent].character, 1);
                break;
            }
        }

        if(pos_start) --pos_start;
        else break;
    }
    return inout;
}
//-----------------------------------------------------------------------------
std::string& toxml(std::string& inout) {
    size_t pos=std::string::npos;

    while( (pos=inout.find_last_of("&<>\"'", pos)) != std::string::npos ) {
        switch(inout[pos]) {
            case '&':
                inout.replace(pos, 1, "&amp;");
                break;
            case '<':
                inout.replace(pos, 1, "&lt;");
                break;
            case '>':
                inout.replace(pos, 1, "&gt;");
                break;
            case '"':
                inout.replace(pos, 1, "&quot;");
                break;
            case '\'':
                inout.replace(pos, 1, "&apos;");
                break;
        }
        if(pos) --pos;
        else break;
    }
    return inout;
}
//-----------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, xmlstring& xs) {
    std::stringstream ss;
    // get everything until < is encountered
    is.get(*ss.rdbuf(), '<'); // get streambuf ref - men " då?
    xs = ss.str();
    // see if we got any forbidden characters
    if( xs.find_first_of(">\"'") == std::string::npos ) {
        // & needs care
        fromxml(xs);
    } else {
        is.setstate(std::ios::failbit);
    }
    return is;
}
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const xmlstring& xs) {
    std::string cpy(xs);
    toxml(cpy);
    os << cpy;
    return os;
}
//-----------------------------------------------------------------------------

// XmlElement
// ****************************************************************************
XmlElement::XmlElement() :
    Parent(NULL),
    elements(),
    Current(begin()),
    Name(),
    Value(),
    TagType('/'),
    isNull(true),
    isParameter(false)
    //, CompAlg(stringEqual)
{}

#include <stdio.h>
//static FILE *fp=NULL;
//static int dep=0;
XmlElement::XmlElement(const XmlElement &cpy, XmlElement* parent) :
    Parent(parent),
    elements(),
    Current(begin()),
    Name(cpy.Name),
    Value(cpy.Value),
    TagType(cpy.TagType),
    isNull(cpy.isNull),
    isParameter(cpy.isParameter)
{
    XmlElement::const_iterator ci = cpy.begin();

    while( ci != cpy.end() ) {
        elements.push_back( new XmlElement( **ci, this ) );
        ci++;
    }
}

XmlElement::XmlElement(const std::string& string_to_parse) :
    Parent(NULL),
    elements(),
    Current(begin()),
    Name(),
    Value(),
    TagType('/'),
    isNull(true),
    isParameter(false)
{
    parse(string_to_parse);
}

XmlElement::XmlElement(const char* string_to_parse) :
    Parent(NULL),
    elements(),
    Current(begin()),
    Name(),
    Value(),
    TagType('/'),
    isNull(true),
    isParameter(false)
{
    parse(std::string(string_to_parse));
}

XmlElement::XmlElement( XmlElement *parent, const std::string& name, xmlstring value, bool isnull ) :
    Parent(parent),
    elements(),
    Current(begin()),
    Name(name),
    Value(value),
    TagType('/'),
    isNull(isnull),
    isParameter(false)
{}

const char* XmlElement::c_str() const {
    return Value.c_str();
}

int XmlElement::to_int() const {
    return std::atoi(c_str());
}

size_t XmlElement::length() {
    return Value.length();
}

void XmlElement::Clear() {
    XmlElement::const_iterator ci = begin();
    XmlElement *tmp;

    //cout << "Clearing: " << Name << "=" << Value << endl;
    while( ci != end() ) {
        tmp = *ci;
        ci++;
        delete tmp;
    }
    elements.clear();
}

XmlElement::~XmlElement() {
    Clear();
}

inline void XmlElement::setParent( XmlElement *parent ) {
    Parent = parent;
}

XmlElement& XmlElement::getCurrent() {
    return **Current;
}

void XmlElement::setCurrent( const XmlElement *cur ) {
    Current = begin();
    while( Current != end() ) {
        if( *Current == cur ) break;
	Current++;
    }
//    if( Current == end() ) throw( XmlException(0,string("Not my child")) );
}

XmlElement& XmlElement::getParent( unsigned int step ) {
    XmlElement *ret=this, *last;
    while( step-- && ret ) {
        last=ret;
	ret=ret->Parent;
	if(ret) ret->setCurrent( last );
    }
    if(!ret) throw XmlException("<"+Name+">.getParent() failed");
    return *ret;
}

XmlElement& XmlElement::copy(const XmlElement &cpy) {
    Clear();

    Parent      = cpy.Parent;
    Current     = begin();
    Name        = cpy.Name;
    Value       = cpy.Value;
    TagType     = cpy.TagType;
    isNull      = cpy.isNull;
    isParameter = cpy.isParameter;

    XmlElement::const_iterator ci = cpy.begin();

    while( ci != cpy.end() ) {
        elements.push_back( new XmlElement( **ci, this ) );
        ci++;
    }
    return *this;
}
XmlElement& XmlElement::operator=(const XmlElement& cpy) {
    if(this == &cpy) return *this; // noop

    // this is needed if one element sets itself to one of its subelements
    XmlElement tmpcpy(cpy);
    copy(tmpcpy);

    return *this;
}

XmlElement &XmlElement::operator=( std::string value ) {
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

bool XmlElement::operator==( std::string value ) {
    return Value == value;
}

bool XmlElement::operator!=( std::string value ) {
    return Value != value;
}

XmlElement& XmlElement::addParameters(const std::string& in_parameters) {
    std::string name;
    xmlstring value;
    size_t spos, epos;
    std::string parameters(in_parameters);
    while( parameters.length() ) {
        spos = parameters.find( "=", 0 );
	if( spos == std::string::npos )
	    throw( XmlException( std::string("Invalid XML document, no '=' found.") ) );

	name = trim(parameters.substr(0, spos));
	if( !name.length() )
	    throw XmlException( std::string("Invalid XML document, no parameter name.") );

        spos = parameters.find( "\"", spos );
	if( spos == std::string::npos ) {
	    spos = parameters.find( "'", spos );
	    if( spos == std::string::npos )
		throw XmlException( std::string("Invalid XML document, start '\"' missing.") );
	}

        epos = parameters.find( "\"", spos+1 );
	if( epos == std::string::npos ) {
	    epos = parameters.find( "'", spos+1 );
	    if( epos == std::string::npos )
		throw XmlException( std::string("Invalid XML document, end '\"' missing.") );
	}

	value = parameters.substr( spos+1, epos-spos-1 );
	parameters = parameters.substr( epos+1, parameters.length()-epos+1 );

	addElement(name, value).isParameter = true;
    }
    return *this;
}

XmlElement& XmlElement::parse(const std::string& xmldata) {
    XmlElement *cur=this, *tmp;
    std::string element_name, value, parameters;
    int ch;
    size_t pos=0, spos, epos, ppos, len=xmldata.length();
    bool hasendtag;

    Clear();
    while( pos<len && cur ) {
        spos=pos;
        while( pos<len && xmldata.at(pos)!='<' ) pos++;
	if( pos<len ) {
	    cur->Value = trim(xmldata.substr(spos,pos-spos)); // set the value of the current XmlElement
            fromxml(cur->Value);
	    epos=pos;
	    while( epos<len && xmldata.at(epos)!='>' ) epos++;


	    if( epos<len ) {
		element_name = trim(xmldata.substr(pos+1,epos-pos-1));
		if( element_name.length()==0 ) throw XmlException( std::string("Invalid XML document") );

		ch = element_name.at(element_name.length()-1);
		if( ch=='/' || ch=='?' || ch=='!' ) {
		    hasendtag=true;
		    element_name = trim(element_name.substr(0,element_name.length()-1));
		} else hasendtag=false;

		if( element_name.length()==0 ) throw XmlException( std::string("Invalid XML document") );

		ppos = element_name.find( " ", 0 );
		if( ppos != std::string::npos ) {
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
    if( pos<len || !cur ) throw XmlException( std::string("Invalid XML document") );

    return *this;
}

XmlElement& XmlElement::load(const std::string& file ) {
    std::string doc;
    ifstream fs( file.c_str(), ios::in | ios::binary );
    char ch;
    if( fs ) {
	while( !fs.eof() ) {
	    fs.get( ch );
	    doc = doc + ch;
	}
	fs.close();
	parse( doc );
    } else throw XmlException(std::string("Could not open file") );

    return *this;
}

XmlElement &XmlElement::addElement( std::string element_name, xmlstring element_value ) {
    // cout << Name << "=" << Value << ".add:" << element_name << "=" << element_value << "." << endl;
    elements.push_back( new XmlElement( this, trim(element_name), trim(element_value) ) );
    setCurrent( elements.back() );
    return( *elements.back() );
}

XmlElement *XmlElement::query_r( XmlElement &Query, int depth, int &max_depth_found,
                                 XmlElement *best_so_far ) {
    XmlElement *retval;
    XmlElement::iterator cu, qi=Query.begin();

    depth++;

    // cout << depth << " " << Query.Name << endl;

    while( qi != Query.end() && max_depth_found != -1 ) {		// query iterator loop start
        cu=begin();
	while( cu != end() && max_depth_found != -1 ) {		// loop on doc being queried

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
	if( cu == end() ) {				// qi not found, at least not in this subquery
	    best_so_far=NULL;
	    break;
	}
	qi++;
    }								// query iterator loop end
    return best_so_far;
}

XmlElement& XmlElement::operator()( XmlElement &Query ) {
    int		depth=0,
		max_depth_found=0;
    XmlElement*	retval = query_r(Query, depth, max_depth_found, NULL);

    // cout << "max_depth_found: " << max_depth_found << endl;
    if( retval ) return *retval;
    else throw XmlQueryException(*this, Query);
}

XmlElement& XmlElement::operator()(std::string query_string) {
    if(query_string.length()==0) throw XmlException("operator() on <"+Name+"> with empty string");

    if( query_string[0] == '<' ) {	// depth search
	XmlElement query(query_string);
	return operator()( query );
    } else {				// same as operator[] - but inserts missing elements instead of throwing
	try {
	    return operator[]( query_string );
	} catch( ... ) {
	    return addElement( query_string );
	}
    }
}

XmlElement& XmlElement::operator[]( std::string element_name ) {
    Current=begin();

    while( Current != end() ) {
        if( (*Current)->Name == element_name ) {
	    return **Current;
	}
        Current++;
    }
    throw XmlException( "<"+element_name+"> not in <"+Name+">" );
}

std::string XmlElement::str() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

static ostream& outstream( std::ostream &os, const XmlElement &tag, int indent ) {
    XmlElement::const_iterator cu = tag.begin();
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

	if( cu != tag.end() && (*cu)->isParameter ) {
	    hasParameter=true;
	} else {
	    hasParameter=false;
	    if( isRealTag ) {
		if( tag.Value.length() || cu != tag.end() ) {
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
	    while( cu != tag.end() ) {
		if( !(*cu)->isParameter ) {
		    if( wasParameter ) {
			wasParameter = false;
			os << ">" << tag.Value << "\n";
		    }
		} else wasParameter=true;

		outstream( os, **cu, indent+(isRealTag?2:0) );
		cu++;
	    }
	    ind=indent;
	    if( wasParameter ) {
	        if( !tag.Value.length() ) os << " " << char(tag.TagType);
		os << ">" << tag.Value;
		if( !tag.Value.length() ) os << "\n";
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

ostream& operator<<( std::ostream &os, const XmlElement &Element ) {
    return outstream( os, Element, 0 );
}

