/*
 * serialization_utils.h
 * ---------------------
 * Purpose: Serializing data to and from MPTM files.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include "../common/misc_util.h"
#include "../common/typedefs.h"
#include <limits>
#ifdef HAS_TYPE_TRAITS
#include <type_traits>
#endif
#include <algorithm>
#include <string.h>

namespace srlztn //SeRiaLiZaTioN
{

typedef std::ostream::off_type Offtype;
typedef Offtype Postype;

typedef uintptr_t	DataSize;	// Data size type.
typedef uintptr_t	RposType;	// Relative position type.
typedef uintptr_t	NumType;	// Entry count type.

const DataSize invalidDatasize = DataSize(-1);

enum 
{
	SNT_PROGRESS =		0x80000000, // = 1 << 31
	SNT_FAILURE =		0x40000000, // = 1 << 30
	SNT_NOTE =			0x20000000, // = 1 << 29
	SNT_WARNING =		0x10000000, // = 1 << 28
	SNT_NONE = 0,

	SNRW_BADGIVEN_STREAM =								1	| SNT_FAILURE,

	// Read failures.
	SNR_BADSTREAM_AFTER_MAPHEADERSEEK =					2	| SNT_FAILURE,
	SNR_STARTBYTE_MISMATCH =							3	| SNT_FAILURE,
	SNR_BADSTREAM_AT_MAP_READ =							4	| SNT_FAILURE,
	SNR_INSUFFICIENT_STREAM_OFFTYPE =					5	| SNT_FAILURE,
	SNR_OBJECTCLASS_IDMISMATCH =						6	| SNT_FAILURE,
	SNR_TOO_MANY_ENTRIES_TO_READ =						7	| SNT_FAILURE,
	SNR_INSUFFICIENT_RPOSTYPE =							8	| SNT_FAILURE,

	// Read notes and warnings.
	SNR_ZEROENTRYCOUNT =								0x80	| SNT_NOTE, // 0x80 == 1 << 7
	SNR_NO_ENTRYIDS_WITH_CUSTOMID_DEFINED =				0x100	| SNT_NOTE,
	SNR_LOADING_OBJECT_WITH_LARGER_VERSION =			0x200	| SNT_NOTE,
	
	// Write failures.
	SNW_INSUFFICIENT_FIXEDSIZE =						(0x10)	| SNT_FAILURE,
	SNW_CHANGING_IDSIZE_WITH_FIXED_IDSIZESETTING =		(0x11)	| SNT_FAILURE,
	SNW_DATASIZETYPE_OVERFLOW =							(0x13)	| SNT_FAILURE,
	SNW_MAX_WRITE_COUNT_REACHED =						(0x14)	| SNT_FAILURE,
	SNW_INSUFFICIENT_DATASIZETYPE =						(0x16)	| SNT_FAILURE
};


void ReadAdaptive12(std::istream& iStrm, uint16& val);
void ReadAdaptive1234(std::istream& iStrm, uint32& val);       
void ReadAdaptive1248(std::istream& iStrm, uint64& val);

void WriteAdaptive12(std::ostream& oStrm, const uint16 num);
void WriteAdaptive1234(std::ostream& oStrm, const uint32 num);
void WriteAdaptive1248(std::ostream& oStrm, const uint64& val);


enum
{
	IdSizeVariable = uint16_max,
	IdSizeMaxFixedSize = (uint8_max >> 1)
};

typedef int32 SsbStatus;


struct ReadEntry
//==============
{
	ReadEntry() : nIdpos(0), rposStart(0), nSize(invalidDatasize), nIdLength(0) {}

	uintptr_t nIdpos;	// Index of id start in ID array.
	RposType rposStart;	// Entry start position.
	DataSize nSize;		// Entry size.
	uint16 nIdLength;	// Length of id.
};


enum Rwf
{
	RwfWMapStartPosEntry,	// Write. True to include data start pos entry to map.
	RwfWMapSizeEntry,		// Write. True to include data size entry to map.
	RwfWMapDescEntry,		// Write. True to include description entry to map.
	RwfWVersionNum,			// Write. True to include version numeric.
	RwfRPartialIdMatch,		// Read. True to allow partial ID match.
	RwfRMapCached,			// Read. True if map has been cached.
	RwfRMapHasId,			// Read. True if map has IDs
	RwfRMapHasStartpos,		// Read. True if map data start pos.
	RwfRMapHasSize,			// Read. True if map has entry size.
	RwfRMapHasDesc,			// Read. True if map has entry description.
	RwfRTwoBytesDescChar,	// Read. True if map description characters are two bytes.
	RwfRHeaderIsRead,		// Read. True when header is read.
	RwfRwHasMap,			// Read/write. True if map exists.
	RwfNumFlags
};


template<class T>
inline void Binarywrite(std::ostream& oStrm, const T& data)
//---------------------------------------------------------
{
	union {
		T t;
		char b[sizeof(T)];
	} conv;
	STATIC_ASSERT(sizeof(conv) == sizeof(T));
	STATIC_ASSERT(sizeof(conv.b) == sizeof(T));
	conv.t = data;
	#ifdef MPT_PLATFORM_BIG_ENDIAN
		std::reverse(conv.b, conv.b+sizeof(T));
	#endif
	oStrm.write(conv.b, sizeof(data));
}

//Write only given number of bytes from the beginning.
template<class T>
inline void Binarywrite(std::ostream& oStrm, const T& data, const std::size_t bytecount)
//--------------------------------------------------------------------------------------
{
	union {
		T t;
		char b[sizeof(T)];
	} conv;
	STATIC_ASSERT(sizeof(conv) == sizeof(T));
	STATIC_ASSERT(sizeof(conv.b) == sizeof(T));
	conv.t = data;
	#ifdef MPT_PLATFORM_BIG_ENDIAN
		std::reverse(conv.b, conv.b+sizeof(T));
	#endif
	oStrm.write(conv.b, std::min(bytecount, sizeof(data)));
}

template <class T>
inline void WriteItem(std::ostream& oStrm, const T& data)
//-------------------------------------------------------
{
	#ifdef HAS_TYPE_TRAITS
		static_assert(std::is_trivial<T>::value == true, "");
	#endif
	Binarywrite(oStrm, data);
}

void WriteItemString(std::ostream& oStrm, const char* const pStr, const size_t nSize);

template <>
inline void WriteItem<std::string>(std::ostream& oStrm, const std::string& str) {WriteItemString(oStrm, str.c_str(), str.length());}

template <>
inline void WriteItem<const char *>(std::ostream& oStrm, const char * const & psz) { WriteItemString(oStrm, psz, strlen(psz));}


template<class T>
inline void Binaryread(std::istream& iStrm, T& data)
//--------------------------------------------------
{
	union {
		T t;
		char b[sizeof(T)];
	} conv;
	STATIC_ASSERT(sizeof(conv) == sizeof(T));
	STATIC_ASSERT(sizeof(conv.b) == sizeof(T));
	iStrm.read(conv.b, sizeof(T));
	#ifdef MPT_PLATFORM_BIG_ENDIAN
		std::reverse(conv.b, conv.b+sizeof(T));
	#endif
	data = conv.t;
}

//Read only given number of bytes to the beginning of data; data bytes are memset to 0 before reading.
template <class T>
inline void Binaryread(std::istream& iStrm, T& data, const Offtype bytecount)
//---------------------------------------------------------------------------
{
	#ifdef HAS_TYPE_TRAITS
		static_assert(std::is_trivial<T>::value == true, "");
	#endif
	union {
		T t;
		char b[sizeof(T)];
	} conv;
	STATIC_ASSERT(sizeof(conv) == sizeof(T));
	STATIC_ASSERT(sizeof(conv.b) == sizeof(T));
	memset(conv.b, 0, sizeof(T));
	iStrm.read(conv.b, (std::min)((size_t)bytecount, sizeof(data)));
	#ifdef MPT_PLATFORM_BIG_ENDIAN
		std::reverse(conv.b, conv.b+sizeof(T));
	#endif
	data = conv.t;
}


template <class T>
inline void ReadItem(std::istream& iStrm, T& data, const DataSize nSize)
//----------------------------------------------------------------------
{
	#ifdef HAS_TYPE_TRAITS
		static_assert(std::is_trivial<T>::value == true, "");
	#endif
	if (nSize == sizeof(T) || nSize == invalidDatasize)
		Binaryread(iStrm, data);
	else
		Binaryread(iStrm, data, nSize);
}

// Read specialization for float. If data size is 8, read double and assign it to given float.
template <>
inline void ReadItem<float>(std::istream& iStrm, float& f, const DataSize nSize)
//------------------------------------------------------------------------------
{
	if (nSize == 8)
	{
		double d;
		Binaryread(iStrm, d);
		f = static_cast<float>(d);
	}
	else
		Binaryread(iStrm, f);
}

// Read specialization for double. If data size is 4, read float and assign it to given double.
template <>
inline void ReadItem<double>(std::istream& iStrm, double& d, const DataSize nSize)
//--------------------------------------------------------------------------------
{
	if (nSize == 4)
	{
		float f;
		Binaryread(iStrm, f);
		d = f;
	}
	else
		Binaryread(iStrm, d);
}

void ReadItemString(std::istream& iStrm, std::string& str, const DataSize);

template <>
inline void ReadItem<std::string>(std::istream& iStrm, std::string& str, const DataSize nSize)
//--------------------------------------------------------------------------------------------
{
	ReadItemString(iStrm, str, nSize);
}



class Ssb
{

protected:

	Ssb();

public:

	// When writing, returns the number of entries written.
	// When reading, returns the number of entries read not including unrecognized entries.
	NumType GetCounter() const {return m_nCounter;}

	void SetFlag(Rwf flag, bool val) {m_Flags.set(flag, val);}
	bool GetFlag(Rwf flag) const {return m_Flags[flag];}

	// Write given string to log if log func is defined.
	void AddToLog(const char *psz);

protected:

	void AddNote(const SsbStatus s, const char* sz);

public:

	SsbStatus m_Status;
	uint32 m_nFixedEntrySize;			// Read/write: If > 0, data entries have given fixed size.

protected:

	Postype m_posStart;					// Read/write: Stream position at the beginning of object.

	uint16 m_nIdbytes;					// Read/Write: Tells map ID entry size in bytes. If size is variable, value is IdSizeVariable.
	NumType m_nCounter;					// Read/write: Keeps count of entries written/read.

	std::bitset<RwfNumFlags> m_Flags;	// Read/write: Various flags.

public:

	static const uint8 s_DefaultFlagbyte = 0;
	static const char s_EntryID[3];

};



class SsbRead
	: public Ssb
{

public:

	enum ReadRv // Read return value.
	{
		EntryRead,
		EntryNotFound
	};
	enum IdMatchStatus
	{
		IdMatch, IdMismatch
	};
	typedef std::vector<ReadEntry>::const_iterator ReadIterator;

	SsbRead(std::istream& iStrm);

	// Call this to begin reading: must be called before other read functions.
	void BeginRead(const char* pId, const size_t nLength, const uint64& nVersion);
	void BeginRead(const char* pszId, const uint64& nVersion) {return BeginRead(pszId, strlen(pszId), nVersion);}

	// After calling BeginRead(), this returns number of entries in the file.
	NumType GetNumEntries() const {return m_nReadEntrycount;}

	// Returns read iterator to the beginning of entries.
	// The behaviour of read iterators is undefined if map doesn't
	// contain entry ids or data begin positions.
	ReadIterator GetReadBegin();

	// Returns read iterator to the end(one past last) of entries.
	ReadIterator GetReadEnd();

	// Compares given id with read entry id 
	IdMatchStatus CompareId(const ReadIterator& iter, const char* pszId) {return CompareId(iter, pszId, strlen(pszId));}
	IdMatchStatus CompareId(const ReadIterator& iter, const char* pId, const size_t nIdSize);

	uint64 GetReadVersion() {return m_nReadVersion;}

	// Read item using default read implementation.
	template <class T>
	ReadRv ReadItem(T& obj, const char* pszId) {return ReadItem(obj, pszId, strlen(pszId), srlztn::ReadItem<T>);}

	template <class T>
	ReadRv ReadItem(T& obj, const char* pId, const size_t nIdSize) {return ReadItem(obj, pId, nIdSize, srlztn::ReadItem<T>);}

	// Read item using given function.
	template <class T, class FuncObj>
	ReadRv ReadItem(T& obj, const char* pId, const size_t nIdSize, FuncObj);

	// Read item using read iterator.
	template <class T>
	ReadRv ReadItem(const ReadIterator& iter, T& obj) {return ReadItem(iter, obj, srlztn::ReadItem<T>);}
	template <class T, class FuncObj>
	ReadRv ReadItem(const ReadIterator& iter, T& obj, FuncObj func);

private:

	// Reads map to cache.
	void CacheMap();

	// Compares ID in file with expected ID.
	void CompareId(std::istream& iStrm, const char* pId, const size_t nLength);

	// Searches for entry with given ID. If found, returns pointer to corresponding entry, else
	// returns nullptr.
	const ReadEntry* Find(const char* pId, const size_t nLength);
	const ReadEntry* Find(const char* pszId) {return Find(pszId, strlen(pszId));}

	// Called after reading an object.
	ReadRv OnReadEntry(const ReadEntry* pE, const char* pId, const size_t nIdSize, const Postype& posReadBegin);

	void AddReadNote(const SsbStatus s);

	// Called after reading entry. pRe is a pointer to associated map entry if exists.
	void AddReadNote(const ReadEntry* const pRe, const NumType nNum);

	void ResetReadstatus();

private:

	std::istream* m_pIstrm;					// Read: Pointer to read stream.

private:

	std::vector<char> m_Idarray;		// Read: Holds entry ids.

	std::vector<ReadEntry> mapData;		// Read: Contains map information.
	uint64 m_nReadVersion;				// Read: Version is placed here when reading.
	RposType m_rposMapBegin;			// Read: If map exists, rpos of map begin, else m_rposEndofHdrData.
	Postype m_posMapEnd;				// Read: If map exists, map end position, else pos of end of hdrData.
	Postype m_posDataBegin;				// Read: Data begin position.
	RposType m_rposEndofHdrData;		// Read: rpos of end of header data.
	NumType m_nReadEntrycount;			// Read: Number of entries.

	NumType m_nNextReadHint;			// Read: Hint where to start looking for the next read entry.

};



class SsbWrite
	: public Ssb
{

public:

	SsbWrite(std::ostream& oStrm);

	// Write header
	void BeginWrite(const char* pId, const size_t nIdSize, const uint64& nVersion);
	void BeginWrite(const char* pszId, const uint64& nVersion) {BeginWrite(pszId, strlen(pszId), nVersion);}

	// Write item using default write implementation.
	template <class T>
	void WriteItem(const T& obj, const char* pszId) {WriteItem(obj, pszId, strlen(pszId), &srlztn::WriteItem<T>);}

	template <class T>
	void WriteItem(const T& obj, const char* pId, const size_t nIdSize) {WriteItem(obj, pId, nIdSize, &srlztn::WriteItem<T>);}

	// Write item using given function.
	template <class T, class FuncObj>
	void WriteItem(const T& obj, const char* pId, const size_t nIdSize, FuncObj);

	// Writes mapping.
	void FinishWrite();

private:

	// Called after writing an item.
	void OnWroteItem(const char* pId, const size_t nIdSize, const Postype& posBeforeWrite);

	void AddWriteNote(const SsbStatus s);
	void AddWriteNote(const char* pId,
		const size_t nIdLength,
		const NumType nEntryNum,
		const DataSize nBytecount,
		const RposType rposStart);

	// Writes mapping item to mapstream.
	void WriteMapItem(const char* pId, 
		const size_t nIdSize,
		const RposType& rposDataStart,
		const DataSize& nDatasize,
		const char* pszDesc);

	void ResetWritestatus() {m_Status = SNT_NONE;}

	void IncrementWriteCounter();

private:

	std::ostream* m_pOstrm;				// Write: Pointer to write stream.

private:

	Postype m_posEntrycount;			// Write: Pos of entrycount field. 
	Postype m_posMapPosField;			// Write: Pos of map position field.
	std::ostringstream m_MapStream;				// Write: Map stream.

};



template<typename T>
struct IdLE
{
	union {
		char b[sizeof(T)];
		T t;
	} conv;
	IdLE(T val)
	{
		conv.t = val;
		#ifdef MPT_PLATFORM_BIG_ENDIAN
			std::reverse(conv.b, conv.b+sizeof(T));
		#endif
	}
	const char* GetChars() const
	{
		return conv.b;
	}
};


template <class T, class FuncObj>
void SsbWrite::WriteItem(const T& obj, const char* pId, const size_t nIdSize, FuncObj Func)
//------------------------------------------------------------------------------------
{
	const Postype pos = m_pOstrm->tellp();
	Func(*m_pOstrm, obj);
	OnWroteItem(pId, nIdSize, pos);
}

template <class T, class FuncObj>
SsbRead::ReadRv SsbRead::ReadItem(T& obj, const char* pId, const size_t nIdSize, FuncObj Func)
//------------------------------------------------------------------------------------
{
	const ReadEntry* pE = Find(pId, nIdSize);
	const Postype pos = m_pIstrm->tellg();
	if (pE != nullptr || GetFlag(RwfRMapHasId) == false)
		Func(*m_pIstrm, obj, (pE) ? (pE->nSize) : invalidDatasize);
	return OnReadEntry(pE, pId, nIdSize, pos);
}


template <class T, class FuncObj>
SsbRead::ReadRv SsbRead::ReadItem(const ReadIterator& iter, T& obj, FuncObj func)
//-----------------------------------------------------------------------
{
	m_pIstrm->clear();
	if (iter->rposStart != 0)
		m_pIstrm->seekg(m_posStart + Postype(iter->rposStart));
	const Postype pos = m_pIstrm->tellg();
	func(*m_pIstrm, obj, iter->nSize);
	return OnReadEntry(&(*iter), &m_Idarray[iter->nIdpos], iter->nIdLength, pos);
}


inline SsbRead::IdMatchStatus SsbRead::CompareId(const ReadIterator& iter, const char* pId, const size_t nIdSize)
//-------------------------------------------------------------------------------------------------------
{
	if (nIdSize == iter->nIdLength && memcmp(&m_Idarray[iter->nIdpos], pId, iter->nIdLength) == 0)
		return IdMatch;
	else
		return IdMismatch;
}


inline SsbRead::ReadIterator SsbRead::GetReadBegin()
//------------------------------------------
{
	ASSERT(GetFlag(RwfRMapHasId) && (GetFlag(RwfRMapHasStartpos) || GetFlag(RwfRMapHasSize) || m_nFixedEntrySize > 0));
	if (GetFlag(RwfRMapCached) == false)
		CacheMap();
	return mapData.begin();
}


inline SsbRead::ReadIterator SsbRead::GetReadEnd()
//----------------------------------------
{
	if (GetFlag(RwfRMapCached) == false)
		CacheMap();
	return mapData.end();
}


template <class T>
struct ArrayWriter
//================
{
	ArrayWriter(size_t nCount) : m_nCount(nCount) {}
	void operator()(std::ostream& oStrm, const T* pData) {
		for(std::size_t i=0; i<m_nCount; ++i)
		{
			Binarywrite(oStrm, pData[i]);
		}
	}
	size_t m_nCount;
};

template <class T>
struct ArrayReader
//================
{
	ArrayReader(size_t nCount) : m_nCount(nCount) {}
	void operator()(std::istream& iStrm, T* pData, const size_t) {
		for(std::size_t i=0; i<m_nCount; ++i)
		{
			Binaryread(iStrm, pData[i]);
		}
	} 
	size_t m_nCount;
};

template<class SIZETYPE>
bool StringToBinaryStream(std::ostream& oStrm, const std::string& str)
//--------------------------------------------------------------------
{
	if(!oStrm.good()) return true;
	if((std::numeric_limits<SIZETYPE>::max)() < str.size()) return true;
	SIZETYPE size = static_cast<SIZETYPE>(str.size());
	Binarywrite(oStrm, size);
	oStrm.write(str.c_str(), size);
	if(oStrm.good()) return false;
	else return true;
}


template<class SIZETYPE>
bool StringFromBinaryStream(std::istream& iStrm, std::string& str, const SIZETYPE maxSize = (std::numeric_limits<SIZETYPE>::max)())
//---------------------------------------------------------------------------------------------------------------------------------
{
	if(!iStrm.good()) return true;
	SIZETYPE strSize;
	Binaryread(iStrm, strSize);
	if(strSize > maxSize)
		return true;
	str.resize(strSize);
	for(SIZETYPE i = 0; i<strSize; i++)
		iStrm.read(&str[i], 1);
	if(iStrm.good()) return false;
	else return true;
}


} //namespace srlztn.
