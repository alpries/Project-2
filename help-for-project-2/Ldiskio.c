#include "Llibc.h"
#include "types.h"
#include "param.h"
#include "fs.h"
#include "buf.h"
#include "Ldiskio.h"

/* This reads or writes one disk block at raw low level  */
void
disk_block_rw(struct buf *b, int writeflag)
{
	b->disk_rw_fail = 0;

	if (Llseek(b->dev_fd, b->blockno * BSIZE, SEEK_SET) < 0)
		b->disk_rw_fail = 1;
	else {	/* seek successful */
		if (writeflag == 0) { /* read */
			if (Lread(b->dev_fd, b->data, BSIZE) < 0)
				b->disk_rw_fail = 1;
		} else { /* write */
			if (Lwrite(b->dev_fd, b->data, BSIZE) < 0)
				b->disk_rw_fail = 1;
    	}
	}
}


