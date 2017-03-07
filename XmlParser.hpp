/*
**
**  XmlParser - (c) 2004 Ted Lyngmo
**
*/

#ifndef _xmlparser_
#define _xmlparser_

#include <string>
#include <list>
#include <exception>
#include <iostream>

using namespace std;

enum QueryComparison { stringEqual,stringNotEqual,numericEqual,numericLess,numericGreater,numericNotEqual };

class XmlException : public std::exception {
public:
    string Description;
    int Errno;
public:
   // XmlException( const XmlException &ex );
    XmlException( int ErrorCode=0, const char *Desc=NULL );
    XmlException( int ErrorCode=0, const string Desc="" );
    ~XmlException() throw();
    friend ostream& operator<<( ostream &os, const XmlException &ex ) {
	os << string("[") << ex.Description << "(" << ex.Errno << ")]";
	return os;
    }
//    const char *what() const throw();
};

class XmlElement;

class XmlQueryException : public XmlException {
public:
    XmlElement *Query;
    XmlQueryException( const XmlQueryException &ex );
    XmlQueryException( int ErrorCode, XmlElement &query );
    ~XmlQueryException() throw();
};

class XmlElement {
private:
    XmlElement &copy( const XmlElement &cpy );
public:
    static int InstanceCounter;
    
    XmlElement *Parent;
    std::list <XmlElement*> elements;	// sub elements / parameters
    std::list <XmlElement*>::iterator Current; // current sub element
    string		Name;		// Name of element
    string		Value;		// Value of element, if any
    int			TagType;	// ?, ! or /
    bool		isNull;		// is Value=="" a value, or is it not even set?
    bool		isParameter;	// as in <tagname parameter="" />
    QueryComparison	CompAlg;	// if this is a query, how should the values be compared?


    XmlElement();
    XmlElement( const XmlElement &copy );
    XmlElement( XmlElement *parent, string name, string value="", bool isnull=false );
    ~XmlElement();

    const char* c_str();
    size_t	length();
    void	Clear();
    XmlElement&	getCurrent()				throw( XmlException );
    void	setCurrent(const XmlElement *)		throw( XmlException );
    void	setParent( XmlElement *parent );
    XmlElement&	getParent( unsigned int step=1 )	throw( XmlException );
    XmlElement&	getFirst( unsigned int step=1 )		throw( XmlException );
    XmlElement&	getNext( unsigned int step=1 )		throw( XmlException );
    bool        atEnd();                                // if getFirst or getNext didn't return a valid element
    XmlElement&	parse( string xmldata )			throw( XmlException );
    XmlElement&	load( string file )			throw( XmlException );
    XmlElement&	addElement( string element_name="", string element_value="" );
    XmlElement&	addParameters( string parameters )	throw( XmlException );	// returns the element to which the parameters were added

    XmlElement*	query_r( XmlElement &query, int depth, int &max_depth_found, XmlElement *best_so_far );	// reentrant
    XmlElement&	operator()( XmlElement &query )		throw( XmlQueryException );
    XmlElement&	operator()( string query_string )	throw( XmlQueryException );	// dual purpose: If query_string starts with '<' the whole string is transformed into an XmlElement and a query is performed
    											//               but otherwise, query_string is actually used as an element_name. If found, treated as ["element_name"] otherwise inserted.
    XmlElement&	operator[]( string element_name )	throw( XmlException );

    XmlElement&	operator=( const XmlElement &rval );
    XmlElement&	operator=( string value );
    bool	operator==( XmlElement &rval );
    bool	operator!=( XmlElement &rval );
    bool	operator==( string value );
    bool	operator!=( string value );

    friend ostream& operator<<( ostream &os, const XmlElement &Element );

};

#endif
