#include "Util.h"
#include "CommandCaller.h"
#include "Debug.h"
#include "FileUtil.h"
#include "DBReader.h"
#include "LocalParameters.h"
#include "createsetdb.sh.h"

void setcreatesetdbDefaults(Parameters *p) {
    p->orfMinLength = 30;  
}

int createsetdb(int argc, const char **argv, const Command& command) {
    LocalParameters& par = LocalParameters::getLocalInstance();
    setcreatesetdbDefaults(&par);
    par.parseParameters(argc, argv, command, true, 0, 0);

    // check if temp dir exists and if not, try to create it:
    std::string tmpDir = par.filenames.back();
    par.filenames.pop_back();

    if (FileUtil::directoryExists(tmpDir.c_str()) == false) {
        Debug(Debug::INFO) << "Tmp " << tmpDir << " folder does not exist or is not a directory.\n";
        if (FileUtil::makeDir(tmpDir.c_str()) == false) {
            Debug(Debug::ERROR) << "Can not create tmp folder " << tmpDir << ".\n";
            return EXIT_FAILURE;
        } else {
            Debug(Debug::INFO) << "Created dir " << tmpDir << "\n";
        }
    }

    std::string hash = SSTR(par.hashParameter(par.filenames, par.createsetdbworkflow));
    if(par.reuseLatest){
        hash = FileUtil::getHashFromSymLink(tmpDir+"/latest");
    }
    tmpDir += "/" + hash;
    if (FileUtil::directoryExists(tmpDir.c_str()) == false) {
        if (FileUtil::makeDir(tmpDir.c_str()) == false) {
            Debug(Debug::ERROR) << "Can not create sub tmp folder " << tmpDir << ".\n";
            return EXIT_FAILURE;
        }
    }

    FileUtil::symlinkAlias(tmpDir, "latest");

    std::string outDb = par.filenames.back();
    par.filenames.pop_back();


    CommandCaller cmd;
    cmd.addVariable("OUTDB", outDb.c_str());
    cmd.addVariable("TMP_PATH", tmpDir.c_str());
    cmd.addVariable("REMOVE_TMP", par.removeTmpFiles ? "TRUE" : NULL);
    cmd.addVariable("REVERSE_FRAGMENTS", par.reverseFragments == 1 ? "TRUE" : NULL);
    cmd.addVariable("EXTRACTORFS_SPACER", par.extractorfsSpacer == 1 ? "TRUE" : NULL);
    cmd.addVariable("CREATEDB_PAR", par.createParameterString(par.createdb).c_str());
    cmd.addVariable("EXTRACTORFS_PAR", par.createParameterString(par.extractorfs).c_str());
    cmd.addVariable("TRANSLATENUCS_PAR", par.createParameterString(par.translatenucs).c_str());
    cmd.addVariable("SWAPDB_PAR", par.createParameterString(par.swapdb).c_str());
    par.stat = "linecount";
    cmd.addVariable("RESULT2STATS_PAR", par.createParameterString(par.result2stats).c_str());
    cmd.addVariable("THREADS_PAR", par.createParameterString(par.onlythreads).c_str());


    FileUtil::writeFile(tmpDir + "/createsetdb.sh", createsetdb_sh, createsetdb_sh_len);
    std::string program(tmpDir + "/createsetdb.sh");
    cmd.execProgram(program.c_str(), par.filenames);

    return EXIT_SUCCESS;
}
