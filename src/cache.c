/* #include <stdlib.h> */
/* #include <stddef.h> */
/* #include <stdio.h> */
/* #include <string.h> */
#include "dbg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "zlib.h"
#include <assert.h>

#define CHUNK 16384
#define CACHE_FILE "/etc/sm-cache/cache"

int compress_cache()
{
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK)
    return ret;

  FILE *source = fopen(CACHE_FILE, "rb");
  FILE *dest = fopen("/tmp/cache.comp", "wb");
  if (!source || !dest) return -1;

  do {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)deflateEnd(&strm);
      return Z_ERRNO;
    }
    flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;

    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = deflate(&strm, flush);
      assert(ret != Z_STREAM_ERROR);
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        (void)deflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);

  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&strm);
  return Z_OK;
}

void cache(const char *postData)
{

  struct stat st = {0};
  /* char *f = */

  if (stat("/etc/sm-cache", &st) == -1) {
    mkdir("/etc/sm-cache", 0755);
  }

  debug("save the datas! %s", postData);
  FILE *out = fopen(CACHE_FILE, "a");

  if(out == NULL) {
    perror("Error opening file.");
  }

  fseek(out, 0L, SEEK_END);
  unsigned long sz = (unsigned long)ftell(out);
  fseek(out, 0, SEEK_SET);

  debug("FILE SIZE: %ld", sz);

  if (sz > 100000) {
    debug("Cache is getting large, not writing...");
    fclose(out);
    return;
  }

  fprintf(out, "%s", postData);
  fprintf(out, "%s", "\n");
  fclose(out);
}

void send_cache()
{
  if( access( CACHE_FILE, F_OK ) == -1 ) {
    return;
  }

  int ret = compress_cache();
  if (ret != Z_OK)
    debug("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

  return;
}
