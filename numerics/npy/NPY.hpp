#pragma once

class G4StepNPY ; 

#include "numpy.hpp"
#include <vector>
#include <string>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "string.h"
#include "stdlib.h"
#include "assert.h"

class NPY {
   friend class G4StepNPY ; 

   public:
       static std::string path(const char* typ, const char* tag);
       static NPY* load(const char* typ, const char* tag);
       static NPY* make_vec3(float* m2w, unsigned int npo=100);  

       // ctor takes ownership of a copy of the inputs 
       NPY(std::vector<int>& shape, std::vector<float>& data, std::string& metadata) 
         :
         m_shape(shape),
         m_data(data),
         m_metadata(metadata)
      {
         m_len0 = getShape(0);
         m_len1 = getShape(1);
         m_len2 = getShape(2);
         m_dim  = m_shape.size();
      } 

       unsigned int getLength();
       unsigned int getDimensions();
       unsigned int getShape(unsigned int dim);
       unsigned int getNumFloats(unsigned int from_dim=1);
       unsigned int getNumBytes(unsigned int from_dim=1);
       float* getFloats();
       void* getBytes();
       unsigned int getFloatIndex(unsigned int i, unsigned int j, unsigned int k);
       unsigned int getByteIndex(unsigned int i, unsigned int j, unsigned int k);
       

       std::string description(const char* msg);

   protected:
       unsigned int       m_dim ; 
       unsigned int       m_len0 ; 
       unsigned int       m_len1 ; 
       unsigned int       m_len2 ; 

   private:
       std::vector<int>   m_shape ; 
       std::vector<float> m_data ; 
       std::string        m_metadata ; 

};


unsigned int NPY::getNumFloats(unsigned int from_dim)
{
    unsigned int nfloat = 1 ; 
    for(unsigned int i=from_dim ; i < m_shape.size() ; i++) nfloat *= m_shape[i] ;
    return nfloat ;  
}
unsigned int NPY::getNumBytes(unsigned int from_dim)
{
    assert(sizeof(float) == 4);
    return sizeof(float)*getNumFloats(from_dim);
}
unsigned int NPY::getDimensions()
{
    return m_shape.size();
}


unsigned int NPY::getShape(unsigned int n)
{
    return n < m_shape.size() ? m_shape[n] : -1 ;
}
unsigned int NPY::getLength()
{
    return getShape(0);
}


float* NPY::getFloats()
{
    return m_data.data();
}
void* NPY::getBytes()
{
    return (void*)getFloats();
}


unsigned int NPY::getByteIndex(unsigned int i, unsigned int j, unsigned int k)
{
    return sizeof(float)*getFloatIndex(i,j,k);
}

unsigned int NPY::getFloatIndex(unsigned int i, unsigned int j, unsigned int k)
{
    assert(m_dim == 3 ); 
    unsigned int nj = m_len1 ;
    unsigned int nk = m_len2 ;
    return  i*nj*nk + j*nk + k ;
}


std::string NPY::description(const char* msg)
{
    std::stringstream ss ; 

    ss << msg << " (" ;

    for(size_t i=0 ; i < m_shape.size() ; i++)
    {
        ss << m_shape[i]  ;
        if( i < m_shape.size() - 1) ss << "," ;
    }
    ss << ") " ;
    ss << " len0 " << m_len0 ;
    ss << " len1 " << m_len1 ;
    ss << " len2 " << m_len2 ;
    ss << " nfloat " << m_data.size() << " " ;

    ss << " getNumBytes(0) " << getNumBytes(0) ;
    ss << " getNumBytes(1) " << getNumBytes(1) ;
    ss << " getNumFloats(0) " << getNumFloats(0) ;
    ss << " getNumFloats(1) " << getNumFloats(1) ;

    ss << m_metadata  ;

    return ss.str();
}





std::string NPY::path(const char* typ, const char* tag)
{
    char* TYP = strdup(typ);
    char* p = TYP ;
    while(*p)
    {
       if( *p >= 'a' && *p <= 'z') *p += 'A' - 'a' ;
       p++ ; 
    } 

    char envvar[64];
    snprintf(envvar, 64, "DAE_%s_PATH_TEMPLATE", TYP ); 
    free(TYP); 

    char* tmpl = getenv(envvar) ;
    if(!tmpl) return "missing-template-envvar" ; 
    
    char path_[256];
    snprintf(path_, 256, tmpl, tag );

    return path_ ;   
}




NPY* NPY::load(const char* typ, const char* tag)
{
    std::string path = NPY::path(typ, tag);

    std::vector<int> shape ;
    std::vector<float> data ;
    std::string metadata = "{}";

    NPY* npy = NULL ;
    try 
    {
        aoba::LoadArrayFromNumpy<float>(path, shape, data );
        npy = new NPY(shape,data,metadata) ;
    } 
    catch(const std::runtime_error& error)
    {
        std::cout << "NPY::load failed " << std::endl ; 
    }


    return npy ;
}


NPY* NPY::make_vec3(float* m2w_, unsigned int npo)
{
    glm::mat4 m2w ;
    if(m2w_) m2w = glm::make_mat4(m2w_);

    std::vector<float> data;
    std::vector<int>   shape = {int(npo), 1, 3} ;
    std::string metadata = "{}";

    float scale = 1.f/float(npo);

    for(int i=0 ; i < npo ; i++ )
    {
        glm::vec4 m(float(i)*scale, float(i)*scale, float(i)*scale, 1.f);
        glm::vec4 w = m2w * m ;

        data.push_back(w.x);
        data.push_back(w.y);
        data.push_back(w.z);
    } 
    NPY* npy = new NPY(shape,data,metadata) ;
    return npy ;
}


