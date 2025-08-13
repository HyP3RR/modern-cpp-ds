#include <iostream>
#include <cstdint>
#include <map>
#include <stdexcept>

//MMU sim for educational purposes.

/*
  total address space = 32 bits -> 4GB
consider virtual address space = 16MB -> 2^24 -> 24 bits
let page size = 4kb(LINUX) -> 12 bits -> offset within a page

let page directory entry = page table entry = 4 Bytes

let Page table -> 32 pages -> size needed 32 * 4 = 2^7 B (or 7 bits)

thus, our page directory size is (24-7-12) = 5 bits (contains 8 Page directory
entry)

[5,7,12]


for now page fault DNE , to add page fault, need to add a present bit in page
table
if the bit is false -> invoke page fault handler of OS
the page fault handler is never swapped out of RAM and it'll always be present.
since we do page walk via "hardware" the tlb entry will be added and page fault
handler will never have the infinite tlb miss pitfall.

implementation of page fault handler involves swapping the disk address in and
out. to service the request of the process.

 */

constexpr uint8_t PAGE_DIRECTORY_SHIFT = 19;
constexpr uint8_t OFFSET_SHIFT = 12;

constexpr uint32_t OFFSET_MASK = 0b111111111111;
constexpr uint32_t VPN_MASK = 0b111111111111000000000000;
constexpr uint32_t PAGE_DIR_MASK = 0b111110000000000000000000;
constexpr uint32_t PAGE_TABLE_MASK = 0b000001111111000000000000;


std::map<uint32_t, std::pair<uint32_t, uint8_t>> TLB; // easy to clear later
// technically should be 24 bit to int, but no 24 bit integer

//virtual address -> (physical address, permission bits)

struct page_directory_entry {
  bool is_valid;
  uint32_t page_frame_Next_PageTable;
};

  page_directory_entry Page_Directory[(1<<5)];

  struct page_table_entry {
  bool is_valid;
    uint8_t protection_bits;
    uint32_t Physical_page_frame_number;
    uint8_t present_bit; //if page in swap space or not.
  };

page_table_entry Page_Table[(1<<7)]; //can be multiple  


uint32_t CR3 = 0; //CR3 is PDBR


bool TLB_Lookup(const uint32_t virtual_address,uint32_t &tlb_entry) {
  if (TLB.count(virtual_address) == true) {
    tlb_entry = TLB[virtual_address].first; //contains page frame number final
      return true;
    }
  else return false;  
}

bool permission_check(const uint32_t virtual_address) {
    if (TLB[virtual_address].second % 2 == 0)
      return false;
    else return true;    
    //right most bit 1 in permission bits -> permission allowed
}



template<typename T>
T Access_Memory(uint32_t addr) {
    T* treatPointer = reinterpret_cast<T*>(addr);
    return *treatPointer;
    
}
bool Can_access(uint8_t protection_bits) {
  return protection_bits%2 == 0; //0 can access
}

void TLB_Insert(uint32_t virtual_page_no, uint32_t page_frame_number,
                uint8_t protection_bits) {
  // add entry
  //need to do manual page table walk and
}

void page_fault_handler() {
  //use disk_address to service page replacement or just get it directly.
  
}


uint32_t Address_Lookup(uint32_t virtual_page_no){
 //should be 24 bit virtual address but no 24 bit int                        
  uint32_t tlb_entry = 0;
  bool found = TLB_Lookup(virtual_page_no, tlb_entry);
  if (found == true) {
      if (permission_check(virtual_page_no) == true) {
        uint32_t offset = virtual_page_no & OFFSET_MASK;
        uint32_t physical_addr = (tlb_entry << OFFSET_SHIFT) | offset;
        // tlb entry contains entry of base of the page corresponding to virtual
        // address

        // if access -> ACCESS_MEM(physical_addr);
        // else just return phy_addr
	return physical_addr;
      } else {
	std::runtime_error("PROTECTION FAULT");
        }      
      }
  else {
    //TLB miss case.
        uint32_t page_directory_no =
            (virtual_page_no & PAGE_DIR_MASK) >> PAGE_DIRECTORY_SHIFT;
        uint32_t PDE_Addr =
            CR3 +
            page_directory_no *
                sizeof(page_directory_entry);
	page_directory_entry value_at_PDE_Addr = Access_Memory<page_directory_entry>(PDE_Addr);        
    // first 5 bits
 if (value_at_PDE_Addr.is_valid == false) {
        // check for validity at the appropriate address (exact)
   //if valid is false, we gon        
      std::runtime_error("Segmentation fault");
    }
 else {

      // this value_at_pde_addr gives the base address of next page table
      // pointed to by the directory
      uint32_t PTE_index =
          (virtual_page_no & PAGE_TABLE_MASK) >> (OFFSET_SHIFT);
      uint32_t PTE_Addr =
          (value_at_PDE_Addr.page_frame_Next_PageTable << OFFSET_SHIFT) +
          sizeof(page_table_entry) * PTE_index;
      // reasont to shift -> PDE entry contains page_frame number EXACT
      // so its like pde_1 [pfn 1,2 ,3 4...] pde_2[8,9,10..]
      // since hardware maps it on byte basis, the technical address we looking
      // for is X<<offset_shift , since its really 1KB pages each so theyre at
      // 0, 1024B, etc.      
      page_table_entry value_at_PTE_Addr = Access_Memory<page_table_entry>(PTE_Addr);
      if (value_at_PTE_Addr.is_valid == false)
        std::runtime_error("SIGSEV");
      else if (Can_access(value_at_PTE_Addr.protection_bits) == false)
        std::runtime_error("PROTECTION_FAULT");
      else if (value_at_PTE_Addr.present_bit == false) {
        // page fault
        page_fault_handler();
        // switch to kernel mode this happens using ESP/RSP register
        // which contains kernel process stack, saving user space info
        // on it. update CPL (current privelege mode) = 0 now the IDT
        // (interrupt descripter table) is checked to get pointer to
        // page fault handler... since we are still operating on
        // virtual addresses of this process, TLB is called to find
        // the page of this func address, this will always be present
        // (kernel pages never swapped out) so the kernel page is
        // found and physical address is used to execute the page
        // fault handler instructions. (fixed)
        // iret instruction, or iretq is called -> restores by popping kernel
        // stack, now the tlb instr (ie this line, executes normally after
        // this).

        // fun summary  :  basically page fault involves another tlb check. 
	int ans = Address_Lookup(virtual_page_no); //retry
        }        
      else {
        TLB_Insert(virtual_page_no,
                   value_at_PTE_Addr.Physical_page_frame_number,
                   value_at_PTE_Addr.protection_bits);
        int ans =
            Address_Lookup(virtual_page_no); // retry and definitely hit 100%
	return ans;
 
        }
    }    
        

    }
  
}



int main() {
    uint32_t x = 5;
    uint32_t virtual_address =(reinterpret_cast<uint64_t>(&x))&(UINT32_MAX);
    uint32_t VPN = (virtual_address&VPN_MASK)>>OFFSET_SHIFT;
    uint32_t phy_add = Address_Lookup(VPN);
    
    
  
}
