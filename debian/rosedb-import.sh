#!/bin/bash

set -e

# Calls rosedb_tool to fill rosedb with data provided by Microsoft Security
# ./src/rosedb_tool <name> <rrtype> <ttl> "<rdata>" <threat_code> <syslog_ip>

# The input looks like this:
# Domain,TTL,RRType,RDATA,Tag,SYSLOGIP,SOATTL,SOAMNAME,NS1,NS2

if [ -z "${1}" ]; then
    echo "usage: ${0} dbdir < csv"
    exit 1
fi

DBDIR="$(dirname ${1})/$(basename ${1})"

echo "Importing into ${DBDIR}"

if [ -e "${DBDIR}" -a ! -d "${DBDIR}" ]; then
    echo "${DBDIR} exists and is not directory"
    echo "usage: ${0} dbdir < csv"
    exit 1
fi

TMPDBDIR=$(mktemp --directory --tmpdir=$(dirname "${DBDIR}") $(basename "${DBDIR}.XXXXXX"))

echo "Temporary directory is ${TMPDBDIR}"

TMPFILE=$(mktemp --tmpdir=$(dirname "${DBDIR}") rosedb-import.XXXXXX)

echo "Temporary file is ${TMPFILE}"

# Default SOA values
SERIAL=1
REFRESH=10800
UPDATE=15
EXPIRY=604800

while IFS=, read DOMAIN TTL RRTYPE RDATA TAG SYSLOGIP SOATTL SOAMNAME NS1 NS2; do

    # Skip CSV Header
    if [ "$DOMAIN" = "Domain" ]; then
	continue;
    fi

    DOMAIN="$(echo "${DOMAIN}" | tr "[A-Z]" "[a-z]")"

    # Append final dot
    if [ "${DOMAIN: -1}" != "." ]; then
	DOMAIN="${DOMAIN}."
    fi

    # Set SYSLOGIP to localhost if empty
    if [ -z "${SYSLOGIP}" ]; then
	SYSLOGIP=127.0.0.1
    fi

    # Create FQDNs for NS1
    unset FQDN1
    if [ -n "${NS1}" ]; then
	FQDN1="$(echo "${NS1}" | tr "[A-Z]" "[a-z]").microsoftinternetsafety.net."
    fi

    # Create FQDNs for NS2
    unset FQDN2
    if [ -n "${NS2}" ]; then
	FQDN2="$(echo "${NS2}" | tr "[A-Z]" "[a-z]").microsoftinternetsafety.net."
    fi

    # Create SOA record if SOATTL, FQDN1 and SOAMNAME is set
    if [ "${SOATTL}" -gt 0 ]; then
	if [ -z "${FQDN1}" ]; then
	    echo "ERROR: SOATTL set, but NS1 is empty";
	    exit 1;
	fi
	if [ -z "${SOAMNAME}" ]; then
	    echo "ERROR: SOATTL set, but SOAMNAME is empty";
	fi
	printf "add\t%s\t%s\t%s\t%s\t%s\t%s\n" "${DOMAIN}" "SOA" "${SOATTL}" "${FQDN1} ${SOAMNAME} ${SERIAL} ${REFRESH} ${UPDATE} ${EXPIRY} ${TTL}" "${TAG}" "${SYSLOGIP}"
    fi

    # Create first nameserver
    if [ -n "${FQDN1}" ]; then
	printf "add\t%s\t%s\t%s\t%s\t%s\t%s\n" "${DOMAIN}" "NS" "${TTL}" "${FQDN1}" "${TAG}" "${SYSLOGIP}"
    fi

    # Create second nameserver
    if [ -n "${FQDN2}" ]; then
	printf "add\t%s\t%s\t%s\t%s\t%s\t%s\n" "${DOMAIN}" "NS" "${TTL}" "${FQDN2}" "${TAG}" "${SYSLOGIP}"
    fi

    case "$RRTYPE" in
	MX) RDATA="0 ${RDATA}";;
	*) ;;
    esac

    # Create the real record
    printf "add\t%s\t%s\t%s\t%s\t%s\t%s\n" "${DOMAIN}" "${RRTYPE}" "${TTL}" "${RDATA}" "${TAG}" "${SYSLOGIP}"
done > "${TMPFILE}"

rosedb_tool "${TMPDBDIR}" import "${TMPFILE}"

#rm "${TMPFILE}"

# Make backup of old rosedb
[ -d "${DBDIR}" ] && mv -f "${DBDIR}" "${DBDIR}.$(date +%s)"

# Replace rosedb with fresh copy
mv -f "${TMPDBDIR}" "${DBDIR}"
