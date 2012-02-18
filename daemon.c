#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <libgen.h>

#include "daemon.h"

#define PID_SIZE 32

/* Check path for a valid pidfile, populating structure as we go */
int read_lock(char *path, struct lock_file *lock)
{
	struct stat st;
	char *tmp = NULL, *dir = NULL;

	memset(lock, (int)sizeof(struct lock_file), 0);
	if(stat(path, &st) == 0)	{
		/* If lockfile exists & is not a regular file (stat S_ISREG), error */
		if(!S_ISREG(st.st_mode))	{
			fputs("Error: path exists and isn't regular file\n", stderr);
			return -1;
		}
		lock->exists = 1;
	} else	{
		lock->exists = 0;
	}

	errno = 0;
	access(path, W_OK | R_OK);
	switch(errno)	{
		case EACCES:	/* Access denied, set perm flag to false */
			lock->perm_ok = 0;
			break;
		case ENOENT:	/* Path doesn't exist, see if dirname(path) does */
			if((tmp = strdup(path)) == NULL)
				return -1;
			dir = dirname(tmp);
			if(stat(dir, &st) != 0)	{	/* If dirname(path) doesn't exist */
				fprintf(stderr, "Error: \"%s\" isn't a directory\n", tmp);
				free(tmp);
				return -1;
			}

			/* Set the perm flag according to directory access */
			if(access(dir, W_OK | R_OK) == 0)
				lock->perm_ok = 1;
			else
				lock->perm_ok = 0;
			free(tmp);
			break;
		case 0:		/* Everything is OK */
			lock->perm_ok = 1;
			break;
		default:
			perror("Error in access()");
			return -1;
	}

	if(lock->exists && lock->perm_ok)	{	/* do we try to read pid? */
		int fd;
		char *end;
		size_t n_read;
		char pidbuf[PID_SIZE];

		/* open(), read(), convert pid to integer, checking sanity as we go */
		if((fd = open(path, O_RDONLY)) == -1)	{
			fprintf(stderr, "Error: cannot read %s\n", path);
			return -1;
		}
		n_read = read(fd, pidbuf, PID_SIZE);

		if(n_read < 0)	{
			perror("Error reading pidfile");
			close(fd);
			return -1;
		}
		else if(n_read == 0 || n_read > PID_SIZE - 1)	{
			fprintf(stderr, "Error: %s not a valid pidfile\n", path);
			close(fd);
			return -1;
		}

		pidbuf[n_read - 1] = '\0';
		lock->pid = strtol(pidbuf, &end, 10);
		if(end != &pidbuf[n_read - 2] && end != &pidbuf[n_read - 1])	{
			fprintf(stderr, "Error: %s didn't contain a pid\n", path);
			close(fd);
			return -1;
		}

		/* If valid, now we test the lock and set the locked flag */
		errno = 0;
		if(!flock(fd, LOCK_SH | LOCK_NB))	{
			if(flock(fd, LOCK_UN) == -1)	{
				perror("Error releasing test lock!?");
				close(fd);
				return -1;
			}
			lock->locked = 0;
		} else {
			if(errno == EWOULDBLOCK)	{
				lock->locked = 1;
			} else	{
				perror("Error testing lock");
				close(fd);
				return -1;
			}
		}
		close(fd);
	}

	if((lock->path = strdup(path)) == NULL)
		return -1;
	return 0;
}

/* Exclusively create a lockfile and write our PID into it */
int create_lockfile(char *path)
{
	int fd;
	pid_t pid = getpid();
	char pidstr[16];

	snprintf(pidstr, 15, "%d\n", pid);

	errno = 0;
	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	switch (errno)	{
		case EEXIST:
			fputs("Error: exclusive open failed, file exists\n", stderr);
			break;
		case EPERM:
		case EACCES:
			fputs("Error: permissions do not allow file creation\n", stderr);
			break;
		case 0:
			if(write(fd, pidstr, strlen(pidstr)) != strlen(pidstr))	{
				fputs("Error: write() wrote too few bytes\n", stderr);
				close(fd);
				return -1;
			}
			return fd;
		default:
			perror("open() lockfile");
			break;
	}
	return -1;
}

/* Acquire a lock on pidfile at *path */
int get_lock(char *path, int *fd, int present_ok)
{
	struct lock_file lf;
	*fd = -1;

	if(read_lock(path, &lf) != 0)	{
		return PID_ERROR;
	}
	if(lf.exists && lf.locked)	{
		fprintf(stderr, "Error: %s already exists and is locked for pid %d\n", lf.path, lf.pid);
		return PID_LOCKED;
	} else if(!lf.perm_ok)	{
		if(lf.exists)
			fprintf(stderr, "Error: lock_file already exists but bad permissions\n");
		else
			fprintf(stderr, "Error: lock_file location isn't accessible\n");
	} else if(lf.exists && !lf.locked && !present_ok)	{
		fprintf(stderr, "Error: lock_file is present but unlocked\n");
	} else {	/* If it exists and ok if it exists, or if it doesn't exist */
		if(lf.exists)
			unlink(lf.path);

		if((*fd = create_lockfile(lf.path)) >= 0)	{
			lf.exists = 1;
			lf.locked = 0;
			errno = 0;
			flock(*fd, LOCK_EX | LOCK_NB);
			switch (errno)	{
				case EWOULDBLOCK:
					fputs("Error: cannot lock newly created file?\n", stderr);
					break;
				case 0:
					lf.locked = 1;
					break;
				default:
					perror("flock()");
			}
			return PID_SUCCESS;
		}
	}
	return PID_ERROR;
}
int become_daemon(char *pidfile, char *std_out, char *std_err,
				  char *wd, int flags)
{

	char cwd[WD_MAX] = {0};
	char *root_dir = "/";
	int fd;

	if(wd != NULL && strlen(wd) >= WD_MAX)	{
		fputs("Error: working dir is too long\n", stderr);
		return -1;
	}

	/* First fork, child falls through */
	switch(fork())	{
		case 0:		break;
		case -1:	return -1;
		default:	usleep(10000); _exit(EXIT_SUCCESS);
	}

	if(setsid() == -1)	{
		perror("setsid()");
		return -1;
	}

	/* Second fork, child falls through, now session leader */
	switch(fork())	{
		case 0:		break;
		case -1:	return -1;
		default:	_exit(EXIT_SUCCESS);
	}

	if(wd != NULL)
		strncpy(cwd, wd, strlen(wd) + 1);
	else
		strncpy(cwd, root_dir, strlen(root_dir) + 1);

	if(chdir(cwd) != 0)	{
		perror("chdir()");
		return -1;
	}

	if(pidfile != NULL)	{
		if(get_lock(pidfile, &fd, flags & DMN_PID_E_OK) != PID_SUCCESS)
			return -1;
	}

	fflush(stdout);
	fflush(stderr);

	if(!(flags & DMN_NOCLOSE))	{
		int maxfd = sysconf(_SC_OPEN_MAX) == -1 ? 512 : sysconf(_SC_OPEN_MAX);
		int i;
		for(i = 0; i < maxfd; i++)	{
			if(pidfile != NULL && i != fd)
				close(i);
		}
	}

	if(!(flags & DMN_NOUMASK))
		umask(022);

	if(open("/dev/null", O_RDWR) != STDIN_FILENO)
		return -1;

	if(std_out != NULL)	{
		if((fd = open(std_out, O_CREAT|O_WRONLY|O_APPEND, 0644)) != STDOUT_FILENO)
			return -1;
	} else	{
		if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -1;
	}

	if(std_err != NULL)	{
		if((fd = open(std_err, O_CREAT|O_WRONLY|O_APPEND, 0644)) != STDERR_FILENO)
			return -1;
	} else	{
		if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -1;
	}
	return 0;

}

//#define _PIDTEST
/****************************************************************************/

#ifdef _PIDTEST

int main(int argc, char *argv[])
{
	struct lock_file l = {0};

	if(argc < 2)	{
		puts("Error, required file argument");
		return 1;
	}

	if(become_daemon(argv[1], NULL, NULL, NULL, DMN_PID_E_OK) == 0)	{
		puts("Become daemon success\n");
		fflush(stdout);
		execl("/bin/sleep", "test_name", "100", (char *)NULL);
	}

	return 0;
}

#endif
