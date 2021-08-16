#pragma once

struct QU
{
    template <typename T>
    static char typecode() ; 

    template <typename T>
    static std::string rng_sequence_reldir(const char* prefix, unsigned ni, unsigned nj, unsigned nk, unsigned ni_tranche_size );

    template <typename T>
    static std::string rng_sequence_name(const char* prefix, unsigned ni, unsigned nj, unsigned nk, unsigned ioffset );

    template <typename T>
    static T* UploadArray(const T* array, unsigned num_items ) ;    

    template <typename T>
    static T* DownloadArray(const T* array, unsigned num_items ) ;    

    template <typename T>
    static T*   device_alloc( unsigned num_items ) ; 

    template <typename T>
    static void device_free( T* d ) ; 

    template <typename T>
    static void copy_device_to_host( T* h, T* d,  unsigned num_items);

    template <typename T>
    static void copy_device_to_host_and_free( T* h, T* d,  unsigned num_items);

    template <typename T>
    static void copy_host_to_device( T* d, const T* h,  unsigned num_items);


    static void ConfigureLaunch( dim3& numBlocks, dim3& threadsPerBlock, unsigned width, unsigned height );
    static void ConfigureLaunch2D( dim3& numBlocks, dim3& threadsPerBlock, unsigned width, unsigned height );

};

