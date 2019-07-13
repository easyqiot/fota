#include "ringbuffer.h"


void rb_pushone(RingBuffer *rb, char byte) {
	rb->blob[rb->tail] = byte;
	rb_increment(rb, rb->tail, 1);
	if (rb->tail == rb->head) {
		rb_increment(rb, rb->head, 1);
	}
}


void rb_push(RingBuffer *rb, char *data, Size datalen) {
	Size i;
	for(i = 0; i < datalen; i++) {
		rb_pushone(rb, data[i]);
	}
	rb->blob[rb->tail] = '\0';
}


void rb_pop(RingBuffer *rb, char *data, Size datalen) {
	Size i;
	for (i = 0; i < datalen; i++) {
		data[i] = rb->blob[rb->head];
		rb_increment(rb, rb->head, 1); 
	}
}


int rb_safepush(RingBuffer *rb, char *data, Size datalen) {
	if (rb_canpush(rb, datalen)) {
		rb_push(rb, data, datalen);
		return RB_OK;
	}
	return RB_FULL;
}


int rb_safepop(RingBuffer *rb, char *data, Size datalen) {
	if (rb_canpop(rb, datalen)) {
		rb_pop(rb, data, datalen);
		return RB_OK;
	}
	return RB_INSUFFICIENT;
}
