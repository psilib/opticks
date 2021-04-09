#pragma once

#include <string>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

#include "plog/Severity.h"
#include "CFG4_API_EXPORT.hh"

struct CFG4_API CGDMLKludgeWrite 
{
    static const plog::Severity LEVEL ; 

    CGDMLKludgeWrite(xercesc::DOMDocument* doc); 
    virtual ~CGDMLKludgeWrite(); 

    void write(const char* path);

    xercesc::DOMElement* NewElement(const char* tagname);
    xercesc::DOMAttr*    NewAttribute(const char* name, const char* value);
    std::string          ConstantToMatrixValues(double value, double nm_lo, double nm_hi);  
    xercesc::DOMElement* ConstantToMatrixElement(const char* name, double value, double nm_lo, double nm_hi ); 


    xercesc::DOMDocument*   doc;
    xercesc::DOMElement*    extElement;
    xercesc::DOMElement*    userinfoElement;
    XMLCh                   tempStr[10000];

};
