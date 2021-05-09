/* epic-xmms interface to xmms library.  someone sent this to me and I
** assume it's in the public domain, however, the following was to be seen
** at the bottom of this file:
** Contributors:
** Igge Rask <igge_rask@yahoo.com> for the find_song() function
** Kim <haudyr@fnuck.dk> for the mute() function
** EOF
** (both functions were rewritten, ;) 
** I leave this code in the public domain.
**
** The following modifications have been made:
** o code-cleanup/style changes.  I rewrote a few of the functions
** o made the program persistent (i.e. it doesn't exit after one command)
** o added a system to print out new info when a new song is started by
**   using select() to wait on input for a limited time. 
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xmmsctrl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#define false 0
#define true !false

/* function declarations */
char *strtolower(char *str);
int waitforstdin(void);

void song_info(void);
void shuffle(char *arg);
void repeat(char *arg);
void mute(char *left, char *right);
void jump(char *string, char *num);
void match(char *string);

/* global variables */
int session = 0;
int file = 0;

/* program below */
int main(int argc, char **argv) {
	char c, ibuf[256], iargs_rd[3][256], *iargs[3];
	int pos = -1, i;
	int chkpos = -1;

	while ((c = getopt(argc, argv, "if")) != -1) {
		switch (c) {
			case 'i':
				file = 0;
				break;
			case 'f':
				file = 1;
				break;
		}
	}

	while (1) {
		char *s = ibuf, *s2;
		if (!xmms_remote_is_running(session)) /* prevent looping horribly! */
			sleep(2);

		if (!waitforstdin()) {
			if (chkpos != xmms_remote_get_playlist_pos(session)) {
				chkpos = xmms_remote_get_playlist_pos(session);
				song_info();
			}
			continue;
		}
		if ((fgets(ibuf, 256, stdin)) == NULL)
				exit(0);
		ibuf[strlen(ibuf) - 1] = '\0';
		iargs[0] = iargs[1] = iargs[2] = NULL;
		for (i = 0;i < 3 && s != NULL && *s != '\0';i++) {
			s2 = strchr(s, ' ');
			if (s2 != NULL)
				*s2 = '\0';
			strcpy(iargs_rd[i], s);
			iargs[i] = iargs_rd[i];
			if (s2 == NULL) {
				s = s2;
				continue;
			}
			s = s2 + 1;
			while (isspace(*s))
				s++;
		}
		if (!strcmp(iargs[0], "info"))
			song_info();
		else if (!strcmp(iargs[0], "next")) {
			pos = xmms_remote_get_playlist_pos(session);
			xmms_remote_playlist_next(session);
			while (xmms_remote_get_playlist_pos(session) == pos)
				sleep(1);
			chkpos = xmms_remote_get_playlist_pos(session);
			song_info();
		} else if (!strcmp(iargs[0], "prev")) {
			pos = xmms_remote_get_playlist_pos(session);
			xmms_remote_playlist_prev(session);
			while (xmms_remote_get_playlist_pos(session) == pos)
				sleep(1);
			chkpos = xmms_remote_get_playlist_pos(session);
			song_info();
		} else if (!strcmp(iargs[0], "stop"))
			xmms_remote_stop(session);
		else if (!strcmp(iargs[0], "play"))
			xmms_remote_play(session);
		else if (!strcmp(iargs[0], "pause"))
			xmms_remote_pause(session);
		else if (!strcmp(iargs[0], "shuffle"))
			shuffle(iargs[1]);
		else if (!strcmp(iargs[0], "repeat"))
			repeat(iargs[1]);
		else if (!strcmp(iargs[0], "mute"))
			mute(iargs[1], iargs[2]);
		else if (!strcmp(iargs[0], "jump")) {
			jump(iargs[1], iargs[2]);
			chkpos = xmms_remote_get_playlist_pos(session);
		}
		else if (!strcmp(iargs[0], "match"))
			match(iargs[1]);
		else if (!strcmp(iargs[0], "quit"))
			exit(0);
	}
}

/* lowercase a string. */
char *strtolower(char *str) {
	char *s = str;
	while (*s != '\0')
		*s++ = tolower(*s);
	return str;
}

/* waitforstdin()
 * selects() for one second on stdin, if new data is available, returns 1,
 * otherwise, returns 0
 */
#define STDIN_FD 0
int waitforstdin(void) {
	fd_set rfd;
	struct timeval tv;

	tv.tv_sec = 1;
	tv.tv_usec = 0;
	FD_ZERO(&rfd);
	FD_SET(STDIN_FD, &rfd);

	if(select(1, &rfd, NULL, NULL, &tv) < 1)
		return 0;
	else
		return 1;
}

/* grab info about the current file and output it in a script-friendly time
** to stdout. */
void song_info(void) {
	int ppos = xmms_remote_get_playlist_pos(session);
	int lsec = xmms_remote_get_playlist_time(session, ppos) / 1000;
	int osec = xmms_remote_get_output_time(session) / 1000;
	int rate, freq, nch;
	char *track = (file ? xmms_remote_get_playlist_file(session, ppos) :
			xmms_remote_get_playlist_title(session, ppos));
	if (xmms_remote_is_playing(session) != true)
		return;

	xmms_remote_get_info(session, &rate, &freq, &nch);

	printf("info %d %d %.2f %d %.02d:%.02d %.02d:%.02d %s\n", ppos + 1, rate,
			(float)((float)freq / 1000.0), nch, lsec / 60, lsec % 60, osec / 60,
			osec % 60, track);
	fflush(stdout);
	return;
}

void shuffle(char *arg) {
	int shuf = xmms_remote_is_shuffle(session);
	
	if (arg == NULL)
		arg = "tog";
	
	if (!strcmp(arg, "tog")) {
		xmms_remote_toggle_shuffle(session);
	}
	else if(!strcmp(arg, "on") && shuf == false)
		xmms_remote_toggle_shuffle(session);
	else if (!strcmp(arg, "off") && shuf == true)
		xmms_remote_toggle_shuffle(session);
	printf("shuffle %s\n", (shuf == true ? "off" : "on"));
	fflush(stdout);
}

void repeat(char *arg) {
	int rep = xmms_remote_is_repeat(session);
	
	if (arg == NULL)
		arg = "tog";
	
	if (!strcmp(arg, "tog")) {
		xmms_remote_toggle_repeat(session);
	}
	else if(!strcmp(arg, "on") && rep == false)
		xmms_remote_toggle_repeat(session);
	else if (!strcmp(arg, "off") && rep == true)
		xmms_remote_toggle_repeat(session);
	printf("repeat %s\n", (rep == true ? "off" : "on"));
	fflush(stdout);
}

void mute(char *left, char *right) {
	int lvol, rvol;

	if (left == NULL)
		left = "50";
	if (right == NULL)
		right = "50";

	xmms_remote_get_volume(session, &lvol, &rvol);
	/* if volumes are not 0, we're muting.  if they are, we're resetting */
	if (lvol == 0 && rvol == 0)
		xmms_remote_set_volume(session, atoi(left), atoi(right));
	else {
		printf("volume %d %d\n", lvol, rvol);
		fflush(stdout);
		xmms_remote_set_volume(session, 0, 0);
	}
}

void jump(char *song, char *snum) {
	int num = atoi(snum != NULL ? snum : "1");
	int songs = xmms_remote_get_playlist_length(session);
	char *title;
	int i;

	if (song == NULL || num < 1)
		return;
	strtolower(song);
	if (!strcmp(song, "*"))
		*song = '\0';
	for (i = 0;i < songs;i++) {
		title = strtolower(strdup(file ?
					xmms_remote_get_playlist_file(session, i) :
					xmms_remote_get_playlist_title(session, i)));
		if (*song == '\0' || strstr(title, song) != NULL) {
			if (--num == 0) { /* winner */
				free(title);
				xmms_remote_set_playlist_pos(session, i);
				while (xmms_remote_get_playlist_pos(session) != i)
					sleep(1);
				song_info();
				return;
			}
		}
		free(title);
	}
	
	if (*song == '\0')
		printf("skip %s\n", snum);
	else
		printf("jump %s\n", song);
	fflush(stdout);
}

void match(char *string) {
	int i, m;
	int songs = xmms_remote_get_playlist_length(session);
	char *title;

	if (string != NULL)
		strtolower(string);

	for (i = 0,m = 0;i < songs;i++) {
		title = strtolower(strdup(file ?
					xmms_remote_get_playlist_file(session, i) :
					xmms_remote_get_playlist_title(session, i)));
		if (string == NULL) {
			printf("file %s\n", title);
			fflush(stdout);
			m++;
		} else if (strstr(title, string) != NULL) {
			printf("file %s\n", title);
			fflush(stdout);
			m++;
		}
		free(title);
	}
	printf("matches %d\n", m);
	fflush(stdout);
}
