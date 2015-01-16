#ifndef CIVILMANAGER_ACCESS_DOWNSTREAMMESSAGE_H_H
#define CIVILMANAGER_ACCESS_DOWNSTREAMMESSAGE_H_H

struct downstreammessage;
struct downstreammessage * downstreammessage_create(struct processappdata * processappdata);
void downstreammessage_destroy(struct downstreammessage * downstreammessage);

#endif
