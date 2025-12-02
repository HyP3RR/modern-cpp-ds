#include <atomic>
#include <condition_variable>
#include <mutex>

/*
  reader-writer

 think more on these:
 First readers-writers problem (reader-priority):
Readers are prioritized — writers may starve.

Second readers-writers problem (writer-priority):
Writers are prioritized — readers may starve.

Fair version:
No starvation for readers or writers.


this is classic, the reader prioritized one
 */
std::mutex read_lock;
std::mutex writer_lock;

int READ_COUNT = 0;

void reader() {
    {
        std::lock_guard<std::mutex> lk(read_lock);
        READ_COUNT++;
        if (READ_COUNT == 1)
            writer_lock.lock();
    }

    // reading etc

    {
        std::lock_guard<std::mutex> lk(read_lock);
        READ_COUNT--;
        if (READ_COUNT == 0)
            writer_lock.unlock();
    }
}

void writer() {

    std::lock_guard<std::mutex> ulk(writer_lock);

    /*
      start writing / updating
    */
}
