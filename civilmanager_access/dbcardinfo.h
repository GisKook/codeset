#ifndef CIVILMANAGER_ACCESS_DBCARDINFO_H_H
#define CIVILMANAGER_ACCESS_DBCARDINFO_H_H

struct dbcardinfo;

struct dbcardinfo * dbcardinfo_start(struct cardmanager * cardmanager);
void dbcardinfo_end(struct dbcardinfo * dbcardinfo);

#endif
