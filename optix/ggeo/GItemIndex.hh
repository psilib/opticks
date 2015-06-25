#pragma once

class GColors ; 
class GColorMap ; 
class GBuffer ; 

class Index ; 

#include "string.h"
#include <string>
#include <map>
#include <vector>

// adds colors and gui to basis Index constituent 
class GItemIndex {
       // friend class GSurfaceIndex ; 
       // friend class GMaterialIndex ; 
   public:
        static GItemIndex* load(const char* idpath, const char* itemtype);
   public:
        GItemIndex(Index* index);
        GItemIndex(const char* itemtype);
   public:
        void add(const char* name, unsigned int source);
        unsigned int getIndexLocal(const char* name, unsigned int missing=0);
        unsigned int getNumItems();
        Index*       getIndex();
        //const char* getItemType();
   public:
        void loadIndex(const char* idpath);
   private:
        void init(const char* itemtype);
   public:
        void save(const char* idpath);
        void dump(const char* msg="GItemIndex::dump");
        void test(const char* msg="GItemIndex::test", bool verbose=true);

   public:
        // color
        void     setColorSource(GColors* colors);
        void     setColorMap(GColorMap* colormap);
        GBuffer* makeColorBuffer();
        GBuffer* getColorBuffer();
   public:
        // GUI 
        void     gui();
        void     formTable();

   private:
        Index*                               m_index ; 
   private:
        GColors*                             m_colors ; 
        GColorMap*                           m_colormap ; 
        GBuffer*                             m_colorbuffer ; 
   private:
        // populated by formTable
        std::vector<std::string>             m_labels ; 
        std::vector<unsigned int>            m_codes ; 
};

inline GItemIndex::GItemIndex(const char* itemtype)
   : 
   m_index(NULL),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL)
{
   init(itemtype);
}

inline GItemIndex::GItemIndex(Index* index)
   : 
   m_index(index),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL)
{
}

inline void GItemIndex::setColorSource(GColors* colors)
{
   m_colors = colors ; 
}
inline void GItemIndex::setColorMap(GColorMap* colormap)
{
   m_colormap = colormap ; 
}

inline Index* GItemIndex::getIndex()
{
   return m_index ; 
}
