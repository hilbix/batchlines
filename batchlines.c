#include "tino/alarm.h"
#include "tino/filetool.h"
#include "tino/buf_line.h"
#include "tino/strprintf.h"
#include "tino/signals.h"
#include "tino/getopt.h"

#include "batchlines_version.h"

/* options	*/
static const char	*filename, *tmpext;
static long		timeout, maxlines;
static int		nosync;
static char		eol_i, eol_o;
static int		width;
static int		safemode;

/* calculated	*/
static char		*intermediate;
static char		**exts;
static int		extscount, rotcount;
static int		filefd = -1;

static int		need_rot;
static TINO_BUF		ibuf, obuf;

static void
flusher(int force)
{
  if (tino_buf_get_lenO(&obuf) < (force ? 1 : BUFSIZ))
    return;
  if (filefd<0)
    {
      filefd	= tino_file_open_createE(intermediate, O_RDWR|(safemode ? O_EXCL : O_TRUNC), 0666);
      if (filefd<0)
        tino_exit("error: cannot create '%s'", intermediate);
    }
  if (tino_buf_write_away_allE(&obuf, filefd, -1))
    tino_exit("error: cannot write to '%s'", intermediate);
}

static void
rotate(int end)
{
  char	*name;

  need_rot	= 0;
  flusher(1);
  if (filefd<0)
    return;

  name	= tino_file_path_appendO(NULL, filename);
  if (!end)
    {
      char	*nr = tino_str_printf(".%*d%s", width, 1+rotcount, exts[rotcount%extscount]);

      name	= tino_file_path_appendO(name, nr);
      tino_freeO(nr);
      rotcount++;
    }
  else
    name	= tino_file_path_appendO(name, exts[0]);

  if (!nosync)
    tino_file_flush_fdE(filefd);
  tino_file_close_ignO(filefd);
  filefd	= -1;
  if ((safemode ? tino_file_renameE : tino_file_rename_unlinkEbs)(intermediate, name))
    tino_exit("error: cannot rename '%s' to '%s'", intermediate, name);
}

static void
usr1(void)
{
  need_rot	= 1;
  tino_alarm_syntheticO();
}

static int
timed_rotate(void *user, long delta, time_t now, long run)
{
  rotate(0);
  return 0;
}

static int
usr1_rotate(void *user, long delta, time_t now, long run)
{
  if (need_rot)
    rotate(0);
  return 0;
}

static int
batchlines(void)
{
  const char	*line;
  int		lines;

  intermediate	= tino_file_path_appendO(tino_file_path_appendO(NULL, filename), tmpext);

  /* I am really not happy how this works out.
   *
   * I'd like to be able to just trigger some alarms from within signals.
   * However, this way, we need two routines and some flag for workaround.
   */
  tino_alarm_set(0, usr1_rotate, NULL);
  if (timeout)
    tino_alarm_set(timeout, timed_rotate, NULL);

  lines	= 0;
  while ((line = tino_buf_line_read(&ibuf, 0, eol_i))!=0)
    {
      lines++;
      tino_buf_add_sO(&obuf, line);
      tino_buf_add_cO(&obuf, eol_o);
      if (!maxlines || lines < maxlines )
        flusher(0);
      else
        {
          rotate(0);
          lines	= 0;
        }
    }
  rotate(1);
  return 0;
}

int
main(int argc, char **argv)
{
  int		argn;
  char		*def[] = { "", NULL };

  argn	= tino_getopt(argc, argv, 1, 0,
                      TINO_GETOPT_VERSION(BATCHLINES_VERSION)
                      " filename [.ext..]\n"
                      "	Write stdin to given filename.tmp, line-by-line. (.tmp see -i)\n"
                      "	At the end sync file and rename it to filename.ext (atomically)\n"
                      "	Multiple .ext are cycled, missing .ext is '' (none)\n"
                      "	The incomplete last lines (missing NL) is written to stdout.\n"
                      "	On SIGUSR1, file is synced plus rotated to filename.NNN.ext\n"
                      "	Rotation can be done after given time (-t) or lines (-n), too"

                      TINO_GETOPT_USAGE
                      "h	this help"
                      ,

                      TINO_GETOPT_STRING
                      TINO_GETOPT_DEFAULT
                      "e ext	intermediate extension"
                      , &tmpext,
                      ".tmp",

                      TINO_GETOPT_CHAR
                      TINO_GETOPT_DEFAULT
                      "i char	input linefeed character ('' is NUL, NUL is always EOL)"
                      , &eol_i,
                      '\n',

                      TINO_GETOPT_LONGINT
                      "n lines	rotate after the given number of lines (default: 0=never)"
                      , &maxlines,

                      TINO_GETOPT_CHAR
                      TINO_GETOPT_DEFAULT
                      "o char	input linefeed character ('' is NUL, NUL is always EOL)"
                      , &eol_o,
                      '\n',

                      TINO_GETOPT_FLAG
                      "s	safe mode (try not to overwrite files)"
                      , &safemode,

                      TINO_GETOPT_LONGINT
                      TINO_GETOPT_TIMESPEC
                      "t sec	rotate after the given timeout in seconds (default: 0=none)"
                      , &timeout,

                      TINO_GETOPT_FLAG
                      "u	unsynced (do not sync file)"
                      , &nosync,

                      TINO_GETOPT_INT
                      TINO_GETOPT_DEFAULT
                      "w nr	counter width (for numbered rotates)"
                      , &width,
                      1,

                      NULL
                      );

  if (argn<=0)
    return 1;

  tino_sigfix(SIGUSR1);
  tino_sigset(SIGUSR1, usr1);
  filename	= argv[argn++];
  exts		= argv+argn;
  extscount	= argc - argn;
  if (!extscount)
    {
      extscount	= 1;
      exts	= def;
    }

  return batchlines();
}

