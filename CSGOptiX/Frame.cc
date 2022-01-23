#include <iostream>
#include <iomanip>
#include <csignal>

#include <cuda_runtime.h>
#include "QU.hh"

#include "SPath.hh"
#include "PLOG.hh"
#include "NP.hh"
#include "Frame.h"

#define SIMG_IMPLEMENTATION 1 
#include "SIMG.hh"


const plog::Severity Frame::LEVEL = PLOG::EnvLevel("Frame", "DEBUG" ); 


Frame::Frame(int width_, int height_, int depth_)
    :
    width(width_),
    height(height_),
    depth(depth_),
    channels(4),
    img(new SIMG(width, height, channels,  nullptr )) 
{
    init();
}


uchar4* Frame::getDevicePixel() const { return d_pixel ;  }
float4* Frame::getDeviceIsect() const  { return d_isect ;  }
quad4*  Frame::getDevicePhoton() const  { return d_photon ;  }


/**
Frame::init
-------------

Allocates pixels and isect on device. 

**/
void Frame::init()
{
    assert( depth == 1 ); 
    unsigned num_pixels = width*height ; 
    QU::device_free_and_alloc<uchar4>( &d_pixel,  num_pixels );  
    QU::device_free_and_alloc<float4>( &d_isect,  num_pixels );  
    QU::device_free_and_alloc<quad4>(  &d_photon, num_pixels );  
}

/**
Frame::download
----------------

From GPU buffers into vectors.

**/
void Frame::download()
{
    unsigned num_pixels = width*height ; 
    QU::Download<uchar4>(pixel, d_pixel, num_pixels ); 
    QU::Download<float4>(isect, d_isect, num_pixels ); 
    QU::Download<quad4>(photon, d_photon, num_pixels ); 

    img->setData( getPixelData() ); 
}

unsigned char* Frame::getPixelData() const {     return (unsigned char*)pixel.data();  }
float*         Frame::getIntersectData() const { return (float*)isect.data(); }
float*         Frame::getPhotonData() const {    return (float*)photon.data(); }


void Frame::annotate( const char* bottom_line, const char* top_line, int line_height )
{
    img->annotate( bottom_line, top_line, line_height ); 
}

void Frame::write(const char* outdir_, int jpg_quality) const 
{
    int create_dirs = 2 ; // 2:create directory path argument
    const char* outdir = SPath::Resolve(outdir_, create_dirs); 
    writePNG(outdir, "f_pixels.png");  
    writeJPG(outdir, "f_pixels.jpg", jpg_quality);  
    writeIsect(outdir, "f_isect.npy" ); // formerly posi.npy
    writePhoton(outdir, "f_photon.npy" ); 
}

void Frame::writePNG(const char* dir, const char* name) const 
{
    img->writePNG(dir, name); 
}
void Frame::writePNG(const char* path) const 
{
    img->writePNG(path); 
}

void Frame::writeJPG(const char* dir, const char* name, int quality) const 
{
    img->writeJPG(dir, name, quality); 
}
void Frame::writeJPG(const char* path, int quality) const 
{
    img->writeJPG(path, quality); 
}


void Frame::writeIsect( const char* dir, const char* name) const 
{
    NP::Write(dir, name, getIntersectData(), height, width, 4 );
}
void Frame::writePhoton( const char* dir, const char* name) const 
{
    NP::Write(dir, name, getPhotonData(), height, width, 4, 4 );
}

