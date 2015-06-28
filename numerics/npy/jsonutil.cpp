#include "jsonutil.hpp"

#include "regexsearch.hh"

#include <string>
#include <iostream>
#include <iomanip>

#include <boost/algorithm/string.hpp> 
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;


template<typename A, typename B> 
void saveMap( typename std::map<A,B> & mp, const char* dir, const char* name)
{
    fs::path fdir(dir);
    if(!fs::exists(fdir))
    {
        if (fs::create_directory(fdir))
        {
            LOG(info)<< "saveMap : created directory " << dir ;
        }
    }
    if(fs::exists(fdir) && fs::is_directory(fdir))
    {
        fs::path fpath(dir);
        fpath /= name ;

        const char* path = fpath.string().c_str() ;
        LOG(info) << "saveMap to " << path ;

        saveMap( mp, path );
    }
    else
    {
        LOG(warning)<< "saveMap : FAILED to create directory " << dir ;
    }
}



template<typename A, typename B> 
void saveMap( typename std::map<A,B> & mp, const char* path)
{
    pt::ptree t;
    for(typename std::map<A,B>::iterator it=mp.begin() ; it != mp.end() ; it++)
    {
        t.put( 
           boost::lexical_cast<std::string>(it->first), 
           boost::lexical_cast<std::string>(it->second)
             ) ;
    }
    fs::path fpath(path);

    std::string ext = fpath.extension().string();

    if(ext.compare(".json")==0)
        pt::write_json(path, t );
    else if(ext.compare(".ini")==0)
        pt::write_ini(path, t );
    else
        LOG(warning) << "saveMap cannot write map to path with extension " << ext ; 

}



bool existsMap(const char* dir, const char* name )
{
    fs::path fdir(dir);
    if(fs::exists(fdir) && fs::is_directory(fdir))
    {
        fs::path fpath(dir);
        fpath /= name ;
        return fs::exists(fpath ) && fs::is_regular_file(fpath) ; 
    }
  
    return false ; 
}


template<typename A, typename B> 
void loadMap( typename std::map<A,B> & mp, const char* dir, const char* name)
{
    fs::path fdir(dir);
    if(fs::exists(fdir) && fs::is_directory(fdir))
    {
        fs::path fpath(dir);
        fpath /= name ;

        const char* path = fpath.string().c_str() ;
        std::string prefix = os_path_expandvars("$LOCAL_BASE/env/geant4/geometry/export/");  // cosmetic shortening only

        if(strncmp(path, prefix.c_str(), strlen(prefix.c_str()))==0)
              LOG(info) << "loadMap " << path + strlen(prefix.c_str()) ;
        else
              LOG(info) << "loadMap " << path  ;

        loadMap( mp, path );
    }
    else
    {
        LOG(fatal)<< "loadMap : no such directory " << dir ;
    }
}


template<typename A, typename B> 
void loadMap( typename std::map<A,B> & mp, const char* path)
{
    pt::ptree t;
    pt::read_json(path, t );

    BOOST_FOREACH( pt::ptree::value_type const& ab, t.get_child("") )
    {
         A a = boost::lexical_cast<A>(ab.first.data());
         B b = boost::lexical_cast<B>(ab.second.data());
         mp[a] = b ;         
    }
}

template<typename A, typename B> 
void dumpMap( typename std::map<A,B> & mp, const char* msg)
{
    LOG(info) << msg ; 
    for(typename std::map<A,B>::iterator it=mp.begin() ; it != mp.end() ; it++)
    {
         std::cout << std::setw(25) << boost::lexical_cast<std::string>(it->first)
                   << std::setw(50) << boost::lexical_cast<std::string>(it->second)
                   << std::endl ; 
                   ;
    }
}



// explicit instantiation of template functions, 
// allowing declaration and definition to reside in separate header and implementation files

template void saveMap<unsigned int, std::string>(std::map<unsigned int, std::string>& mp, const char* dir, const char* name) ;
template void saveMap<unsigned int, std::string>(std::map<unsigned int, std::string>& mp, const char* path) ;
template void loadMap<unsigned int, std::string>(std::map<unsigned int, std::string>& mp, const char* dir, const char* name) ;
template void loadMap<unsigned int, std::string>(std::map<unsigned int, std::string>& mp, const char* path) ;
template void dumpMap<unsigned int, std::string>(std::map<unsigned int, std::string>& mp, const char* msg) ;

template void saveMap<std::string, unsigned int>(std::map<std::string, unsigned int>& mp, const char* dir, const char* name ) ;
template void saveMap<std::string, unsigned int>(std::map<std::string, unsigned int>& mp, const char* path) ;
template void loadMap<std::string, unsigned int>(std::map<std::string, unsigned int>& mp, const char* dir, const char* name ) ;
template void loadMap<std::string, unsigned int>(std::map<std::string, unsigned int>& mp, const char* path) ;
template void dumpMap<std::string, unsigned int>(std::map<std::string, unsigned int>& mp, const char* msg) ;

template void saveMap<std::string, std::string>(std::map<std::string, std::string>& mp, const char* dir, const char* name ) ;
template void saveMap<std::string, std::string>(std::map<std::string, std::string>& mp, const char* path) ;
template void loadMap<std::string, std::string>(std::map<std::string, std::string>& mp, const char* dir, const char* name ) ;
template void loadMap<std::string, std::string>(std::map<std::string, std::string>& mp, const char* path) ;
template void dumpMap<std::string, std::string>(std::map<std::string, std::string>& mp, const char* msg) ;


