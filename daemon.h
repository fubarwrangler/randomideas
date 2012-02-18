#ifndef _DAEMON_H
#define _DAEMON_H


#define PID_SUCCESS 0
#define PID_LOCKED  1
#define PID_ERROR   2

/* Flags to be |'d together and passed to become_daemon() */
#define DMN_NOUMASK		01	/* Do not set the umask to 022 */
#define DMN_PID_E_OK	02	/* Overwrite *pidfile if it exists and is unlocked */
#define DMN_NOCLOSE		04	/* Do not close all fds except pidfile */

#define WD_MAX		2048


/* The object representing a pidfile, will be populated by read_lock() below.
 *	.path =		the path to the desired/existing lockfile
 *	.exists =	bool, true if the file is a regular file present in filesystem
 *	.perm_ok =	bool, true if user has read/write permissions on file named
 *				in path if it exists, or on the directory before it if it
 * 				doesn't already exist (as returned by dirname()).
 *	.locked =	bool, true if another process has a lock on the file.
 *	.pid = 		the pid contained in the lockfile iff it exists and is valid
 */
struct lock_file {
    char *path;
    int exists;
    int locked;
    int perm_ok;
    pid_t pid;
};

/* Read lockfile at *path into the lock_file structure *lock, returning 0 on
 * success or, on error, print a message on stderr and return -1.
 */
int read_lock(char *path, struct lock_file *lock);

/* Acquire a lockfile at *path.  The flag present_ok is a boolean that, if
 * set, means that it is not an error if the file already exists and is
 * not locked.  If the file exists and isn't a valid pidfile (contains only
 * the pid--digits and an optional newline) it is still an error to avoid
 * overwriting some unintended file.  The descriptor is available in fd.
 */
int get_lock(char *path, int *fd, int present_ok);

/* Daemonize yourself, (double fork, session leader, close fds, etc...)
 *	@pidfile:	NULL or the path to the pidfile lock-file to use.
 *	@std_out/err:	Path of file to write stdout/err to, or NULL for /dev/null
 *	@wd:	Path of workign dir for the daemon, '/' by default if NULL.
 *	@flags:	See DMN_* above, OR them together to toggle the settings
 *
 * Note: if pidfile is created, its file descriptor will not be closed
 * even if DMN_NOCLOSE is not set. THE USER IS RESPONSIBLE FOR CLOSING
 * THE FD AND CLEANING UP THE FILE JUST BEFORE THE PROGRAM EXITS
 *
 * Returns 0 on success, otherwise prints an error message (if possible,
 * i.e. if we haven't closed stdout/stderr descriptors yet) and returns
 * non-zero.
 */
int become_daemon(char *pidfile, char *std_out, char *std_err,
				  char *wd, int flags);

#endif
