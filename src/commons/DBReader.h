#ifndef DBREADER_H
#define DBREADER_H

// Written by Martin Steinegger & Maria Hauser mhauser@genzentrum.lmu.de
//
// Manages DB read access.
//

#include <cstddef>
#include <utility>
#include <vector>
#include <string>
#include "Sequence.h"
#include "Parameters.h"
#include "FileUtil.h"

#define ZSTD_STATIC_LINKING_ONLY // ZSTD_findDecompressedSize
#include <zstd.h>

template <typename T>
class DBReader {

public:
    struct Index {
        T id;
        size_t offset;
        static bool compareById(const Index& x, const Index& y){
            return (x.id <= y.id);
        }
        static bool compareByOffset(const Index& x, const Index& y){
            return (x.offset <= y.offset);
        }
    };


    struct LookupEntry {
        T id;
        std::string entryName;
        unsigned int fileNumber;
        LookupEntry(){}
        LookupEntry(T id) {
            this->id = id;
        }
        static bool compareById(const LookupEntry& x, const LookupEntry& y){
            return (x.id <= y.id);
        }
    };

    // = USE_DATA|USE_INDEX
    DBReader(const char* dataFileName, const char* indexFileName, int threads, int mode);

    DBReader(Index* index, unsigned int *seqLens, size_t size, size_t aaDbSize, T lastKey,
             int dbType, unsigned int maxSeqLen, int threads);

    void setDataFile(const char* dataFileName);

    virtual ~DBReader();

    bool open(int sort);

    void close();

    const char* getDataFileName() { return dataFileName; }

    const char* getIndexFileName() { return indexFileName; }

    size_t getAminoAcidDBSize();

    size_t getDataSize() { return dataSize; }

    char* getData(size_t id, int thrIdx);

    char* getDataCompressed(size_t id, int thrIdx);

    char* getDataUncompressed(size_t id);

    void touchData(size_t id);

    char* getDataByDBKey(T key, int thrIdx);

    char * getDataByOffset(size_t offset);

    size_t getSize();

    size_t getLookupSize();


    T getDbKey(size_t id);

    unsigned int * getSeqLens();

    size_t getSeqLens(size_t id);

    size_t maxCount(char c);

    void remapData();

    size_t bsearch(const Index * index, size_t size, T value);

    // does a binary search in the index and returns index of the entry with dbKey
    // returns UINT_MAX if the key is not contained in index
    size_t getId (T dbKey);

    // does a binary search in the lookup and returns index of the entry
    size_t getLookupId(T dbKey);
    std::string getLookupEntryName(size_t id);
    unsigned int getLookupFileNumber(size_t id);

    static const int NOSORT = 0;
    static const int SORT_BY_LENGTH = 1;
    static const int LINEAR_ACCCESS = 2;
    static const int SORT_BY_ID     = 3;
    static const int SORT_BY_LINE   = 4; // the local IDs correspond to the line number in the original index file
    static const int SHUFFLE        = 5;
    static const int HARDNOSORT = 6; // do not even sort by ids.
    static const int SORT_BY_ID_OFFSET = 7;
    static const int SORT_BY_OFFSET = 8; // only offset sorting saves memory and does not support random access


    static const unsigned int USE_INDEX    = 0;
    static const unsigned int USE_DATA     = 1;
    static const unsigned int USE_WRITABLE = 2;
    static const unsigned int USE_FREAD    = 4;
    static const unsigned int USE_LOOKUP   = 8;


    // compressed
    static const int UNCOMPRESSED    = 0;
    static const int COMPRESSED     = 1;

    char * getDataForFile(size_t fileIdx){
        return dataFiles[fileIdx];
    }

    size_t getDataFileCnt(){
        return dataFileCnt;
    }

    size_t getDataSizeForFile(size_t fileIdx){
        return dataSizeOffset[fileIdx+1]-dataSizeOffset[fileIdx];
    }

    std::vector<std::string> getDataFileNames(){
        return dataFileNames;
    }

    size_t getTotalDataSize(){
        return totalDataSize;
    }

    static void moveDatafiles(const std::vector<std::string>& files, const std::string& destination);

    static void moveDb(const std::string &srcDbName, const std::string &dstDbName);

    static void removeDb(const std::string &databaseName);

    char *mmapData(FILE *file, size_t *dataSize);

    bool readIndex(char *data, size_t indexDataSize, Index *index, unsigned int *entryLength, size_t & dataSize);

    void readLookup(char *data, size_t dataSize, LookupEntry *lookup);

    void readIndexId(T* id, char * line, const char** cols);

    void readMmapedDataInMemory();

    void mlock();

    void sortIndex(bool isSortedById);
    bool isSortedByOffset();

    void unmapData();

    size_t getDataOffset(T i);


    Index* getIndex() {
        return index;
    }

    Index* getIndex(size_t id) {
        return index + local2id[id];
    }


    void printMagicNumber();

    T getLastKey();

    static size_t indexMemorySize(const DBReader<unsigned int> &idx);

    static char* serialize(const DBReader<unsigned int> &idx);

    static DBReader<unsigned int> *unserialize(const char* data, int threads);

    int getDbtype(){
        return dbtype;
    }

    const char* getDbTypeName() const {
        return Parameters::getDbTypeName(dbtype);
    }


    struct sortIndecesById {
        sortIndecesById(const Index * ind) : _ind(ind) {}
        bool operator() (unsigned int i, unsigned int j) const { 
            return (_ind[i].id < _ind[j].id); 
        }
        const Index * _ind;
    };

    struct compareIndexLengthPairById {
        bool operator() (const std::pair<Index, unsigned  int>& lhs, const std::pair<Index, unsigned  int>& rhs) const{
            return (lhs.first.id < rhs.first.id);
        }
    };

    struct compareIndexLengthPairByIdKeepTrack {
        bool operator() (const std::pair<Index, std::pair<size_t, unsigned int> >& lhs, const std::pair<Index, std::pair<size_t, unsigned int> >& rhs) const{
            return (lhs.first.id < rhs.first.id);
        }
    };

    struct comparePairBySeqLength {
        bool operator() (const std::pair<unsigned int, unsigned  int>& lhs, const std::pair<unsigned int, unsigned  int>& rhs) const{
            if(lhs.second > rhs.second)
                return true;
            if(rhs.second > lhs.second)
                return false;
            if(lhs.first < rhs.first  )
                return true;
            if(rhs.first < lhs.first )
                return false;
            return false;
        }
    };

    struct comparePairByIdAndOffset {
        bool operator() (const std::pair<unsigned int, Index>& lhs, const std::pair<unsigned int, Index>& rhs) const{
            if(lhs.second.id < rhs.second.id)
                return true;
            if(rhs.second.id < lhs.second.id)
                return false;
            if(lhs.second.offset < rhs.second.offset  )
                return true;
            if(rhs.second.offset < lhs.second.offset )
                return false;
            return false;
        }
    };


    struct comparePairByOffset{
        bool operator() (const std::pair<unsigned int, size_t >& lhs, const std::pair<unsigned int, size_t >& rhs) const{
            return (lhs.second < rhs.second);
        }
    };

    void setData(char *data, size_t dataSize);

    void setMode(const int mode);

    size_t getOffset(size_t id);

    size_t findNextOffsetid(size_t id);

    int isCompressed(){
        return isCompressed(dbtype);
    }

    static int isCompressed(int dbtype);

    void setSequentialAdvice();

    static void softLink(DBReader<unsigned int> &reader, std::string &outDb);

private:

    void checkClosed();


    int threads;

    int dataMode;

    char* dataFileName;
    char* indexFileName;

    // number of entries in the index
    size_t size;

    // offset for all datafiles
    char** dataFiles;
    size_t * dataSizeOffset;
    size_t dataFileCnt;
    size_t totalDataSize;
    std::vector<std::string> dataFileNames;


    // summed up size of all entries
    size_t dataSize;
    // Last Key in Index
    T lastKey;
    // max seqLen
    unsigned int maxSeqLen;
    // flag to check if db was closed
    int closed;
    // stores the dbtype (if dbtype file exists)
    int dbtype;
    int compression;
    char ** compressedBuffers;
    size_t * compressedBufferSizes;
    ZSTD_DStream ** dstream;

    Index * index;
    size_t lookupSize;
    LookupEntry * lookup;
    bool sortedByOffset;

    unsigned int * seqLens;
    unsigned int * id2local;
    unsigned int * local2id;

    bool dataMapped;
    int accessType;

    bool externalData;

    bool didMlock;

    // needed to prevent the compiler from optimizing away the loop
    char magicBytes;

};

#endif
