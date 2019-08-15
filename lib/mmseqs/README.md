# MMseqs2: ultra fast and sensitive protein search and clustering suite
MMseqs2 (Many-against-Many sequence searching) is a software suite to search and cluster huge protein and nucleotide sequence sets. MMseqs2 is open source GPL-licensed software implemented in C++ for Linux, MacOS, and (as beta version, via cygwin) Windows. The software is designed to run on multiple cores and servers and exhibits very good scalability. MMseqs2 can run 10000 times faster than BLAST. At 100 times its speed it achieves almost the same sensitivity. It can perform profile searches with the same sensitivity as PSI-BLAST at over 400 times its speed.

##  Publications

[Steinegger M and Soeding J. MMseqs2 enables sensitive protein sequence searching for the analysis of massive data sets. Nature Biotechnology, doi: 10.1038/nbt.3988 (2017)](https://www.nature.com/nbt/journal/vaop/ncurrent/full/nbt.3988.html).

[Steinegger M and Soeding J. Clustering huge protein sequence sets in linear time. Nature Communications, doi: 10.1038/s41467-018-04964-5 (2018)](https://www.nature.com/articles/s41467-018-04964-5).

[![BioConda Install](https://img.shields.io/conda/dn/bioconda/mmseqs2.svg?style=flag&label=BioConda%20install)](https://anaconda.org/bioconda/mmseqs2)
[![Github All Releases](https://img.shields.io/github/downloads/soedinglab/mmseqs2/total.svg)](https://github.com/soedinglab/mmseqs2/releases/latest)
[![Docker Pulls](https://img.shields.io/docker/pulls/soedinglab/mmseqs2.svg)](https://hub.docker.com/r/soedinglab/mmseqs2)
[![Build Status](https://dev.azure.com/themartinsteinegger/mmseqs2/_apis/build/status/soedinglab.MMseqs2?branchName=master)](https://dev.azure.com/themartinsteinegger/mmseqs2/_build/latest?definitionId=2&branchName=master)
[![Travis CI](https://travis-ci.org/soedinglab/MMseqs2.svg?branch=master)](https://travis-ci.org/soedinglab/MMseqs2)
[![Zenodo DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.840208.svg)](https://zenodo.org/record/1718312)

<p align="center"><img src="https://raw.githubusercontent.com/soedinglab/mmseqs2/master/.github/mmseqs2_logo.png" height="256" /></p>


## Documentation 
The MMseqs2 user guide is available in our [GitHub Wiki](https://github.com/soedinglab/mmseqs2/wiki) or as a [PDF file](https://mmseqs.com/latest/userguide.pdf) (Thanks to [pandoc](https://github.com/jgm/pandoc)!). We provide a tutorial of MMseqs2 [here](https://github.com/soedinglab/metaG-ECCB18-partII).

## News
Keep posted about MMseqs2/Linclust updates by following Martin on [Twitter](https://twitter.com/thesteinegger).

08/10/2018 ECCB18 tutorial of MMseqs2 is available [here](https://github.com/soedinglab/metaG-ECCB18-partII).

07/07/2018 Linclust has just been published at [Nature Communications](https://www.nature.com/articles/s41467-018-04964-5).

17/10/2017 MMseqs2 has just been published at [Nature Biotechnology](https://www.nature.com/nbt/journal/vaop/ncurrent/full/nbt.3988.html).

## Installation
MMseqs2 can be used by compiling from source, downloading a statically compiled version, using [Homebrew](https://github.com/Homebrew/brew), [conda](https://github.com/conda/conda) or [Docker](https://github.com/moby/moby). MMseqs2 requires a 64-bit system (check with `uname -a | grep x86_64`) with at least the SSE4.1 instruction set (check by executing `cat /proc/cpuinfo | grep sse4_1` on Linux or `sysctl -a | grep machdep.cpu.features | grep SSE4.1` on MacOS).
     
     # install by brew
     brew install mmseqs2
     # install via conda
     conda install -c bioconda mmseqs2 
     # install docker
     docker pull soedinglab/mmseqs2
     # static build with SSE4.1
     wget https://mmseqs.com/latest/mmseqs-linux-sse41.tar.gz; tar xvfz mmseqs-linux-sse41.tar.gz; export PATH=$(pwd)/mmseqs/bin/:$PATH
     # static build with AVX2
     wget https://mmseqs.com/latest/mmseqs-linux-avx2.tar.gz; tar xvfz mmseqs-linux-avx2.tar.gz; export PATH=$(pwd)/mmseqs/bin/:$PATH

The AVX2 version is faster than SSE4.1, check if AVX2 is supported by executing `cat /proc/cpuinfo | grep avx2` on Linux and `sysctl -a | grep machdep.cpu.leaf7_features | grep AVX2` on MacOS).
We also provide static binaries for MacOS and Windows at [mmseqs.com/latest](https://mmseqs.com/latest).

MMseqs2 comes with a bash command and parameter auto completion, which can be activated by adding the following lines to your $HOME/.bash_profile:

<pre>
        if [ -f /<b>Path to MMseqs2</b>/util/bash-completion.sh ]; then
            source /<b>Path to MMseqs2</b>/util/bash-completion.sh
        fi
</pre>
         
### Compilation from source
Compiling MMseqs2 from source has the advantage that it will be optimized to the specific system, which should improve its performance. To compile MMseqs2 `git`, `g++` (4.8 or later) and `cmake` (3.0 or later) are needed. Afterwards, the MMseqs2 binary will be located in the `build/bin/` directory.

        git clone https://github.com/soedinglab/MMseqs2.git
        cd MMseqs2
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=. ..
        make -j 4
        make install 
        export PATH=$(pwd)/bin/:$PATH

:exclamation: To compile MMseqs2 on MacOS, first install the `gcc` compiler from Homebrew. The default MacOS `clang` compiler does not support OpenMP and MMseqs2 will only be able to use a single thread. Then use the following `cmake` call:

        CC="$(brew --prefix)/bin/gcc-9" CXX="$(brew --prefix)/bin/g++-9" cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=. ..
                
        
## Easy workflows 
We provide `easy` workflows to search and cluster. The `easy-search` searches directly with a FASTA/FASTQ files against a either another FASTA/FASTQ file or an already existing MMseqs2 database.
        
        mmseqs createdb examples/DB.fasta targetDB
        mmseqs easy-search examples/QUERY.fasta targetDB alnRes tmp 
        
For clustering, MMseqs2 `easy-cluster` and `easy-linclust` are available.

`easy-cluster` by default clusters the entries of a FASTA/FASTQ file using a cascaded clustering algorithm.
        
        mmseqs easy-cluster examples/DB.fasta clusterRes tmp         
        
`easy-linclust` clusters the entries of a FASTA/FASTQ file. The runtime scales linearly with input size. This mode is recommended for huge datasets.
                
        mmseqs easy-linclust examples/DB.fasta clusterRes tmp     
        
These `easy` workflows are a shorthand to deal directly with FASTA/FASTQ files as input and output. MMseqs2 provides many modules to transform, filter, execute external programs and search. However, these modules use the MMseqs2 database formats, instead of the FASTA/FASTQ format. For maximum flexibility, we recommend using MMseqs2 workflows and modules directly.
       
## How to search
You can use the query database "QUERY.fasta" and target database "DB.fasta" in the examples folder to test the search workflow. First, you need to convert the FASTA files into the MMseqs2 database format.

        mmseqs createdb examples/QUERY.fasta queryDB
        mmseqs createdb examples/DB.fasta targetDB
        
If the target database is going to be used several times, we recommend precomputing an index of `targetDB` as this saves overhead computations.

        mmseqs createindex targetDB tmp

MMseqs2 stores intermediate results in `tmp`. Using a fast local drive can reduce load on a shared filesystem and increase speed.

To run the search, execute:

        mmseqs search queryDB targetDB resultDB tmp

The speed and sensitivity of the `search` can be adjusted with `-s` parameter and should be adapted based on your use case (see [setting sensitivity -s parameter](https://github.com/soedinglab/mmseqs2/wiki#set-sensitivity--s-parameter)). A very fast search would use a sensitivity of `-s 1.0`, while a very sensitive search would use a sensitivity of up to `-s 7.0`.

If you require the exact alignment information (Sequence identity, alignment string, ...) in later steps add the option `-a` parameter, without this parameter MMseqs2 will automatically decide if only the score needs to be computed or the exact alignment to optimize computation time.

Please ensure that, in case of large input databases, the `tmp` directory provides enough free space.
Our user guide provides information on [disk space requirements](https://github.com/soedinglab/mmseqs2/wiki#prefiltering-module).

Then convert the result database into a BLAST-tab formatted database (format: qId, tId, seqIdentity, alnLen, mismatchCnt, gapOpenCnt, qStart, qEnd, tStart, tEnd, eVal, bitScore).

        mmseqs convertalis queryDB targetDB resultDB resultDB.m8

The output can be customized with the `--format-output` option e.g. `--format-output "query,target,qaln,taln"` returns the query and target accession and the pairwise alignments in tab separated format. You can choose many different [output columns](https://github.com/soedinglab/mmseqs2/wiki#custom-alignment-format-with-convertalis) in the `convertalis` module. Make sure that you used the option `-a` during the search (`mmseqs search ... -a`).

        mmseqs convertalis queryDB targetDB resultDB resultDB.pair --format-output "query,target,qaln,taln"

### Other search modes

MMseqs2 provides many additional search modes:
 * Iterative sequences-profile searches (like PSI-BLAST) with the `--num-iterations` parameter
 * [Translated searches](https://github.com/soedinglab/MMseqs2/wiki#translated-sequence-searching) of nucleotides against proteins (blastx), proteins against nucleotides (tblastn) or nucleotide against nucleotide (tblastx)
 * [Iterative increasing sensitivity searches](https://github.com/soedinglab/MMseqs2/wiki#how-to-find-the-best-hit-the-fastest-way) to find only the best hits faster
 * [Taxonomic assignment](https://github.com/soedinglab/MMseqs2/wiki#taxonomy-assignment-using-mmseqs-taxonomy) using 2bLCA or LCA
 * Fast ungapped alignment searches to find [very similar sequence matches](https://github.com/soedinglab/MMseqs2/wiki#mapping-very-similar-sequences-using-mmseqs-map)
 * Very fast and sensitive Searches against [profile databases such as the PFAM](https://github.com/soedinglab/MMseqs2/wiki#how-to-create-a-target-profile-database-from-pfam)
 * [Reciprocal best hits search](https://github.com/soedinglab/MMseqs2/wiki#reciprocal-best-hit-using-mmseqs-rbh)
 * [Web search API and user interface](https://github.com/soedinglab/MMseqs2-App)


Many modes can also be combined. You can, for example, do a translated nucleotide against protein profile search.

## How to cluster 
Before clustering, convert your database into the MMseqs2 database format:

        mmseqs createdb examples/DB.fasta DB

Then execute the clustering:

        mmseqs cluster DB clu tmp
        
or linear time clustering (faster but less sensitive):

        mmseqs linclust DB clu tmp

Please adjust the [clustering criteria](https://github.com/soedinglab/MMseqs2/wiki#clustering-criteria) and check if temporary directory provides enough free space. For disk space requirements, see the user guide.

To generate a FASTA-style formatted output file from our MMseqs2 databases, type:

        mmseqs createseqfiledb DB clu clu_seq 
        mmseqs result2flat DB DB clu_seq clu_seq.fasta
        
To generate a TSV-style formatted output file from our MMseqs2 databases, type:

        mmseqs createtsv DB DB clu clu.tsv
        
To extract the representative sequences from the clustering result call:    
    
        mmseqs result2repseq DB clu DB_clu_rep
        mmseqs result2flat DB DB DB_clu_rep DB_clu_rep.fasta --use-fasta-header

Read more about the [clustering format](https://github.com/soedinglab/mmseqs2/wiki#clustering-format) in our user guide.

### Memory Requirements
MMseqs2 checks the available system memory and automatically divides the target database in parts that fit into memory. Splitting the database will increase the runtime slightly.

The memory consumption grows linearly with the number of residues in the database. The following formula can be used to estimate the index size.  
        
        M = (7 × N × L) byte + (8 × a^k) byte

Where `L` is the average sequence length and `N` is the database size.

### How to run MMseqs2 on multiple servers using MPI
MMseqs2 can run on multiple cores and servers using OpenMP and Message Passing Interface (MPI).
MPI assigns database splits to each compute node, which are then computed with multiple cores (OpenMP).

Make sure that MMseqs2 was compiled with MPI by using the `-DHAVE_MPI=1` flag (`cmake -DHAVE_MPI=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. ..`). Our precompiled static version of MMseqs2 cannot use MPI. The version string of MMseqs2 will have a `-MPI` suffix, if it was built successfully with MPI support.

To search with multiple servers, call the `search` or `cluster` workflow with the MPI command exported in the RUNNER environment variable. The databases and temporary folder have to be shared between all nodes (e.g. through NFS):

        RUNNER="mpirun -pernode -np 42" mmseqs search queryDB targetDB resultDB tmp
