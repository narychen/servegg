#include "util.h"
#include <sstream>
using namespace std;

CSLog g_iminfo = CSLog("INFO");
CSLog g_imdebug = CSLog("DEBUG");
CSLog g_imwarn = CSLog("WARN");
CSLog g_imerror = CSLog("ERROR");
CSLog g_imtrace = CSLog("TRACE");

CRefObject::CRefObject()
{
	m_lock = NULL;
	m_refCount = 1;
}

CRefObject::~CRefObject()
{

}

void CRefObject::AddRef()
{
	if (m_lock)
	{
		m_lock->lock();
		m_refCount++;
		m_lock->unlock();
	}
	else
	{
		m_refCount++;
	}
}

void CRefObject::ReleaseRef()
{
	if (m_lock)
	{
		m_lock->lock();
		m_refCount--;
		if (m_refCount == 0)
		{
			delete this;
			return;
		}
		m_lock->unlock();
	}
	else
	{
		m_refCount--;
		if (m_refCount == 0)
			delete this;
	}
}

uint64_t get_tick_count()
{
#ifdef _WIN32
	LARGE_INTEGER liCounter; 
	LARGE_INTEGER liCurrent;

	if (!QueryPerformanceFrequency(&liCounter))
		return GetTickCount();

	QueryPerformanceCounter(&liCurrent);
	return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
	struct timeval tval;
	uint64_t ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond)
{
#ifdef _WIN32
	Sleep(millisecond);
#else
	usleep(millisecond * 1000);
#endif
}

CStrExplode::CStrExplode(char* str, char seperator)
{
	m_item_cnt = 1;
	char* pos = str;
	while (*pos) {
		if (*pos == seperator) {
			m_item_cnt++;
		}

		pos++;
	}

	m_item_list = new char* [m_item_cnt];

	int idx = 0;
	char* start = pos = str;
	while (*pos) {
		if ( pos != start && *pos == seperator) {
			uint32_t len = pos - start;
			m_item_list[idx] = new char [len + 1];
			strncpy(m_item_list[idx], start, len);
			m_item_list[idx][len]  = '\0';
			idx++;

			start = pos + 1;
		}

		pos++;
	}

	uint32_t len = pos - start;
    if(len != 0)
    {
        m_item_list[idx] = new char [len + 1];
        strncpy(m_item_list[idx], start, len);
        m_item_list[idx][len]  = '\0';
    }
}

CStrExplode::~CStrExplode()
{
	for (uint32_t i = 0; i < m_item_cnt; i++) {
		delete [] m_item_list[i];
	}

	delete [] m_item_list;
}

char* replaceStr(char* pSrc, char oldChar, char newChar)
{
    if(NULL == pSrc)
    {
        return NULL;
    }
    
    char *pHead = pSrc;
    while (*pHead != '\0') {
        if(*pHead == oldChar)
        {
            *pHead = newChar;
        }
        ++pHead;
    }
    return pSrc;
}

string int2string(uint32_t user_id)
{
    stringstream ss;
    ss << user_id;
    return ss.str();
}

uint32_t string2int(const string& value)
{
    return (uint32_t)atoi(value.c_str());
}

// 由于被替换的内容可能包含?号，所以需要更新开始搜寻的位置信息来避免替换刚刚插入的?号
void replace_mark(string& str, string& new_value, uint32_t& begin_pos)
{
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) {
        return;
    }
    
    string prime_new_value = "'"+ new_value + "'";
    str.replace(pos, 1, prime_new_value);
    
    begin_pos = pos + prime_new_value.size();
}

void replace_mark(string& str, uint32_t new_value, uint32_t& begin_pos)
{
    stringstream ss;
    ss << new_value;
    
    string str_value = ss.str();
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) {
        return;
    }
    
    str.replace(pos, 1, str_value);
    begin_pos = pos + str_value.size();
}


void writePid()
{
	uint32_t curPid;
#ifdef _WIN32
	curPid = (uint32_t) GetCurrentProcess();
#else
	curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
    log("Pid = %s", szPid);
}

inline unsigned char toHex(const unsigned char &x)
{
    return x > 9 ? x -10 + 'A': x + '0';
}

inline unsigned char fromHex(const unsigned char &x)
{
    return isdigit(x) ? x-'0' : x-'A'+10;
}

string URLEncode(const string &sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        unsigned char buf[4];
        memset( buf, 0, 4 );
        if( isalnum( (unsigned char)sIn[ix] ) )
        {
            buf[0] = sIn[ix];
        }
        //else if ( isspace( (unsigned char)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
        //{
        //    buf[0] = '+';
        //}
        else
        {
            buf[0] = '%';
            buf[1] = toHex( (unsigned char)sIn[ix] >> 4 );
            buf[2] = toHex( (unsigned char)sIn[ix] % 16);
        }
        sOut += (char *)buf;
    }
    return sOut;
}

string URLDecode(const string &sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        unsigned char ch = 0;
        if(sIn[ix]=='%')
        {
            ch = (fromHex(sIn[ix+1])<<4);
            ch |= fromHex(sIn[ix+2]);
            ix += 2;
        }
        else if(sIn[ix] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
    return sOut;
}


int64_t get_file_size(const char *path)
{
    int64_t filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

const char*  memfind(const char *src_str,size_t src_len, const char *sub_str, size_t sub_len, bool flag)
{
    if(NULL == src_str || NULL == sub_str || src_len <= 0)
    {
        return NULL;
    }
    if(src_len < sub_len)
    {
        return NULL;
    }
    const char *p;
    if (sub_len == 0)
        sub_len = strlen(sub_str);
    if(src_len == sub_len)
    {
        if(0 == (memcmp(src_str, sub_str, src_len)))
        {
            return src_str;
        }
        else
        {
            return NULL;
        }
    }
    if(flag)
    {
        for (size_t i = 0; i < src_len - sub_len; i++)
        {
            p = src_str + i;
            if(0 == memcmp(p, sub_str, sub_len))
                return p;
        }
    }
    else
    {
        for ( int i = (src_len - sub_len) ; i >= 0;i--  )
        {
            p = src_str + i;
            if ( 0 == memcmp(  p,sub_str,sub_len ) )
                return p;
            
        }
    }
    return NULL;
}

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void print_stacktrace(FILE *out, unsigned int max_frames)
{
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
	    fprintf(out, "  <empty, possibly corrupt>\n");
	    return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
	    char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

	    // find parentheses and +address offset surrounding the mangled name:
	    // ./module(function+0x15c) [0x8048a6d]
	    for (char *p = symbollist[i]; *p; ++p)
	    {
	        if (*p == '(')
	        	begin_name = p;
	        else if (*p == '+')
	        	begin_offset = p;
	        else if (*p == ')' && begin_offset) {
	        	end_offset = p;
	        	break;
	        }
	    }
    
	    if (begin_name && begin_offset && end_offset
	        && begin_name < begin_offset)
	    {
	        *begin_name++ = '\0';
	        *begin_offset++ = '\0';
	        *end_offset = '\0';
    
	        // mangled name is now in [begin_name, begin_offset) and caller
	        // offset in [begin_offset, end_offset). now apply
	        // __cxa_demangle():
    
	        int status;
	        char* ret = abi::__cxa_demangle(begin_name,
	    				    funcname, &funcnamesize, &status);
	        if (status == 0) {
	        	funcname = ret; // use possibly realloc()-ed string
	        	fprintf(out, "  %s : %s+%s\n",
	      		symbollist[i], funcname, begin_offset);
	        }
	        else {
	    	// demangling failed. Output function name as a C function with
	    	// no arguments.
	        	fprintf(out, "  %s : %s()+%s\n",
	    	  	symbollist[i], begin_name, begin_offset);
	        }
	    }
	    else
	    {
	        // couldn't parse the line? print the whole line.
	        fprintf(out, "  %s\n", symbollist[i]);
	    }
    }

    free(funcname);
    free(symbollist);
}
