#!/bin/sh

# $Id: rotate.sh,v 1.4 2002/11/11 11:08:23 wd Exp $

# this is a script to perform log rotation for the script.  it requires
# that log rotation be set on (in some form) by the user, and that the
# script install a crontab entry to run it at the appropriate time.

# arguments should be:
# rotate.sh <path-to-script>
APATH=$1

# function to test a directory for usability
test_directory () {
    DIR="$1"
    if test "x$DIR" = "x" ; then
	echo missing directory entry
	return 1
    fi

    if test ! -d "$DIR"  ; then
	echo $DIR is not a directory!
	return 1
    fi
    return 0
}

# function to get a setting from a savefile.  the setting is the first
# argument.  the value of the setting is stored in a variable of the same
# name.
get_setting () {
    FILE="etc/modules/log.save"
    NAME="$1"

    setting=`grep -E "^ac\\.config\\.set $NAME " $FILE`
    setting=`echo $setting | sed -E "s/^ac\.config\.set $NAME (.*)$/\\1/"`

    eval $NAME="$setting"
}    

if test_directory "$APATH" ; then
    cd $APATH
else
    exit 1
fi

get_setting log_directory
get_setting log_directory_mask
get_setting log_archive
get_setting log_archive_compress
get_setting log_archive_dir

if test_directory "$log_directory" ; then
    cd $log_directory
else
    exit 1
fi
if test_directory "$log_archive_dir" ; then
else
    exit 1
fi

# determine which directories we need to archive in this run.  we
# assume that we've been run on a reasonable schedule.  if there is machine
# downtime or other unexpected occurances, there's not a lot we can do.
case $log_archive in
    daily)
	if test -d $dir ; then
	    dirs=`date -v -1d +%Y-%m-%d`
	fi
    ;;
    weekly)
	for i in 7 6 5 4 3 2 1 ; do
	    dir=`date -v -${i}d +%Y-%m-%d`
	    if test -d $dir ; then
		dirs="$dirs $dir"
	    fi
	done
    ;;
    monthly)
	dirs="`date -v -1m +%Y-%m-`*"
    ;;
    *)
	exit 0
    ;;
esac

if test "x$dirs" = "x" ; then
    echo Nothing to archive.
    exit 0
fi

# now build our command line.  we name the file after the day it was
# archived on, but we must decide what flags to give to tar.

flags="cf"
file="$log_archive_dir/logs.`date +%Y-%m-%d`.tar"
case $log_archive_compress in
    b)
	if test "x`which bzip2`" != "x" ; then
	    flags="ycf"
	    file="$file.bz2"
	fi
    ;;
    z)
	if test "x`which gzip`" != "x" ; then
	    flags="zcf"
	    file="$file.gz"
	fi
    ;;
esac

echo $flags
echo $file
echo $dirs
if tar $flags $file $dirs ; then
    rm -rf $dirs
else
    echo "could not complete archive operation"
fi

chmod $log_directory_mask $file
chmod -x $file
exit $?

