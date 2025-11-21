
#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
namespace exception {
template <typename T> class stack {
  private:
    struct node {
        T data;
        node *next;
        node() : data{}, next{nullptr} {}
        node(const T &data) : data(data) {}
    };
    std::atomic<node *> head;

  public:
    void push(T obj) {
        node *new_head = new node{obj};
        new_head->next = head.load();
        while (!head.compare_exchange_weak(new_head->next, new_head))
            ;
    }

    void pop(T &result) {
        node *old_head = head.load();
        while (old_head &&
               !head.compare_exchange_weak(old_head, old_head->next))
            ;

        result = old_head ? old_head->data
                          : T{}; // copy can throw, and no way to recover
    }
};

}; // namespace exception

namespace mem_leak {

template <typename T> class stack {
  private:
    struct node {
        std::shared_ptr<T> data;
        node *next;
        node() : data{}, next{nullptr} {}
        node(const T &data) : data(std::make_shared<T>(data)) {}
    };
    std::atomic<node *> head;

  public:
    void push(T obj) {
        node *new_head = new node{obj};
        new_head->next = head.load();
        while (!head.compare_exchange_weak(new_head->next, new_head))
            ;
    }

    std::shared_ptr<T> pop(T &result) {
        node *old_head = head.load();
        while (old_head &&
               !head.compare_exchange_weak(old_head, old_head->next))
            ;
        return old_head
                   ? old_head->data
                   : std::shared_ptr<T>{}; // even if rvo and move both fail
        // copy might still throw, but practically copying a ptr only involves
        // atomic increments etc, so theres no chance
        // practically allocators/control block creations are more likely to
        // throw

        // mem leak, as the new_head we allocated is never deleted! the node
        // always holds shared_ptr... deleting normally might cause problems if
        // other threads are viewing that old_head and access deleted old_head
        // in while loop!
    }
};
}; // namespace mem_leak

namespace low_contention {
template <typename T> class stack {
  private:
    struct node {
        std::shared_ptr<T> data;
        node *next;
        node() : data{}, next{nullptr} {}
        node(const T &data) : data(std::make_shared<T>(data)) {}
    };
    std::atomic<node *> head;
    std::atomic<std::size_t> threads_in_pop{0};
    std::atomic<node *> to_be_deleted{nullptr};

    static void delete_nodes(node *list) {
        while (list) {
            node *next = list->next;
            delete list;
            list = next;
        }
    }

    void chain_pending_nodes(node *first, node *last) {
        last->next = to_be_deleted;
        while (!to_be_deleted.compare_exchange_strong(last->next, first))
            ;
    }

    void chain_pending_nodes(node *list) {
        node *first = list;
        while (list->next) {
            list = list->next;
        }
        chain_pending_nodes(first, list);
    }
    void chain_pending_node(node *n) { chain_pending_nodes(n, n); }

    void try_reclaim(node *old_head) {
        if (!old_head) {
            --threads_in_pop;
            return;
        }
        if (threads_in_pop == 1) {
            node *nodes_to_delete = to_be_deleted.exchange(nullptr);
            if (--threads_in_pop == 0) {
                // free to delete
                delete_nodes(nodes_to_delete);
            } else if (nodes_to_delete) {
                // some new guy came, and there still pending nodes
                chain_pending_nodes(nodes_to_delete); // append back again
            }
            delete old_head; // only this guy has ref to old_head
        } else {
            chain_pending_node(old_head);
            --threads_in_pop;
        }
    }

  public:
    void push(T obj) {
        node *new_head = new node{obj};
        new_head->next = head.load();
        while (!head.compare_exchange_weak(new_head->next, new_head))
            ;
    }

    std::shared_ptr<T> pop(T &result) {
        threads_in_pop++;
        node *old_head = head.load();
        while (old_head &&
               !head.compare_exchange_strong(old_head, old_head->next))
            ;
        std::shared_ptr<T> res;
        if (old_head) {
            res.swap(old_head->data);
        }
        try_reclaim(old_head);
        return res;
    }
};
}; // namespace low_contention

namespace hazard_ptr {
// Generic hazard_pointer implementation!!
const std::size_t MAX_PTR = 100;
struct hazard_pointer {
    std::atomic<void *> pointer;
    std::atomic<std::thread::id> id;
};

hazard_pointer hazard_pointers[MAX_PTR];

class hp_owner { // this obj will be thread local, so hp is pointer to my own
                 // thread slot
  private:
    hazard_pointer *hp{nullptr};

  public:
    hp_owner() {
        for (int i = 0; i < MAX_PTR; i++) {
            std::thread::id old_id;
            if (hazard_pointers[i].id.compare_exchange_strong(
                    old_id, std::this_thread::get_id())) {
                // success, free slot
                hp = &hazard_pointers[i]; // give it to hp
                break;
            }
        }
        if (!hp) {
            throw std::runtime_error("no hazard pointer availabe");
        }
    }

    ~hp_owner() {
        hp->id = std::thread::id();
        hp->pointer = nullptr;
    }

    std::atomic<void *> &get_pointer() { return hp->pointer; }
};

bool outstanding_hazard_pointers_for(void *ptr) {
    for (int i = 0; i < MAX_PTR; i++) {
        if (hazard_pointers[i].pointer.load() == ptr) {
            return true; // some other thread sees it
        }
    }
    return false;
}

std::atomic<void *> &get_hazard_pointer_for_this_thread() {
    thread_local static hp_owner hp;
    return hp.get_pointer();
}

struct data_to_reclaim {
    void *data;
    std::function<void(void *)> deleter;
    data_to_reclaim *next;

    template <typename T>
    data_to_reclaim(T *data_)
        : data(static_cast<void *>(data_)),
          deleter{[](void *data) { delete static_cast<T *>(data); }},
          next{nullptr} {}
    // original was T type, so static_cast is wellformed!!

    // deleter via a type erasure way!
    ~data_to_reclaim() { deleter(data); }
};

std::atomic<data_to_reclaim *> nodes_to_reclaim{nullptr}; // linked list to
                                                          // retrieve nodes

void add_to_reclaim_list(data_to_reclaim *node) {
    node->next = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->next, node))
        ;
}

template <typename T> void reclaim_later(T *data) {
    add_to_reclaim_list(new data_to_reclaim{data});
    // data is like data -> Heap_mem , and we pass pointer p -> heap
    // reclaim_node ---> heap_mem (data node) when we delete this
    // reclaim_node on heap, it cleans up after heap_memory
    // as well of the original data.
}

void delete_nodes_with_no_hazard() {
    data_to_reclaim *current = nodes_to_reclaim.exchange(
        nullptr); // remove cur list, the list will build up next time
    while (current) {
        data_to_reclaim *const next = current->next;
        if (!outstanding_hazard_pointers_for(current->data)) {
            delete current; // cleans up internal ptr via type erased deleter,
                            // and deletes for the new as well.
        } else {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

template <typename T> class stack {
    struct node {
        std::shared_ptr<T> data;
        node *next;
        node(const T &data) : data(std::make_shared<T>(data)), next(nullptr) {}
    };
    std::atomic<node *> head;

  public:
    void push(T obj) {
        node *new_head = new node{obj};
        new_head->next = head.load();
        while (!head.compare_exchange_strong(new_head->next, new_head))
            ; // new_head->next is definitely constructed! no visbility issue
    }
    std::shared_ptr<T> pop() {
        std::atomic<void *> &hp = get_hazard_pointer_for_this_thread();
        node *old_head = head.load();
        do {
            node *tmp;
            do {
                tmp = old_head;
                hp.store(tmp); // register as a hazard ptr for this thread
                old_head = head.load();
            } while (tmp != old_head); // do until we are sure old_head is
                                       // registered (no changes in bg)
        } while (old_head &&
                 !head.compare_exchange_strong(old_head, old_head->next));

        hp.store(nullptr); // clear hazard pointer, once done.
        // so it means this thread is not looking at anything!
        // so it'll help in logic of outstanding_hazard_ptr for old_head.
        std::shared_ptr<T> res;
        if (old_head) {
            // if we removed something
            res.swap(
                old_head
                    ->data); // this enables NRVO, so no ref count
                             // incrementing if we did old_head->data, plus easy
                             // to return if we delete old_head as well.
            if (outstanding_hazard_pointers_for(old_head)) {
                reclaim_later(old_head);
            } else {
                // two invariants hold: 1. no other thread currently sees this
                // expired ptr (outstanding check)
                //  2. no thread can see it later (node removed)
                // can free
                delete old_head;
            }
            hazard_ptr::delete_nodes_with_no_hazard();
        }
        return res;
    }
};
}; // namespace hazard_ptr

int main() {}
