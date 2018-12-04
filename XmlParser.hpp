/*
**
**  XmlParser - (c) 2004 Ted Lyngmo
**
*/

#ifndef _xmlparser_
#define _xmlparser_

#include <string>
#include <list>
#include <stdexcept>
#include <iostream>

using namespace std;

//enum QueryComparison { stringEqual,stringNotEqual,numericEqual,numericLess,numericGreater,numericNotEqual };

class xmlstring : public std::basic_string<char> {
public:
    using std::basic_string<char>::operator=;

    xmlstring() : std::basic_string<char>() {}
    xmlstring(const char* str) : std::basic_string<char>(str) {}
    xmlstring(const std::basic_string<char>& str) : std::basic_string<char>(str) {}
    xmlstring& operator=(const char* str) {
        std::basic_string<char>::operator=(str);
        return *this;
    }

    friend std::istream& operator>>(std::istream&, xmlstring&);
    friend std::ostream& operator<<(std::ostream&, const xmlstring&);
};

std::istream& operator>>(std::istream&, xmlstring&);
std::ostream& operator<<(std::ostream&, const xmlstring&);


class XmlElement {
private:
    XmlElement& copy( const XmlElement &cpy );
public:
    XmlElement *Parent;
    std::list<XmlElement*> elements;	// sub elements / parameters

    typedef std::list<XmlElement*>::iterator iterator;
    typedef std::list<XmlElement*>::const_iterator const_iterator;

    iterator Current; // current sub element
    std::string		Name;		// Name of element
    xmlstring   	Value;		// Value of element, if any
    int			TagType;	// ?, ! or /
    bool		isNull;		// is Value=="" a value, or is it not even set?
    bool		isParameter;	// as in <tagname parameter="" />
    //QueryComparison	CompAlg;	// if this is a query, how should the values be compared?


    XmlElement();                       // default ctor
    XmlElement(const XmlElement &copy, XmlElement* parent=NULL); // copy ctor
    XmlElement(const std::string& string_to_parse);
    XmlElement(const char* string_to_parse);
    XmlElement(XmlElement *parent, const std::string& name, xmlstring value="", bool isnull=false);
    ~XmlElement();

    const char* c_str() const;          // Value as char* string
    int         to_int() const;         // Value as an integer
    std::string str() const;            // the complete doc as a string
    size_t	length();
    void	Clear();
    XmlElement&	getCurrent();
    void	setCurrent(const XmlElement *);
    void	setParent( XmlElement *parent );
    XmlElement&	getParent( unsigned int step=1 );
    XmlElement&	parse(const std::string& xmldata);
    XmlElement&	load(const std::string& file);
    XmlElement&	addElement(std::string element_name="", xmlstring element_value="" );
    XmlElement&	addParameters(const std::string& parameters);	// returns the element to which the parameters were added

    XmlElement*	query_r( XmlElement &query, int depth, int &max_depth_found, XmlElement *best_so_far );	// reentrant
    XmlElement&	operator()( XmlElement &query );
    XmlElement&	operator()( std::string query_string );	// dual purpose: If query_string starts with '<' the whole string is transformed into an XmlElement and a query is performed
    											//               but otherwise, query_string is actually used as an element_name. If found, treated as ["element_name"] otherwise inserted.
    XmlElement&	operator[]( const std::string element_name );

    XmlElement&	operator=( const XmlElement &rval );
    XmlElement&	operator=( std::string value );
    bool	operator==( XmlElement &rval );
    bool	operator!=( XmlElement &rval );
    bool	operator==( std::string value );
    bool	operator!=( std::string value );

    iterator begin() { return elements.begin(); }
    const_iterator begin() const { return elements.begin(); }
    iterator end() { return elements.end(); }
    const_iterator end() const { return elements.end(); }

    friend ostream& operator<<( std::ostream& os, const XmlElement& Element );

};

ostream& operator<<( std::ostream& os, const XmlElement& Element );

class XmlException : public std::runtime_error {
public:
    //XmlException();                             // default ctor
    //XmlException( const XmlException& ex );     // copy ctor
    //XmlException( XmlException&& ex );          // move ctor
    XmlException( const char *what_arg );
    XmlException( const std::string& desc );
    virtual ~XmlException() throw();

    friend ostream& operator<<( ostream& os, const XmlException& ex ) {
	os << ex.what();
	return os;
    }
};

class XmlQueryException : public XmlException {
public:
    XmlElement Query;
    XmlQueryException(const XmlQueryException& ex);
    XmlQueryException(const XmlElement& doc, const XmlElement& query);
    virtual ~XmlQueryException() throw();
};

#endif
