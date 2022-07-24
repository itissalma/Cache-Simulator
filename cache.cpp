#include <iostream>
#include  <iomanip>
#include <vector>
#include <cmath> 
#include <climits>
using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024) // 2^26
#define		CACHE_SIZE		(64*1024)	// 2^16

enum cacheResType {MISS=0, HIT=1};

// class that represents what is stored in cacheline + counter for LFU
class cacheLine
{
	public:
	unsigned int counter;
	bool valid;
	unsigned int tag;
	cacheLine(unsigned int c, bool v, unsigned int t): counter(c), valid(v), tag(t){}
};

// set associative cache class, takes line size and number of ways as input, initialize cache size to already defined CACHE_SIZE macro
class setAssociativeCache
{
	private:
		unsigned int size = CACHE_SIZE;
		unsigned int lineSize;
		unsigned int totCacheLines;
		unsigned int numWays;
		unsigned int numSet;
		unsigned int byteOffset;
		unsigned int setOffset;
		vector< vector<cacheLine> > cache; //2D, setNum x numLines per set/num ways
	public:
	//Constructor 
	setAssociativeCache(unsigned int line_size, unsigned int ways)
	{
		byteOffset = log2(line_size); 
		lineSize = line_size;
		totCacheLines = size/lineSize;
		numWays = ways;
		numSet = totCacheLines / numWays;
		setOffset = log2(numSet);
		cache = vector< vector<cacheLine> >(numSet, vector<cacheLine>(numWays, cacheLine(0, 0, 0))); // fill cache with empty cachelines 
	}
	// function  to check if address is in cache, returns boolean
	bool search(unsigned int address)
	{
		// get details from address
		address >>= byteOffset; // remove byte offset
		unsigned int setAddress = address % ((int)pow(2, setOffset)); // get set number
		address = address >> setOffset;
		bool dirty = false;
		// loop and check which line has the tag
		
		for(int i = 0; i < numWays; i++){
			if(cache[setAddress][i].valid){
				if(cache[setAddress][i].tag == address){
					cache[setAddress][i].counter++;
					return true;
				}
			}
		}
		return false;
	}
	// function to insert address in cache, uses LFU replacement policy
	void insert(unsigned int address)
	{
		unsigned int setAddress, tag;
		address >>= byteOffset;
		setAddress = address % ((int)pow(2, setOffset));
		tag = address >> setOffset;
		bool flag = false; // flag to check if added, if not, we loop again till first empty slot and add tag
		for(int i = 0; i < numWays; i++)
		{
		    
			if(cache[setAddress][i].valid)
			{
				continue;
			}
			else
			{
			    
				flag = true;
				cache[setAddress][i].counter++;
				cache[setAddress][i].valid = 1;
				cache[setAddress][i].tag = tag;
				break;
			}
		}
		// replacement policy -> lfu
		if(!flag)
		{
			int least, leastAddress;
			least = INT_MAX;
			leastAddress = -1;
			for(int i = 0; i < numWays; i++)
			{	
				cacheLine line = cache[setAddress][i];
				if(line.counter < least)
				{
					least = line.counter;
					leastAddress = i;
				}
			}
			cache[setAddress][leastAddress].counter = 1;
			cache[setAddress][leastAddress].tag = tag;
		}
	}
	// function to print what is present in cache
	void printCache()
	{
	    for(int i = 0; i< numSet; i++){
	        for (int j = 0; j < numWays; j++){
	            cout << cache[i][j].valid << "\t" << cache[i][j].counter << "\t" <<hex << cache[i][j].tag << endl;
	        }
	    }
	}

};

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	static unsigned int addr=0;
	return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
	static unsigned int addr=0;
	return  rand_()%(24*1024);
}

unsigned int memGen3()
{
	return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
	static unsigned int addr=0;
	return (addr++)%(4*1024);
}

unsigned int memGen5()
{
	static unsigned int addr=0;
	return (addr++)%(1024*64);
}

unsigned int memGen6()
{
	static unsigned int addr=0;
	return (addr+=32)%(64*4*1024);
}

// cache simulator function
cacheResType cacheSim(setAssociativeCache &cache, unsigned int address)
{
	// searches for address in cache first
	bool flag = cache.search(address);
	if(!flag){
		// if cache not there, then we insert it into the cache, we return miss regardless of conflict miss or cold start
		cache.insert(address);
		return MISS;
	}
	return HIT;
}


char *msg[2] = {"Miss","Hit"};

#define		NO_OF_Iterations	1000000	

int main()
{
	unsigned int hit = 0;
	unsigned int miss = 0;
	cacheResType r;
	
	unsigned int addr;
	unsigned int lineSize = 128, ways = 4; // vary those to generate the needed data
	cout << "Set Associative Cache Simulator\n";
	setAssociativeCache cache(lineSize, ways);
	for(int inst=0;inst<NO_OF_Iterations;inst++)
	{
		addr = memGen4();
		r = cacheSim(cache, addr);
		if(r == HIT) hit++;
		else miss++;
		cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\n";
		 
	}
	cout<<"This is the output using memGen4, line size = 128, and number of ways = 4"<<endl;
	cout << dec << "Number of hits = " << hit << endl;
	cout << dec << "Number of misses = "  << miss << endl;
	cout << dec << "Hit ratio = " << (100.0*hit/NO_OF_Iterations)<< endl;
	cout << dec << "Miss ratio = " << (100.0*miss/NO_OF_Iterations)<< endl;
}