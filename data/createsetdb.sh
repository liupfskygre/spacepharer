#!/bin/sh -e
fail() {
    echo "Error: $1"
    exit 1
}

notExists() {
	[ ! -f "$1" ]
}

[ -z "$MMSEQS" ] && echo "Please set the environment variable \$MMSEQS to your MMSEQS binary." && exit 1;

if notExists "${OUTDB}"; then
    # shellcheck disable=SC2086
    "${MMSEQS}" createdb "$@" "${OUTDB}" ${CREATEDB_PAR} \
        || fail "createdb failed"
fi

if [ "$("${MMSEQS}" dbtype "${OUTDB}")" = "Nucleotide" ]; then
    mv -f "${OUTDB}" "${OUTDB}_nucl"
    mv -f "${OUTDB}.index" "${OUTDB}_nucl.index"
    mv -f "${OUTDB}.lookup" "${OUTDB}_nucl.lookup"
    mv -f "${OUTDB}.dbtype" "${OUTDB}_nucl.dbtype"

    mv -f "${OUTDB}_h" "${OUTDB}_nucl_h"
    mv -f "${OUTDB}_h.index" "${OUTDB}_nucl_h.index"
    mv -f "${OUTDB}_h.dbtype" "${OUTDB}_nucl_h.dbtype"

    if notExists "${OUTDB}_nucl_contig_to_set.index"; then
        awk '{ print $1"\t"$3; }' "${OUTDB}_nucl.lookup" | sort -k1,1n -k2,2n > "${OUTDB}_nucl_contig_to_set.tsv"
        "${MMSEQS}" tsv2db "${OUTDB}_nucl_contig_to_set.tsv" "${OUTDB}_nucl_contig_to_set" "--output-dbtype" "5"\
            || fail "tsv2db failed"
    fi

    if notExists "${OUTDB}_nucl_set_to_contig.index"; then
        awk '{ print $3"\t"$1; }' "${OUTDB}_nucl.lookup" | sort -k1,1n -k2,2n > "${OUTDB}_nucl_set_to_contig.tsv"
        "${MMSEQS}" tsv2db "${OUTDB}_nucl_set_to_contig.tsv" "${OUTDB}_nucl_set_to_contig" "--output-dbtype" "5"\
            || fail "tsv2db failed"
    fi

    if notExists "${OUTDB}_nucl_orf.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" extractorfs "${OUTDB}_nucl" "${OUTDB}_nucl_orf" ${EXTRACTORFS_PAR} \
            || fail "extractorfs failed"
    fi

    if notExists "${OUTDB}.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" translatenucs "${OUTDB}_nucl_orf" "${OUTDB}" ${TRANSLATENUCS_PAR} \
            || fail "translatenucs failed"
    fi

    # write extracted orfs locations on contig in alignment format
    if notExists "${OUTDB}_nucl_orf_aligned_to_contig.index"; then
        "${MMSEQS}" orftocontig "${OUTDB}_nucl" "${OUTDB}_nucl_orf" "${OUTDB}_nucl_orf_aligned_to_contig" \
            || fail "orftocontig step died"
    fi

    if notExists "${OUTDB}_nucl_orf_to_contig.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" filterdb "${OUTDB}_nucl_orf_aligned_to_contig" "${OUTDB}_nucl_orf_to_contig" --trim-to-one-column --filter-regex "^.*$" ${THREADS_PAR} \
            || fail "filterdb failed"
    fi

    if notExists "${OUTDB}_member_to_set.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" filterdb "${OUTDB}_nucl_orf_to_contig" "${OUTDB}_member_to_set" --mapping-file "${OUTDB}_nucl_contig_to_set.tsv" ${THREADS_PAR} \
            || fail "filterdb failed"
    fi

    if notExists "${OUTDB}_set_to_member.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" swapdb "${OUTDB}_member_to_set" "${OUTDB}_set_to_member" ${SWAPDB_PAR} \
            || fail "swapdb failed"
    fi

    if notExists "${OUTDB}_set_size.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" result2stats "${OUTDB}_nucl" "${OUTDB}_nucl" "${OUTDB}_set_to_member" "${OUTDB}_set_size" ${RESULT2STATS_PAR} \
            || fail "result2stats failed"
    fi

    if [ -n "$REVERSE_FRAGMENTS" ]; then
        # shellcheck disable=SC2086
        "${MMSEQS}" reverseseq "${OUTDB}" "${OUTDB}_reverse" ${THREADS_PAR} \
        || fail "reverseseq step died"
        mv -f "${OUTDB}_reverse" "${OUTDB}"
        mv -f "${OUTDB}_reverse.index" "${OUTDB}.index"
        mv -f "${OUTDB}_reverse.dbtype" "${OUTDB}.dbtype"
        #OUTDB="${OUTDB}_reverse"
        #echo "Will base search on ${OUTDB}"
    fi
else
    fail "protein mode not implemented"
fi

if [ -n "${REMOVE_TMP}" ]; then
    echo "Remove temporary files"
    rmdir "${TMP_PATH}/search"
    if [ -n "${NUCL}" ]; then
        rm -f "${TMP_PATH}/nucl_set.tsv"
    fi
    rm -f "${TMP_PATH}/createsetdb.sh"
fi


# #!/bin/bash -e

# # predict exons workflow script
# fail() {
#     echo "Error: $1"
#     exit 1
# }

# notExists() {
#     [ ! -f "$1" ]
# }

# abspath() {
#     if [ -d "$1" ]; then
#         (cd "$1"; pwd)
#     elif [ -f "$1" ]; then
#         if [[ "$1" == */* ]]; then
#             echo "$(cd "${1%/*}"; pwd)/${1##*/}"
#         else
#             echo "$(pwd)/$1"
#         fi
#     elif [ -d "$(dirname "$1")" ]; then
#             echo "$(cd "$(dirname "$1")"; pwd)/$(basename "$1")"
#     fi
# }

# # check number of input variables
# [ "$#" -ne 4 ] && echo "Please provide <contigsDB> <proteinsDB> <predictmatchBaseName> <tmpDir>" && exit 1;
# # check if files exist
# [ ! -f "$1.dbtype" ] && echo "$1.dbtype not found!" && exit 1;
# [ ! -f "$2.dbtype" ] && echo "$2.dbtype not found!" && exit 1;
# [ ! -d "$4" ] && echo "tmp directory $4 not found!" && mkdir -p "$4";

# INPUT_CONTIGS="$(abspath "$1")"
# INPUT_TARGET_PROTEINS="$(abspath "$2")"
# TMP_PATH="$(abspath "$4")"

# # extract coding fragments from input contigs (result in DNA)
# if notExists "${TMP_PATH}/nucl_6f.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" extractorfs "${INPUT_CONTIGS}" "${TMP_PATH}/nucl_6f" ${EXTRACTORFS_PAR} \
#         || fail "extractorfs step died"
# fi

# # write extracted orfs locations on contig in alignment format
# if notExists "${TMP_PATH}/nucl_6f_orf_aligned_to_contig.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" orftocontig "${INPUT_CONTIGS}" "${TMP_PATH}/nucl_6f" "${TMP_PATH}/nucl_6f_orf_aligned_to_contig" ${THREADS_PAR} \
#         || fail "orftocontig step died"
# fi

# # translate each coding fragment (result in AA)
# if notExists "${TMP_PATH}/aa_6f.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" translatenucs "${TMP_PATH}/nucl_6f" "${TMP_PATH}/aa_6f" ${TRANSLATENUCS_PAR} \
#         || fail "translatenucs step died"
# fi

# # when running in null mode (to assess evalues), we reverse the AA fragments:
# AA_FRAGS="${TMP_PATH}/aa_6f"
# if [ -n "$REVERSE_FRAGMENTS" ]; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" reverseseq "${AA_FRAGS}" "${AA_FRAGS}_reverse" ${THREADS_PAR} \
#         || fail "reverseseq step died"
#     AA_FRAGS="${AA_FRAGS}_reverse"
#     echo "Will base search on ${AA_FRAGS}"
# fi

# # search with each aa fragment against a target DB (result has queries as implicit keys)
# if notExists "${TMP_PATH}/search_res.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" search "${AA_FRAGS}" "${INPUT_TARGET_PROTEINS}" "${TMP_PATH}/search_res" "${TMP_PATH}/tmp_search" ${SEARCH_PAR} \
#         || fail "search step died"
# fi

# # swap results (result has targets as implicit keys)
# if notExists "${TMP_PATH}/search_res_swap.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" swapresults "${AA_FRAGS}" "${INPUT_TARGET_PROTEINS}" "${TMP_PATH}/search_res" "${TMP_PATH}/search_res_swap" ${SWAPRESULT_PAR} \
#         || fail "swap step died"
# fi

# # join contig information to swapped results (result has additional info about the origin of the AA fragments)
# if notExists "${TMP_PATH}/search_res_swap_w_contig_info.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" filterdb "${TMP_PATH}/search_res_swap" "${TMP_PATH}/search_res_swap_w_contig_info" --join-db "${TMP_PATH}/nucl_6f_orf_aligned_to_contig" --filter-column 1 ${THREADS_PAR} \
#         || fail "filterdb (to join contig info) step died"
# fi

# # sort joined swapped results by contig id
# if notExists "${TMP_PATH}/search_res_swap_w_contig_info_sorted.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" filterdb "${TMP_PATH}/search_res_swap_w_contig_info" "${TMP_PATH}/search_res_swap_w_contig_info_sorted" --sort-entries 1 --filter-column 11 ${THREADS_PAR} \
#         || fail "filterdb (to sort by contig) step died"
# fi

# # for each target, with respect to each contig and each strand, find the optimal set of exons
# if notExists "${TMP_PATH}/dp_protein_contig_strand_map.dbtype"; then
#     # shellcheck disable=SC2086
#     "$MMSEQS" collectoptimalset "${TMP_PATH}/search_res_swap_w_contig_info_sorted" "${INPUT_TARGET_PROTEINS}" "${TMP_PATH}/" ${COLLECTOPTIMALSET_PAR} \
#         || fail "collectoptimalset step died"
# fi

# # post processing
# mv -f "${TMP_PATH}/dp_protein_contig_strand_map" "$3_dp_protein_contig_strand_map" || fail "Could not move result to $3_dp_protein_contig_strand_map"
# mv -f "${TMP_PATH}/dp_protein_contig_strand_map.index" "$3_dp_protein_contig_strand_map.index" || fail "Could not move result to $3_dp_protein_contig_strand_map.index"
# mv -f "${TMP_PATH}/dp_protein_contig_strand_map.dbtype" "$3_dp_protein_contig_strand_map.dbtype" || fail "Could not move result to $3_dp_protein_contig_strand_map.dbtype"

# mv -f "${TMP_PATH}/dp_optimal_exon_sets" "$3_dp_optimal_exon_sets" || fail "Could not move result to $3_dp_optimal_exon_sets"
# mv -f "${TMP_PATH}/dp_optimal_exon_sets.index" "$3_dp_optimal_exon_sets.index" || fail "Could not move result to $3_dp_optimal_exon_sets.index"
# mv -f "${TMP_PATH}/dp_optimal_exon_sets.dbtype" "$3_dp_optimal_exon_sets.dbtype" || fail "Could not move result to $3_dp_optimal_exon_sets.dbtype"


# if [ -n "$REMOVE_TMP" ]; then
#     echo "Removing temporary files from ${TMP_PATH}"
#     rm -f "${TMP_PATH}"/nucl_6f*
#     rm -f "${TMP_PATH}"/aa_6f*
#     rm -f "${TMP_PATH}"/search_res*
#     rm -r "${TMP_PATH}"/tmp_search/
# fi

