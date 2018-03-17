
void init_queue_ipc(ipc_queue * q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void enqueue_ipc(ipc_queue * q, Msg_Des * task) {
    if (q->size == 0) {
        q->head = task;
    } else {
        q->tail->next = task;
    }
    q->tail = task;
    task->next = NULL;
    q->size++;
}

Msg_Des * deque_ipc(ipc_queue * q) {
    Msg_Des * p = NULL;

    if (q->size == 0) {
        return p;
    } else if (q->size == 1) {
        p = q->head;
        q->head = NULL; 
        q->tail = NULL;
        q->size--;
        return p;
    } else {
        p = q->head;
        q->head = p->next;
        q->size--;
        return p;
    }
}

Msg_Des * peek_ipc(ipc_queue * q) {
	return q->head;
}
