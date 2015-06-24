#include "SequenceNPY.hpp"
#include "uif.h"
#include "NPY.hpp"
#include "RecordsNPY.hpp"
#include "Index.hpp"

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include <glm/glm.hpp>
#include "limits.h"
#include "GLMFormat.hpp"
#include "GLMPrint.hpp"


#include <boost/log/trivial.hpp>
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal



void SequenceNPY::setRecs(RecordsNPY* recs)
{
    m_recs = recs ; 
    m_maxrec = recs->getMaxRec();
}

void SequenceNPY::examinePhotonHistories()
{
    // find counts of all histories 
    typedef std::map<unsigned int,unsigned int>  MUU ; 
    MUU uu = m_photons->count_unique_u(3,3) ; 
    dumpMaskCounts("SequenceNPY::examinePhotonHistories : ", Types::HISTORY, uu, 1);
}

void SequenceNPY::prepSequenceIndex()
{
    unsigned int ni = m_photons->m_len0 ;

    typedef std::vector<unsigned int> VU ;
    VU mismatch ;

    typedef std::map<unsigned int, unsigned int> MUU ;
    MUU uuh ;  
    MUU uum ;  

    typedef std::map<std::string, unsigned int>  MSU ;
    MSU sum ;  
    MSU suh ;  

    typedef std::map<std::string, std::vector<unsigned int> >  MSV ;
    MSV svh ;  
    MSV svm ;  


    for(unsigned int i=0 ; i < ni ; i++) // over all photons
    { 
         unsigned int photon_id = i ; 

         // from the (upto m_maxrec) records for each photon 
         // fabricate history and material masks : by or-ing together the bits
         unsigned int history(0) ;
         unsigned int bounce(0) ;
         unsigned int material(0) ;
         m_recs->constructFromRecord(photon_id, bounce, history, material); 

         // compare with the photon mask, formed GPU side 
         // should match perfectly so long as bounce_max < maxrec 
         // (that truncates big-bouncers in the same way on GPU/CPU)
         unsigned int phistory = m_photons->getUInt(photon_id, 3, 3);
         if(history != phistory) mismatch.push_back(photon_id);
         assert(history == phistory);

         // map counting different history/material masks 
         uuh[history] += 1 ; 
         uum[material] += 1 ; 

         // construct sequences of materials or history flags for each step of the photon
         std::string seqmat = m_recs->getSequenceString(photon_id, Types::MATERIAL);
         std::string seqhis = m_recs->getSequenceString(photon_id, Types::HISTORY);


         if(i< 10)
         {
            printf("seqmat %s \n", seqmat.c_str());
            printf("seqhis %s \n", seqhis.c_str());
         } 

         // map counting difference history/material sequences
         suh[seqhis] += 1; 
         sum[seqmat] += 1 ; 

         // collect vectors of photon_id for each distinct sequence
         svh[seqhis].push_back(photon_id); 
         svm[seqmat].push_back(photon_id); 
    }
    assert( mismatch.size() == 0);

    printf("SequenceNPY::consistencyCheck photons %u mismatch %lu \n", ni, mismatch.size());
    dumpMaskCounts("SequenceNPY::consistencyCheck histories", Types::HISTORY, uuh, 1 );
    dumpMaskCounts("SequenceNPY::consistencyCheck materials", Types::MATERIAL, uum, 1000 );
    dumpSequenceCounts("SequenceNPY::consistencyCheck seqhis", Types::HISTORY, suh , svh, 1000);
    dumpSequenceCounts("SequenceNPY::consistencyCheck seqmat", Types::MATERIAL, sum , svm, 1000);


    Index* idxh = makeSequenceCountsIndex( Types::HISTORYSEQ,  suh , svh, 1000 );
    idxh->dump();
    fillSequenceIndex( e_seqhis , idxh, svh );

    Index* idxm = makeSequenceCountsIndex( Types::MATERIALSEQ,  sum , svm, 1000 );
    idxm->dump();
    fillSequenceIndex( e_seqmat, idxm, svm );

    m_seqhis = idxh ; 
    m_seqmat = idxm ; 
}


void SequenceNPY::fillSequenceIndex(
       unsigned int k,
       Index* idx, 
       std::map<std::string, std::vector<unsigned int> >&  sv 
)
{
    assert( k < 4 );
    unsigned int ni = m_photons->m_len0 ;
    NPY<unsigned char>* seqidx = getSeqIdx(); // creates if not exists

    unsigned int nseq(0) ; 

    for(unsigned int iseq=0 ; iseq < idx->getNumItems() ; iseq++)
    {
        unsigned int pseq = iseq + 1 ; // 1-based local seq index
        std::string seq = idx->getNameLocal(pseq); 
        typedef std::vector<unsigned int> VU ; 
        VU& pids = sv[seq];

        if(pseq >= 255)
        {
            LOG(warning) << "SequenceNPY::fillSequenceIndex TOO MANY SEQUENCES : TRUNCATING " ; 
            break ; 
        }

        for(VU::iterator it=pids.begin() ; it != pids.end() ; it++)
        {
            unsigned int photon_id = *it ;  
            seqidx->setValue(photon_id, 0, k, pseq );
            nseq++;
        }
    }

    std::cout << "SequenceNPY::fillSequenceIndex " 
              << std::setw(3) << k
              << std::setw(15) << idx->getItemType()
              << " sequenced/total " 
              << std::setw(7) << nseq 
              << "/"
              << std::setw(7) << ni
              << std::endl ; 

}



bool SequenceNPY::second_value_order(const std::pair<int,int>&a, const std::pair<int,int>&b)
{
    return a.second > b.second ;
}

void SequenceNPY::dumpMaskCounts(const char* msg, Types::Item_t etype, 
        std::map<unsigned int, unsigned int>& uu, 
        unsigned int cutoff)
{
    typedef std::map<unsigned int, unsigned int> MUU ;
    typedef std::pair<unsigned int, unsigned int> PUU ;

    std::vector<PUU> pairs ; 
    for(MUU::iterator it=uu.begin() ; it != uu.end() ; it++) pairs.push_back(*it);
    std::sort(pairs.begin(), pairs.end(), second_value_order );

    std::cout << msg << std::endl ; 

    unsigned int total(0);

    for(unsigned int i=0 ; i < pairs.size() ; i++)
    {
        PUU p = pairs[i];
        total += p.second ;  

        if(p.second > cutoff) 
            std::cout 
               << std::setw(5) << i 
               << " : "
               << std::setw(10) << std::hex << p.first
               << " : " 
               << std::setw(10) << std::dec << p.second
               << " : "
               << m_types->getMaskString(p.first, etype) 
               << std::endl ; 
    }

    std::cout 
              << " total " << total 
              << " cutoff " << cutoff 
              << std::endl ; 
}


bool SequenceNPY::su_second_value_order(const std::pair<std::string,unsigned int>&a, const std::pair<std::string,unsigned int>&b)
{
    return a.second > b.second ;
}


Index* SequenceNPY::makeSequenceCountsIndex(
       Types::Item_t etype, 
       std::map<std::string, unsigned int>& su,
       std::map<std::string, std::vector<unsigned int> >&  sv,
       unsigned int cutoff
       )
{
    Index* idx = new Index(m_types->getItemName(etype));

    typedef std::map<std::string, std::vector<unsigned int> >  MSV ;
    typedef std::map<std::string, unsigned int> MSU ;
    typedef std::pair<std::string, unsigned int> PSU ;

    // order by counts of that sequence
    std::vector<PSU> pairs ; 
    for(MSU::iterator it=su.begin() ; it != su.end() ; it++) pairs.push_back(*it);
    std::sort(pairs.begin(), pairs.end(), su_second_value_order );


    // populate idx with the sequences having greater than cutoff ocurrences
    unsigned int total(0);
    for(unsigned int i=0 ; i < pairs.size() ; i++)
    {
        PSU p = pairs[i];
        total += p.second ;  
        assert( sv[p.first].size() == p.second );
        if(p.second > cutoff)
            idx->add( p.first.c_str(), i );
    }

    std::cout 
              << "SequenceNPY::makeSequenceCountsIndex" 
              << " total " << total 
              << " cutoff " << cutoff 
              << std::endl ; 


    for(unsigned int i=0 ; i < idx->getNumItems() ; i++)
    {
         std::cout << std::setw(3) << i + 1 
                   << std::setw(20) << idx->getNameLocal(i+1)
                   << std::endl ; 
    }  


    return idx ; 
}



void SequenceNPY::dumpSequenceCounts(const char* msg, Types::Item_t etype, 
       std::map<std::string, unsigned int>& su,
       std::map<std::string, std::vector<unsigned int> >& sv,
       unsigned int cutoff
    )
{
    typedef std::map<std::string, unsigned int> MSU ;
    typedef std::pair<std::string, unsigned int> PSU ;

    std::vector<PSU> pairs ; 
    for(MSU::iterator it=su.begin() ; it != su.end() ; it++) pairs.push_back(*it);
    std::sort(pairs.begin(), pairs.end(), su_second_value_order );

    std::cout << msg << std::endl ; 

    unsigned int total(0);

    for(unsigned int i=0 ; i < pairs.size() ; i++)
    {
        PSU p = pairs[i];
        total += p.second ;  

        assert( sv[p.first].size() == p.second );

        if(p.second > cutoff)
            std::cout 
               << std::setw(5) << i          
               << " : "
               << std::setw(30) << p.first
               << " : " 
               << std::setw(10) << std::dec << p.second
               << std::setw(10) << std::dec << sv[p.first].size()
               << " : "
               << m_recs->decodeSequenceString(p.first, etype) 
               << std::endl ; 
    }

    std::cout 
              << " total " << total 
              << " cutoff " << cutoff 
              << std::endl ; 

}




