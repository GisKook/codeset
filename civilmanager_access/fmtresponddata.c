#include <assert.h>
#include "encodeprotocol.h"
#include "list.h"

struct fmtresponddata{
	unsigned char* buf;
	unsigned char len; // the max length is 232
	int fd;
	struct list_head list;
}

int fmtresponddata_add(struct list_head * head, unsigned char * buf, int len, int fd){
	assert( buf != NULL && fd > 0 && len <= MAX_RESPOND_LEN); 
	struct fmtresponddata * fmtresponddata = (struct fmtresponddata*)malloc(struct fmtresponddata);
	if(unlikely(fmtresponddata == NULL)){
		fprintf(stderr, "malloc error. %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

		return -1;
	}
	fmtresponddata->buf = buf;
	fmtresponddata->len = len;
	fmtresponddata->fd = fd;
	list_add_tail(head, &fmtresponddata->list);

	return 0;
}

int fmtresponddata_clear(struct fmtresponddata* fsd){
	assert(fsd != NULL);
	free(fsd->buf);
	fsd->buf = NULL;
	list_del(&fsd->list);
	free(fsd);
	fsd = 0;

	return 0;
}

