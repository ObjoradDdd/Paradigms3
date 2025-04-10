
#ifndef REQUEST_H
#define REQUEST_H



struct Request {
    int groupId;
    int priority;
    int requestId;

    Request(int group, int prio, int id)
        : groupId(group), priority(prio), requestId(id) {}
};



#endif
