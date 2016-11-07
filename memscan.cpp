#include <iostream>
#include <stdio.h>
#include <vector>
#include <windows.h>

using namespace std;

#define WRITABLE ( PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY )
#define IS_IN_SEARCH(mb, offset) (mb->searchmask[(offset)/8] & (1<<((offset) % 8)))
#define REMOVE_FROM_SEARCH(mb, offset) mb->searchmask[(offset)/8] &= ~(1<<(offset % 8))

// Memory block data structure
// -hproc: Handle of the process the memory block belongs to.
// -addr: Base address of the memory block in the process's virtual address space.
// -size: Size of the page region of pages with similar attributes.
// -buffer: Buffer to hold 
// -searchmask:
// -matches:
// -data_size:
// -next: Next memory block. Acts as a linked list.
typedef class _Memblock {
public:
	HANDLE hProc;
	unsigned char *addr;
	int size;
	vector<unsigned char> buffer;

	vector<bool> searchmask;
	int matches;
	int data_size;

	_Memblock *next;

	_Memblock(HANDLE hProc, MEMORY_BASIC_INFORMATION *meminfo, int data_size) {
		this->hProc = hProc;
		this->addr = (unsigned char*) meminfo->BaseAddress;
		this->size = meminfo->RegionSize;
		vector<unsigned char> temp_buf(meminfo->RegionSize, 0);
		this->buffer = temp_buf;
		vector<bool> temp_mask(meminfo->RegionSize/8, 1);
		this->searchmask = temp_mask;
		this->matches = meminfo->RegionSize;
		this->data_size = data_size;
		this->next = NULL;
	}

/*
	void update() {
		static unsigned char temp_buf[128*1024];
		SIZE_T bytes_left;
		SIZE_T total_read;
		SIZE_T bytes_to_read;
		SIZE_T bytes_read;

		if(mb->matches > 0) {
			bytes_left = this->size;
			total_read = 0;
			this->matches = 0;
		}

	}
	*/

	~_Memblock() {}

} Memblock;

typedef class _Scan {
public:
	Memblock *head;

	_Scan(unsigned int pid, int data_size) {
		head = NULL;
		MEMORY_BASIC_INFORMATION meminfo;
		unsigned char *addr;
		
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

		if(hProc) {
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			while(addr < si.lpMaximumApplicationAddress) {
				if(VirtualQueryEx(hProc, addr, &meminfo, sizeof(meminfo)) == 0) {
					break;
				}
				if((meminfo.State & MEM_COMMIT) && (meminfo.Protect & WRITABLE)) {
					Memblock *mb = new Memblock(hProc, &meminfo, data_size);
					if(mb) {
						mb->next = head;
						head = mb;
					}
				}
				addr = (unsigned char*) meminfo.BaseAddress + meminfo.RegionSize;
			}
		}
	}

	void scan_dump() {
		Memblock *temp_head = this->head;
		while(temp_head) {
			printf("0x%08x %d\r\n", temp_head->addr, temp_head->size);
			/*
			for(int i = 0; i < temp_head->size; i++) {
				printf("%02x", temp_head->buffer[i]);
			}
			*/
			temp_head = temp_head->next;
		}
	}

	~_Scan() {}

} Scan;

int main(int argc, char *argv[]) {
	Scan new_scan(atoi(argv[1]), 4);
	if(new_scan.head) {
		new_scan.scan_dump();
	}
	return 0;
}















