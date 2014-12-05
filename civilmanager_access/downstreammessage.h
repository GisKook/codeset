#ifndef CIVILMANAGER_ACCESS_DOWNSTREAMMESSAGE_H_H
#define CIVILMANAGER_ACCESS_DOWNSTREAMMESSAGE_H_H

struct downstreammessage;
struct downstreammessage * downstreammessage_create(struct sockets_buffer * sbuffer);
void downstreammessage_destroy(struct sockets_buffer * sbuffer);

#endif
