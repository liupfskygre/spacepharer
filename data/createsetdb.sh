#!/bin/sh -e
fail() {
    echo "Error: $1"
    exit 1
}

notExists() {
	[ ! -f "$1" ]
}

hasCommand () {
    command -v "$1" >/dev/null 2>&1 || { echo "Please make sure that $1 is in \$PATH."; exit 1; }
}

hasCommand awk
hasCommand sort

[ -z "$MMSEQS" ] && echo "Please set the environment variable \$MMSEQS to your MMSEQS binary." && exit 1;

#check if already created db
if notExists "${OUTDB}.dbtype"; then
    if notExists "${1}.dbtype"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" createdb "$@" "${OUTDB}" ${CREATEDB_PAR} \
            || fail "createdb failed"
    else
        cp "$1" "${OUTDB}"
        cp "$1.index" "${OUTDB}.index"
        cp "$1.lookup" "${OUTDB}.lookup"
        cp "$1.source" "${OUTDB}.source"
        cp "$1.dbtype" "${OUTDB}.dbtype"

        cp "$1_h" "${OUTDB}_h"
        cp "$1_h.index" "${OUTDB}_h.index"
        cp "$1_h.dbtype" "${OUTDB}_h.dbtype"
    fi
fi


# if notExists "${OUTDB}"; then
#     # shellcheck disable=SC2086
#     "${MMSEQS}" createdb "$@" "${OUTDB}" ${CREATEDB_PAR} \
#         || fail "createdb failed"
# fi


if [ "$("${MMSEQS}" dbtype "${OUTDB}")" = "Nucleotide" ]; then
    mv -f "${OUTDB}" "${OUTDB}_nucl"
    mv -f "${OUTDB}.index" "${OUTDB}_nucl.index"
    mv -f "${OUTDB}.lookup" "${OUTDB}_nucl.lookup"
    mv -f "${OUTDB}.source" "${OUTDB}_nucl.source"
    mv -f "${OUTDB}.dbtype" "${OUTDB}_nucl.dbtype"

    mv -f "${OUTDB}_h" "${OUTDB}_nucl_h"
    mv -f "${OUTDB}_h.index" "${OUTDB}_nucl_h.index"
    mv -f "${OUTDB}_h.dbtype" "${OUTDB}_nucl_h.dbtype"

    if notExists "${OUTDB}_nucl_contig_to_set.index"; then
        awk '{ print $1"\t"$3; }' "${OUTDB}_nucl.lookup" | sort -k1,1n -k2,2n > "${OUTDB}_nucl_contig_to_set.tsv"
        # shellcheck disable=SC2086
        "${MMSEQS}" tsv2db "${OUTDB}_nucl_contig_to_set.tsv" "${OUTDB}_nucl_contig_to_set" --output-dbtype 5 ${VERBOSITY_PAR} \
            || fail "tsv2db failed"
    fi

    if notExists "${OUTDB}_nucl_set_to_contig.index"; then
        awk '{ print $3"\t"$1; }' "${OUTDB}_nucl.lookup" | sort -k1,1n -k2,2n > "${OUTDB}_nucl_set_to_contig.tsv"
        # shellcheck disable=SC2086
        "${MMSEQS}" tsv2db "${OUTDB}_nucl_set_to_contig.tsv" "${OUTDB}_nucl_set_to_contig" --output-dbtype 5 ${VERBOSITY_PAR} \
            || fail "tsv2db failed"
    fi

    if notExists "${OUTDB}_nucl_orf.index"; then
        if [ -n "${EXTRACTORFS_SPACER}" ]; then
            # shellcheck disable=SC2086
            "${MMSEQS}" extractorfs "${OUTDB}_nucl" "${OUTDB}_nucl_orf" --min-length 9 --orf-start-mode 1 ${THREADS_PAR} \
                || fail "extractorfs failed"     
        else   
            # shellcheck disable=SC2086
            "${MMSEQS}" extractorfs "${OUTDB}_nucl" "${OUTDB}_nucl_orf" --orf-start-mode 0 ${THREADS_PAR} \
                || fail "extractorfs failed"
        fi
    fi

    if [ -n "${REVERSE_FRAGMENTS}" ]; then
        # shellcheck disable=SC2086
        "${MMSEQS}" reverseseqbycodon "${OUTDB}_nucl_orf" "${OUTDB}_nucl_orf_reverse" ${THREADS_PAR} \
            || fail "reverseseqbycodon died"
        # TODO: check mvdb instead
        mv -f "${OUTDB}_nucl_orf_reverse" "${OUTDB}_nucl_orf"
        mv -f "${OUTDB}_nucl_orf_reverse.index" "${OUTDB}_nucl_orf.index"
        mv -f "${OUTDB}_nucl_orf_reverse.dbtype" "${OUTDB}_nucl_orf.dbtype"
    fi

    if notExists "${OUTDB}.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" translatenucs "${OUTDB}_nucl_orf" "${OUTDB}" ${TRANSLATENUCS_PAR} \
            || fail "translatenucs failed"
    fi

    # write extracted orfs locations on contig in alignment format
    if notExists "${OUTDB}_nucl_orf_aligned_to_contig.index"; then
        # shellcheck disable=SC2086
        "${MMSEQS}" orftocontig "${OUTDB}_nucl" "${OUTDB}_nucl_orf" "${OUTDB}_nucl_orf_aligned_to_contig" ${THREADS_PAR} \
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
        "${MMSEQS}" result2stats "${OUTDB}_nucl" "${OUTDB}_nucl" "${OUTDB}_set_to_member" "${OUTDB}_set_size" --stat linecount ${THREADS_PAR} \
            || fail "result2stats failed"
    fi

else
    fail "protein mode not implemented"
fi

if [ -n "${REMOVE_TMP}" ]; then
    echo "Remove temporary files"
    rm -f "${TMP_PATH}/createsetdb.sh"
fi

