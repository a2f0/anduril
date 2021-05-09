#!/bin/sh
#
# $Id: append.sh,v 1.2 2004/01/21 07:11:13 wd Exp $
#
# usage: log_append.sh <into> <file> [file2 ...]
# the script must be run in an 'empty' staging area.  it creates a tmp
# directory and extracts each file individually into that, then moves two
# levels in and starts copying in.  when it is done it removes the tmp area
# and the files on the command line.

CWD=`pwd`
if test -z "$2" ; then
    echo "usage: $0 <into> <file> [file2 ...]"
    exit
fi

ldir=$1
shift
if ! test -d "$ldir" ; then
    echo "$ldir is not a directory"
    exit
fi
cd $ldir
ldir=`pwd`
cd $CWD

mkdir -p tmp
for file in $* ; do
    ext=`expr $file : '.*\.\(tar.*\)'`
    case $ext in
	tar)
	    flg="xf"
	;;
	tar.gz)
	    flg="zxf"
	;;
	tar.bz2)
	    flg="yxf"
	;;
	*)
	    flg=""
	    echo "unknown extension $ext"
	    continue
	;;
    esac
    echo -n "appending logs from $file ..."
    count=0
    (cd tmp && tar $flg ../$file)
    cd tmp
    for tdir in `ls` ; do
	if test -d "$tdir" ; then
	    cd $tdir
	    for lf in `find *` ; do
		if ! test -d "$lf" ; then
		    lfd=`dirname $lf`
		    lfb=`basename $lf`
		    mkdir -p $ldir/$lfd
		    cat $lf >> $ldir/$lfd/$lfb
		    count=`expr $count + 1`
		fi
	    done
	    cd ..
	fi
    done
    cd $CWD
    rm -rf tmp/*
    #rm -f $file
    echo "($count files)"
done

rm -rf tmp
