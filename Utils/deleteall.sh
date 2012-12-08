#!/bin/bash
# Auxiliary script to delete all the jobs 
# found in the cluster (but the interactive ones)
#
# J. Duarte Campderros (IFCA) Dec. 4th 2012

# Help function
help() 
{
	
	cat <<EOF

Delete jobs from the cluster

SYNTAX:

   $0 parent_directory

   The script will delete all the jobs related with the
   'cluster_dataname' folders found in 'parent_directory' folder


EOF
}

while getopts h o;
	do
		case "$o" in
			h)	help;
				exit 1;;
		esac
	done

PD=$1

CONTENTPD=`find $PD -type d -name cluster_*`

for i in $CONTENTPD;
do
	sendcluster delete -w $i;
done;


