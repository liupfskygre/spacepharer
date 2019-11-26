#include "LocalParameters.h"
#include "FileUtil.h"
#include "DBReader.h"
#include "DBWriter.h"
#include "Debug.h"
#include "Util.h"
#include "MemoryMapped.h"

#include <climits>

enum Format {
    PILERCR,
    CRT,
    CRISPRFINDER,
    CRISPRDETECT
};

const char *Format_names[] = { "PILERCR", "CRT", "CRISPRFINDER" ,"CRISPRDETECT" };

Format detectFileType(char* data){
    std::string line;
    //check first 5 char for format type
    for( size_t i = 0; i < 5; i++){
        line.push_back(data[i]);
    }
    if(line == "piler"){
        return PILERCR;
    }
    else if(line == "ORGAN"){
        return CRT;
    }
    else if(line == "#####"){
        return CRISPRFINDER;
    } 
    else if(line == "Array"){
        return CRISPRDETECT;
    }
    else{
        Debug(Debug::ERROR) << "Undetectable file type!\n";
        EXIT(EXIT_FAILURE);
    }
}

std::string getCurrentLine(char* data, size_t pos = 0){
    std::string line;
    while(data[pos] != '\n' && data[pos] != '\0'){
        line.push_back(data[pos]);
        pos++;
    }
    return line;
}

bool isNucl(std::string sequence){
    for(size_t i = 0; i < sequence.size(); i++){
        if(sequence[i] != 'A'&& sequence[i] != 'T' && sequence[i] != 'C' && sequence[i] != 'G'){
            return false;
        }
    }
    return true;
}

int parsespacer(int argc, const char **argv, const Command& command) {
    LocalParameters& par = LocalParameters::getLocalInstance();
    par.parseParameters(argc, argv, command, true, Parameters::PARSE_VARIADIC, 0);

    std::string outDb = par.filenames.back();
    par.filenames.pop_back();

    DBWriter writer(outDb.c_str(), (outDb + ".index").c_str(), 1, par.compressed, Parameters::DBTYPE_NUCLEOTIDES);
    writer.open();

    DBWriter headers((outDb + "_h").c_str(), (outDb + "_h.index").c_str(), 1, par.compressed, Parameters::DBTYPE_GENERIC_DB);
    headers.open();

    FILE* lookup = FileUtil::openAndDelete((outDb + ".lookup").c_str(), "w");
    FILE* source = FileUtil::openAndDelete((outDb + ".source").c_str(), "w");

    size_t key = 0;
    for (size_t i = 0; i < par.filenames.size(); ++i) {
        std::string& file = par.filenames[i];
        MemoryMapped input(file, MemoryMapped::WholeFile, MemoryMapped::SequentialScan);
        char* data = (char *) input.getData();

        std::string sequence;
        sequence.reserve(par.maxSeqLen);

        std::string arrayHeader;
        std::string header;
        header.reserve(10000);        
        std::string accession;

        unsigned int spacerStartPos;
        unsigned int spacerEndPos;
        size_t minSpacerLen = 20;
        //char dbKey[256];

        Format type;

        size_t entry = 0;
        size_t arrayEntry;
        size_t headerEntry;
        size_t arrayNum = 0;
        size_t spacerNum = 0;
        bool isArrayReverse;
        while (*data != '\0') {
            // pointer to the beginning of any line

            // if (entry == 0) {
            //     type = detectFileType(data);
            //     if (type != CRT||CRISPRDetect) {
            //         continue;
            //     }
            // }
            if (entry == 0) {
                type = detectFileType(data);
                Debug(Debug::INFO) << "Detected input file "<< file << " is of type " << Format_names[type] << "\n";
                if(type == CRT){
                    arrayHeader = getCurrentLine(data, 11);
                    accession = Util::parseFastaHeader(arrayHeader);
                }                
            }

            switch (type){
                case PILERCR:
                    if (*data == 'A'){
                        arrayEntry = entry;
                        arrayNum++;
                    }                
                    if (*data == '>' && (entry == arrayEntry + 1)){
                        arrayHeader = getCurrentLine(data, 1);
                        accession = Util::parseFastaHeader(arrayHeader);
                        headerEntry = entry;
                    }
                    if(*data == ' ' && (entry == headerEntry + 4)){   
                        while (*data != '='){
                            std::string line = getCurrentLine(data);
                            std::vector<std::string> values = Util::split(line, " ");
                            sequence = Util::removeWhiteSpace(sequence = values.back().c_str());
                            if(sequence.size() >= minSpacerLen && isNucl(sequence)){                            
                                key++;
                                spacerNum++;
                                //position + repeat length                            
                                spacerStartPos = (unsigned int) strtoul(values[0].c_str(), NULL, 10) + (unsigned int) strtoul(values[1].c_str(), NULL, 10);
                                spacerEndPos = spacerStartPos + sequence.size();
                                //"accession_Array_#_spacer_#_spacerStartPos_spacerEndPos_spacerLen"
                                header.append(accession.c_str() + std::string("_Array_") + std::to_string(arrayNum) + \
                                    std::string("_spacer_") + std::to_string(spacerNum) + std::string("_") + \
                                    std::to_string(spacerStartPos) + std::string("_") +  std::to_string(spacerEndPos) + std::string("_") + \
                                    std::to_string(sequence.size()));
                                fprintf(lookup, "%zu\t%s\t%zu\n", key, header.c_str(), i);          
                                sequence += std::string("\n");
                                header += std::string("\n");                      
                                writer.writeData(sequence.c_str(), sequence.size(), key, 0);
                                headers.writeData(header.c_str(), header.size(), key, 0);
                            }
                            data = Util::skipLine(data); 
                            entry++;                            
                            sequence.clear();
                            header.clear();
                        }
                    }
                    data = Util::skipLine(data);
                    entry++;
                    break;

                case CRT:
                    if (*data == 'C'){
                        arrayEntry = entry;
                        arrayNum++;
                    }                

                    if(entry == arrayEntry + 3){   
                        while (*data != '-'){
                            std::string line = getCurrentLine(data);
                            std::vector<std::string> values = Util::split(line, "\t");
                            sequence = Util::removeWhiteSpace(values[2].c_str());
                            if(sequence.size() >= minSpacerLen && isNucl(sequence)){
                                key++;
                                spacerNum++;      
                                spacerStartPos = (unsigned int) strtoul(values[0].c_str(), NULL, 10) + values[1].size();
                                spacerEndPos = spacerStartPos + sequence.size();
                                //"accession_Array_#_spacer_#_spacerStartPos_spacerEndPos_spacerLen"
                                header.append(accession.c_str() + std::string("_Array_") + std::to_string(arrayNum) + \
                                    std::string("_spacer_") + std::to_string(spacerNum) + std::string("_") + \
                                    std::to_string(spacerStartPos) + std::string("_") +  std::to_string(spacerEndPos) + std::string("_") + \
                                    std::to_string(sequence.size()));   
                                fprintf(lookup, "%zu\t%s\t%zu\n", key, header.c_str(), i);                                   
                                sequence += std::string("\n");
                                header += std::string("\n");                     
                                writer.writeData(sequence.c_str(), sequence.size(), key, 0);
                                headers.writeData(header.c_str(), header.size(), key, 0);                       
                            }
                            data = Util::skipLine(data); 
                            entry++;                                  
                            sequence.clear();
                            header.clear();  
                        }
                    }
                    data = Util::skipLine(data);
                    entry++;
                    break;
                
                case CRISPRDETECT:
                    if (*data == 'A'){
                        arrayEntry = entry;
                        arrayNum++;
                    }                
                    if (*data == '>' && (entry == arrayEntry + 1)){
                        arrayHeader = getCurrentLine(data, 1);
                        std::vector<std::string> headerValues = Util::split(arrayHeader, "\t");
                        isArrayReverse = ((headerValues[1].find("Reverse") != std::string::npos) ? true:false);
                        accession = Util::parseFastaHeader(arrayHeader);
                        headerEntry = entry;
                    }
                    if(*data == ' ' && (entry == headerEntry + 4)){   
                        while (*data != '='){
                            std::string line = getCurrentLine(data);
                            std::vector<std::string> values = Util::split(line, "\t");
                            sequence = Util::removeWhiteSpace(values[5].c_str());
                            if(sequence.size() >= minSpacerLen && isNucl(sequence)){
                                key++;
                                spacerNum++; 
                                if(isArrayReverse){
                                    spacerStartPos = (unsigned int) strtoul(values[0].c_str(), NULL, 10) - (unsigned int) strtoul(values[1].c_str(), NULL, 10);
                                    spacerEndPos = spacerStartPos - sequence.size();
                                } 
                                else {
                                    spacerStartPos = (unsigned int) strtoul(values[0].c_str(), NULL, 10) + (unsigned int) strtoul(values[1].c_str(), NULL, 10);
                                    spacerEndPos = spacerStartPos + sequence.size();
                                }
                                //"accession_Array_#_spacer_#_spacerStartPos_spacerEndPos_spacerLen"
                                header.append(accession.c_str() + std::string("_Array_") + std::to_string(arrayNum) + \
                                    std::string("_spacer_") + std::to_string(spacerNum) + std::string("_") + \
                                    std::to_string(spacerStartPos) + std::string("_") +  std::to_string(spacerEndPos) + std::string("_") + \
                                    std::to_string(sequence.size()));                                
                                fprintf(lookup, "%zu\t%s\t%zu\n", key, header.c_str(), i);   
                                sequence += std::string("\n");
                                header += std::string("\n");                           
                                writer.writeData(sequence.c_str(), sequence.size(), key, 0);                            
                                headers.writeData(header.c_str(), header.size(), key, 0);

                       
                            }
                            data = Util::skipLine(data); 
                            entry++;                                  
                            sequence.clear();
                            header.clear();  
                        }
                    }
                    data = Util::skipLine(data);
                    entry++;
                    break;
                    
                case CRISPRFINDER:
                //TODO: crisprfinder has 1 file for each array...
                break;
            }


        }
        input.close();
        fprintf(source, "%zu\t%s\n", i, file.c_str());
    }
    fclose(lookup);
    fclose(source);
    headers.close(true);
    writer.close(true);

    return EXIT_SUCCESS;
}