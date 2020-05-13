#include "Command.h"
#include "LocalCommandDeclarations.h"
#include "LocalParameters.h"

const char* binary_name = "spacepharer";
const char* tool_name = "SpacePHARER";
// TODO Write one full sentence
const char* tool_introduction = "CRISPR spacer phage-host finder";
const char* main_author = "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>";
const char* show_extended_help = "1";
const char* show_bash_info = NULL;
bool hide_base_commands = true;

LocalParameters& localPar = LocalParameters::getLocalInstance();
std::vector<Command> commands = {
        {"easy-predict",             easypredict,           &localPar.predictmatchworkflow,              COMMAND_EASY,
                "Predict phage-host matches from common spacer files (PILER-CR, CRISPRDetect and CRT)",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:spacerFile1[.txt]> ... <i:spacerFileN[.txt]> <i:targetDB> <o:output[.tsv]> <tmpDir>",
                CITATION_SPACEPHARER, {{"spacerFile", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA | DbType::VARIADIC, &DbValidator::flatfile},
                                       {"targetDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"result", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::flatfile},
                                       {"tmpDir", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::directory}}},
        {"parsespacer",             parsespacer,            &localPar.threadsandcompression,    COMMAND_MAIN,
                "Parse spacer files (PILER-CR, CRISPRDetect and CRT) and create sequence database",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:spacerFile1[.txt]> ... <i:spacerFileN[.txt]> <o:spacerDB>",
                CITATION_SPACEPHARER, {{"spacerFile", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA | DbType::VARIADIC, &DbValidator::flatfile},
                                       {"spacerDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::sequenceDb}}},
        {"downloadgenome",          downloadgenome,         &localPar.downloadgenome,           COMMAND_MAIN,
                "Download GenBank phage genomes and create sequence database",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<name/downloadFile> <o:sequenceDB> <tmpDir>",
                CITATION_SPACEPHARER, {{"name", 0, DbType::ZERO_OR_ALL, &DbValidator::empty },
                                       {"sequenceDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::sequenceDb },
                                       {"tmpDir",     DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::directory }}},
       {"createsetdb",              createsetdb,            &localPar.createsetdbworkflow,      COMMAND_MAIN,
                "Create sequence database from FASTA input",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:fastaFile1[.gz]> ... <i:fastaFileN[.gz]>|<i:DB> <o:setDB> <tmpDir>",
                CITATION_SPACEPHARER, {{"fast[a|q]File[.gz|bz]", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA | DbType::VARIADIC,  &DbValidator::flatfile},
                                       {"setDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::flatfile},
                                       {"tmpDir", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::directory}}},
        {"predictmatch",            predictmatch,           &localPar.predictmatchworkflow,     COMMAND_MAIN,
                "Predict phage-host matches",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:queryDB> <i:targetDB> <i:controltargetDB> <o:output[.tsv]> <tmpDir>",
                CITATION_SPACEPHARER, {{"queryDB",  DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"targetDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"controlTargetDB",  DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"cScoreDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::flatfile},
                                       {"tmpDir", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::directory}}},
                                         
       {"filtermatchbyfdr",         filtermatchbyfdr,       &localPar.filtermatchbyfdr,         COMMAND_SPECIAL | COMMAND_EXPERT,
                "Report matches based on false discovey rate",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:cScoreDB> <i:controlcScoreDB> <o:resultDB>",
                CITATION_SPACEPHARER, {{"cScoreDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::resultDb},
                                       {"controlcScoreDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::resultDb},
                                       {"resultDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::genericDb}}},
       {"findpam",                  findpam,                &localPar.threadsandcompression,    COMMAND_SPECIAL | COMMAND_EXPERT,
                "Report PAM upstream or downstream of protospacer",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:targetDB> <i:alnResultDB>  <o:resultDB>",
                CITATION_SPACEPHARER, {{"targetDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"alnResultDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::genericDb},
                                       {"resultDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::genericDb}}},
       {"truncatebesthits",         truncatebesthits,       &localPar.threadsandcompression,    COMMAND_SPECIAL | COMMAND_EXPERT,
                "Truncate list of best hits based on query set size",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:queryDB> <i:besthitsDB> <o:resultDB>",
                CITATION_SPACEPHARER, {{"queryDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
                                       {"besthitsDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::resultDb},
                                       {"resultDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::resultDb}}},
       {"summarizeresults",         summarizeresults,       &localPar.summarizeresults,         COMMAND_SPECIAL | COMMAND_EXPERT,
                "Summarize results on predicted matches (E-value) and hits (alignments and PAM)",
                NULL,
                "Ruoshi Zhang <ruoshi.zhang@mpibpc.mpg.de>",
                "<i:matchDB> <i:alnDB> <o:output[.tsv]>",
                CITATION_SPACEPHARER, {{"matchDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::genericDb},
                                       {"alnDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, NULL}, //&DbValidator::resultDb},
                                       {"output", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::flatfile}}}
};

