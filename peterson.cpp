#define N 2



volatile int turn;
volatile int interested[N];



/*
  peterson's solution gives mutual exclusion, progress and
  bounded wait (BW for 2 threads only)
  peterson's solution assumes Sequential consistent ordering and atomic
  read and writes.
  first software soln, when hardware didn't have locked instructions.

  classic peterson makes sense, but logically felt a bit off.
  so i tweaked it to match my own idea of peterson's solution.
  the correctness is preserved, just the idea is more intuitive than
  what books and academia presents.

  process numbers 0,1 allowed only
*/


//my approach
void enter_region(int process){
  interested[process] = true;
  int other = 1-process;
  while(interested[other] == true && turn == other); // if both interested, the process who has its turn goes in
  turn = process; //now its my turn
}

void leave_region(int process){
  int other = 1 - process;
  interested[process] = false;
  turn = other; //to ensure bounded wait, atleast give the other process a chance.
}

//peterson's solution
void enter(int process){
  interested[process] = true;
  int other = 1-process;
  turn = other; // politely give other process a chance
  while(interested[other] == true && turn == other);
}

void exit(int process){
  interested[process] = false;
}
