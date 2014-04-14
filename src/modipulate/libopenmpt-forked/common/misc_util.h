/*
 * misc_util.h
 * -----------
 * Purpose: Various useful utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <limits>
#include <string>
#include <vector>

#include <cmath>
#include <cstdlib>

#include <string.h>
#include <time.h>

#include "typedefs.h"

#if defined(HAS_TYPE_TRAITS)
#include <type_traits>
#endif

bool ConvertStrToBool(const std::string &str);
signed char ConvertStrToSignedChar(const std::string &str);
unsigned char ConvertStrToUnsignedChar(const std::string &str);
signed short ConvertStrToSignedShort(const std::string &str);
unsigned short ConvertStrToUnsignedShort(const std::string &str);
signed int ConvertStrToSignedInt(const std::string &str);
unsigned int ConvertStrToUnsignedInt(const std::string &str);
signed long ConvertStrToSignedLong(const std::string &str);
unsigned long ConvertStrToUnsignedLong(const std::string &str);
signed long long ConvertStrToSignedLongLong(const std::string &str);
unsigned long long ConvertStrToUnsignedLongLong(const std::string &str);
float ConvertStrToFloat(const std::string &str);
double ConvertStrToDouble(const std::string &str);
long double ConvertStrToLongDouble(const std::string &str);

template<typename T> inline T ConvertStrTo(const std::string &str); // not defined, generates compiler error for non-specialized types
template<> inline bool ConvertStrTo(const std::string &str) { return ConvertStrToBool(str); }
template<> inline signed char ConvertStrTo(const std::string &str) { return ConvertStrToSignedChar(str); }
template<> inline unsigned char ConvertStrTo(const std::string &str) { return ConvertStrToUnsignedChar(str); }
template<> inline signed short ConvertStrTo(const std::string &str) { return ConvertStrToSignedShort(str); }
template<> inline unsigned short ConvertStrTo(const std::string &str) { return ConvertStrToUnsignedShort(str); }
template<> inline signed int ConvertStrTo(const std::string &str) { return ConvertStrToSignedInt(str); }
template<> inline unsigned int ConvertStrTo(const std::string &str) { return ConvertStrToUnsignedInt(str); }
template<> inline signed long ConvertStrTo(const std::string &str) { return ConvertStrToSignedLong(str); }
template<> inline unsigned long ConvertStrTo(const std::string &str) { return ConvertStrToUnsignedLong(str); }
template<> inline signed long long ConvertStrTo(const std::string &str) { return ConvertStrToSignedLongLong(str); }
template<> inline unsigned long long ConvertStrTo(const std::string &str) { return ConvertStrToUnsignedLongLong(str); }
template<> inline float ConvertStrTo(const std::string &str) { return ConvertStrToFloat(str); }
template<> inline double ConvertStrTo(const std::string &str) { return ConvertStrToDouble(str); }
template<> inline long double ConvertStrTo(const std::string &str) { return ConvertStrToLongDouble(str); }

bool ConvertStrToBool(const std::wstring &str);
signed char ConvertStrToSignedChar(const std::wstring &str);
unsigned char ConvertStrToUnsignedChar(const std::wstring &str);
signed short ConvertStrToSignedShort(const std::wstring &str);
unsigned short ConvertStrToUnsignedShort(const std::wstring &str);
signed int ConvertStrToSignedInt(const std::wstring &str);
unsigned int ConvertStrToUnsignedInt(const std::wstring &str);
signed long ConvertStrToSignedLong(const std::wstring &str);
unsigned long ConvertStrToUnsignedLong(const std::wstring &str);
signed long long ConvertStrToSignedLongLong(const std::wstring &str);
unsigned long long ConvertStrToUnsignedLongLong(const std::wstring &str);
float ConvertStrToFloat(const std::wstring &str);
double ConvertStrToDouble(const std::wstring &str);
long double ConvertStrToLongDouble(const std::wstring &str);

template<typename T> inline T ConvertStrTo(const std::wstring &str); // not defined, generates compiler error for non-specialized types
template<> inline bool ConvertStrTo(const std::wstring &str) { return ConvertStrToBool(str); }
template<> inline signed char ConvertStrTo(const std::wstring &str) { return ConvertStrToSignedChar(str); }
template<> inline unsigned char ConvertStrTo(const std::wstring &str) { return ConvertStrToUnsignedChar(str); }
template<> inline signed short ConvertStrTo(const std::wstring &str) { return ConvertStrToSignedShort(str); }
template<> inline unsigned short ConvertStrTo(const std::wstring &str) { return ConvertStrToUnsignedShort(str); }
template<> inline signed int ConvertStrTo(const std::wstring &str) { return ConvertStrToSignedInt(str); }
template<> inline unsigned int ConvertStrTo(const std::wstring &str) { return ConvertStrToUnsignedInt(str); }
template<> inline signed long ConvertStrTo(const std::wstring &str) { return ConvertStrToSignedLong(str); }
template<> inline unsigned long ConvertStrTo(const std::wstring &str) { return ConvertStrToUnsignedLong(str); }
template<> inline signed long long ConvertStrTo(const std::wstring &str) { return ConvertStrToSignedLongLong(str); }
template<> inline unsigned long long ConvertStrTo(const std::wstring &str) { return ConvertStrToUnsignedLongLong(str); }
template<> inline float ConvertStrTo(const std::wstring &str) { return ConvertStrToFloat(str); }
template<> inline double ConvertStrTo(const std::wstring &str) { return ConvertStrToDouble(str); }
template<> inline long double ConvertStrTo(const std::wstring &str) { return ConvertStrToLongDouble(str); }

template<typename T>
inline T ConvertStrTo(const char *str)
{
	if(!str)
	{
		return T();
	}
	return ConvertStrTo<T>(std::string(str));
}

template<typename T>
inline T ConvertStrTo(const wchar_t *str)
{
	if(!str)
	{
		return T();
	}
	return ConvertStrTo<T>(std::wstring(str));
}


namespace mpt { namespace String {

// Combine a vector of values into a string, separated with the given separator.
// No escaping is performed.
template<typename T>
std::string Combine(const std::vector<T> &vals, const std::string &sep=",")
//-------------------------------------------------------------------------
{
	std::string str;
	for(std::size_t i = 0; i < vals.size(); ++i)
	{
		if(i > 0)
		{
			str += sep;
		}
		str += mpt::ToString(vals[i]);
	}
	return str;
}

// Split the given string at separator positions into individual values returned as a vector.
// An empty string results in an empty vector.
// Leading or trailing separators result in a default-constructed element being inserted before or after the other elements.
template<typename T>
std::vector<T> Split(const std::string &str, const std::string &sep=",")
//----------------------------------------------------------------------
{
	std::vector<T> vals;
	std::size_t pos = 0;
	while(str.find(sep, pos) != std::string::npos)
	{
		vals.push_back(ConvertStrTo<int>(str.substr(pos, str.find(sep, pos) - pos)));
		pos = str.find(sep, pos) + sep.length();
	}
	if(!vals.empty() || (str.substr(pos).length() > 0))
	{
		vals.push_back(ConvertStrTo<T>(str.substr(pos)));
	}
	return vals;
}

} } // namespace mpt::String


// Memset given object to zero.
template <class T>
inline void MemsetZero(T &a)
//--------------------------
{
#ifdef HAS_TYPE_TRAITS
	static_assert(std::is_pointer<T>::value == false, "Won't memset pointers.");
	static_assert(std::is_pod<T>::value == true, "Won't memset non-pods.");
#endif
	memset(&a, 0, sizeof(T));
}


// Copy given object to other location.
template <class T>
inline T &MemCopy(T &destination, const T &source)
//------------------------------------------------
{
#ifdef HAS_TYPE_TRAITS
	static_assert(std::is_pointer<T>::value == false, "Won't copy pointers.");
	static_assert(std::is_pod<T>::value == true, "Won't copy non-pods.");
#endif
	return *static_cast<T *>(memcpy(&destination, &source, sizeof(T)));
}


namespace mpt {

// Saturate the value of src to the domain of Tdst
template <typename Tdst, typename Tsrc>
inline Tdst saturate_cast(Tsrc src)
//---------------------------------
{
	// This code tries not only to obviously avoid overflows but also to avoid signed/unsigned comparison warnings and type truncation warnings (which in fact would be safe here) by explicit casting.
	STATIC_ASSERT(std::numeric_limits<Tdst>::is_integer);
	STATIC_ASSERT(std::numeric_limits<Tsrc>::is_integer);
	if(std::numeric_limits<Tdst>::is_signed && std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(src);
		}
		return static_cast<Tdst>(std::max<Tsrc>(std::numeric_limits<Tdst>::min(), std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	} else if(!std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(src);
		}
		return static_cast<Tdst>(std::min<Tsrc>(src, std::numeric_limits<Tdst>::max()));
	} else if(std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(std::min<Tsrc>(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max())));
		}
		return static_cast<Tdst>(std::max<Tsrc>(std::numeric_limits<Tdst>::min(), std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	} else // Tdst unsigned, Tsrc signed
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(std::max<Tsrc>(0, src));
		}
		return static_cast<Tdst>(std::max<Tsrc>(0, std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	}
}

template <typename Tdst>
inline Tdst saturate_cast(double src)
//-----------------------------------
{
	if(src >= std::numeric_limits<Tdst>::max())
	{
		return std::numeric_limits<Tdst>::max();
	}
	if(src <= std::numeric_limits<Tdst>::min())
	{
		return std::numeric_limits<Tdst>::min();
	}
	return static_cast<Tdst>(src);
}

template <typename Tdst>
inline Tdst saturate_cast(float src)
//----------------------------------
{
	if(src >= std::numeric_limits<Tdst>::max())
	{
		return std::numeric_limits<Tdst>::max();
	}
	if(src <= std::numeric_limits<Tdst>::min())
	{
		return std::numeric_limits<Tdst>::min();
	}
	return static_cast<Tdst>(src);
}

} // namespace mpt


// Limits 'val' to given range. If 'val' is less than 'lowerLimit', 'val' is set to value 'lowerLimit'.
// Similarly if 'val' is greater than 'upperLimit', 'val' is set to value 'upperLimit'.
// If 'lowerLimit' > 'upperLimit', 'val' won't be modified.
template<class T, class C>
inline void Limit(T& val, const C lowerLimit, const C upperLimit)
//---------------------------------------------------------------
{
	if(lowerLimit > upperLimit) return;
	if(val < lowerLimit) val = lowerLimit;
	else if(val > upperLimit) val = upperLimit;
}


// Like Limit, but returns value
template<class T, class C>
inline T Clamp(T val, const C lowerLimit, const C upperLimit)
//-----------------------------------------------------------
{
	if(val < lowerLimit) return lowerLimit;
	else if(val > upperLimit) return upperLimit;
	else return val;
}

// Check if val is in [lo,hi] without causing compiler warnings
// if theses checks are always true due to the domain of T.
// GCC does not warn if the type is templated.
template<typename T, typename C>
inline bool IsInRange(T val, C lo, C hi)
//--------------------------------------
{
	return lo <= val && val <= hi;
}

// Like Limit, but with upperlimit only.
template<class T, class C>
inline void LimitMax(T& val, const C upperLimit)
//----------------------------------------------
{
	if(val > upperLimit)
		val = upperLimit;
}


// Like Limit, but returns value
#ifndef CLAMP
#define CLAMP(number, low, high) MIN(high, MAX(low, number))
#endif


// Greatest Common Divisor.
template <class T>
T gcd(T a, T b)
//-------------
{
	if(a < 0)
		a = -a;
	if(b < 0)
		b = -b;
	do
	{
		if(a == 0)
			return b;
		b %= a;
		if(b == 0)
			return a;
		a %= b;
	} while(true);
}


// Returns sign of a number (-1 for negative numbers, 1 for positive numbers, 0 for 0)
template <class T>
int sgn(T value)
//--------------
{
	return (value > T(0)) - (value < T(0));
}


// Convert a variable-length MIDI integer <value> to a normal integer <result>.
// Function returns how many bytes have been read.
template <class TIn, class TOut>
size_t ConvertMIDI2Int(TOut &result, TIn value)
//---------------------------------------------
{
	return ConvertMIDI2Int(result, (uint8 *)(&value), sizeof(TIn));
}


// Convert a variable-length MIDI integer held in the byte buffer <value> to a normal integer <result>.
// maxLength bytes are read from the byte buffer at max.
// Function returns how many bytes have been read.
// TODO This should report overflow errors if TOut is too small!
template <class TOut>
size_t ConvertMIDI2Int(TOut &result, uint8 *value, size_t maxLength)
//------------------------------------------------------------------
{
	static_assert(std::numeric_limits<TOut>::is_integer == true, "Output type is a not an integer");

	if(maxLength <= 0)
	{
		result = 0;
		return 0;
	}
	size_t bytesUsed = 0;
	result = 0;
	uint8 b;
	do
	{
		b = *value;
		result <<= 7;
		result |= (b & 0x7F);
		value++;
	} while (++bytesUsed < maxLength && (b & 0x80) != 0);
	return bytesUsed;
}


// Convert an integer <value> to a variable-length MIDI integer, held in the byte buffer <result>.
// maxLength bytes are written to the byte buffer at max.
// Function returns how many bytes have been written.
template <class TIn>
size_t ConvertInt2MIDI(uint8 *result, size_t maxLength, TIn value)
//----------------------------------------------------------------
{
	static_assert(std::numeric_limits<TIn>::is_integer == true, "Input type is a not an integer");

	if(maxLength <= 0)
	{
		*result = 0;
		return 0;
	}
	size_t bytesUsed = 0;
	do
	{
		*result = (value & 0x7F);
		value >>= 7;
		if(value != 0)
		{
			*result |= 0x80;
		}
		result++;
	} while (++bytesUsed < maxLength && value != 0);
	return bytesUsed;
}


namespace Util
{

	time_t MakeGmTime(tm *timeUtc);

	// Minimum of 3 values
	template <class T> inline const T& Min(const T& a, const T& b, const T& c) {return std::min(std::min(a, b), c);}

	// Returns maximum value of given integer type.
	template <class T> inline T MaxValueOfType(const T&) {static_assert(std::numeric_limits<T>::is_integer == true, "Only integer types are allowed."); return (std::numeric_limits<T>::max)();}

	/// Returns value rounded to nearest integer.
#if (MPT_COMPILER_MSVC && MPT_MSVC_BEFORE(2012,0)) || defined(ANDROID)
	// revisit this with vs2012
	inline double Round(const double& val) {if(val >= 0.0) return std::floor(val + 0.5); else return std::ceil(val - 0.5);}
	inline float Round(const float& val) {if(val >= 0.0f) return std::floor(val + 0.5f); else return std::ceil(val - 0.5f);}
#else
	inline double Round(const double& val) { return std::round(val); }
	inline float Round(const float& val) { return std::round(val); }
#endif

	/// Rounds given double value to nearest integer value of type T.
	template <class T> inline T Round(const double& val)
	{
		static_assert(std::numeric_limits<T>::is_integer == true, "Type is a not an integer");
		const double valRounded = Round(val);
		ASSERT(valRounded >= (std::numeric_limits<T>::min)() && valRounded <= (std::numeric_limits<T>::max)());
		const T intval = static_cast<T>(valRounded);
		return intval;
	}

	template <class T> inline T Round(const float& val)
	{
		static_assert(std::numeric_limits<T>::is_integer == true, "Type is a not an integer");
		const float valRounded = Round(val);
		ASSERT(valRounded >= (std::numeric_limits<T>::min)() && valRounded <= (std::numeric_limits<T>::max)());
		const T intval = static_cast<T>(valRounded);
		return intval;
	}

	template<typename T>
	T Weight(T x)
	{
		STATIC_ASSERT(std::numeric_limits<T>::is_integer);
		T c;
		for(c = 0; x; x >>= 1)
		{
			c += x & 1;
		}
		return c;
	}
	
}

namespace Util {

	inline int32 muldiv(int32 a, int32 b, int32 c)
	{
		return static_cast<int32>( ( static_cast<int64>(a) * b ) / c );
	}

	inline int32 muldivr(int32 a, int32 b, int32 c)
	{
		return static_cast<int32>( ( static_cast<int64>(a) * b + ( c / 2 ) ) / c );
	}

	// Do not use overloading because catching unsigned version by accident results in slower X86 code.
	inline uint32 muldiv_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return static_cast<uint32>( ( static_cast<uint64>(a) * b ) / c );
	}
	inline uint32 muldivr_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return static_cast<uint32>( ( static_cast<uint64>(a) * b + ( c / 2 ) ) / c );
	}

	inline int32 muldivrfloor(int64 a, uint32 b, uint32 c)
	{
		a *= b;
		a += c / 2;
		return (a >= 0) ? mpt::saturate_cast<int32>(a / c) : mpt::saturate_cast<int32>((a - (c - 1)) / c);
	}

	template<typename T, std::size_t n>
	class fixed_size_queue
	{
	private:
		T buffer[n+1];
		std::size_t read_position;
		std::size_t write_position;
	public:
		fixed_size_queue() : read_position(0), write_position(0)
		{
			return;
		}
		void clear()
		{
			read_position = 0;
			write_position = 0;
		}
		std::size_t read_size() const
		{
			if ( write_position > read_position )
			{
				return write_position - read_position;
			} else if ( write_position < read_position )
			{
				return write_position - read_position + n + 1;
			} else
			{
				return 0;
			}
		}
		std::size_t write_size() const
		{
			if ( write_position > read_position )
			{
				return read_position - write_position + n;
			} else if ( write_position < read_position )
			{
				return read_position - write_position - 1;
			} else
			{
				return n;
			}
		}
		bool push( const T & v )
		{
			if ( !write_size() )
			{
				return false;
			}
			buffer[write_position] = v;
			write_position = ( write_position + 1 ) % ( n + 1 );
			return true;
		}
		bool pop() {
			if ( !read_size() )
			{
				return false;
			}
			read_position = ( read_position + 1 ) % ( n + 1 );
			return true;
		}
		T peek() {
			if ( !read_size() )
			{
				return T();
			}
			return buffer[read_position];
		}
		const T * peek_p()
		{
			if ( !read_size() )
			{
				return nullptr;
			}
			return &(buffer[read_position]);
		}
		const T * peek_next_p()
		{
			if ( read_size() < 2 )
			{
				return nullptr;
			}
			return &(buffer[(read_position+1)%(n+1)]);
		}
	};

} // namespace Util


#ifdef MODPLUG_TRACKER

namespace Util
{

// RAII wrapper around timeBeginPeriod/timeEndPeriod/timeGetTime (on Windows).
// This clock is monotonic, even across changing its resolution.
// This is needed to synchronize time in Steinberg APIs (ASIO and VST).
class MultimediaClock
{
private:
	UINT m_CurrentPeriod;
private:
	void Init();
	void SetPeriod(uint32 ms);
	void Cleanup();
public:
	MultimediaClock();
	MultimediaClock(uint32 ms);
	~MultimediaClock();
public:
	// Sets the desired resolution in milliseconds, returns the obtained resolution in milliseconds.
	// A parameter of 0 causes the resolution to be reset to system defaults.
	// A return value of 0 means the resolution is unknown, but timestamps will still be valid.
	uint32 SetResolution(uint32 ms);
	// Returns obtained resolution in milliseconds.
	// A return value of 0 means the resolution is unknown, but timestamps will still be valid.
	uint32 GetResolution() const; 
	// Returns current instantaneous timestamp in milliseconds.
	// The epoch (offset) of the timestamps is undefined but constant until the next system reboot.
	// The resolution is the value returned from GetResolution().
	uint32 Now() const;
	// Returns current instantaneous timestamp in nanoseconds.
	// The epoch (offset) of the timestamps is undefined but constant until the next system reboot.
	// The resolution is the value returned from GetResolution() in milliseconds.
	uint64 NowNanoseconds() const;
};

} // namespace Util

#endif

#ifdef ENABLE_ASM
#define PROCSUPPORT_MMX        0x00001 // Processor supports MMX instructions
#define PROCSUPPORT_SSE        0x00010 // Processor supports SSE instructions
#define PROCSUPPORT_SSE2       0x00020 // Processor supports SSE2 instructions
#define PROCSUPPORT_SSE3       0x00040 // Processor supports SSE3 instructions
#define PROCSUPPORT_AMD_MMXEXT 0x10000 // Processor supports AMD MMX extensions
#define PROCSUPPORT_AMD_3DNOW  0x20000 // Processor supports AMD 3DNow! instructions
#define PROCSUPPORT_AMD_3DNOW2 0x40000 // Processor supports AMD 3DNow!2 instructions
extern uint32 ProcSupport;
void InitProcSupport();
static inline uint32 GetProcSupport()
{
	return ProcSupport;
}
#endif // ENABLE_ASM


#ifdef MODPLUG_TRACKER

namespace Util
{
	std::wstring CLSIDToString(CLSID clsid);
	bool StringToCLSID(const std::wstring &str, CLSID &clsid);
	bool IsCLSID(const std::wstring &str);
} // namespace Util

#endif // MODPLUG_TRACKER

namespace Util
{

std::wstring BinToHex(const std::vector<char> &src);
std::vector<char> HexToBin(const std::wstring &src);

} // namespace Util

namespace mpt
{

#if defined(WIN32)

namespace Windows
{

enum WinNTVersion
{
	VerWin2000  = 0x0500,
	VerWinXP    = 0x0501,
	VerWinXPSP2 = 0x0502,
	VerWinVista = 0x0600,
	VerWin7     = 0x0601,
	VerWin8     = 0x0602,
};

// returns version in the form of the _WIN32_WINNT_* macros or mpt::Windows::WinNTVersion
static inline uint32 GetWinNTVersion()
//------------------------------------
{
	OSVERSIONINFO versioninfo;
	MemsetZero(versioninfo);
	versioninfo.dwOSVersionInfoSize = sizeof(versioninfo);
	GetVersionEx(&versioninfo);
	return ((uint32)mpt::saturate_cast<uint8>(versioninfo.dwMajorVersion) << 8) | (uint32)mpt::saturate_cast<uint8>(versioninfo.dwMinorVersion);
}

} // namespace Windows

#endif // WIN32

} // namespace mpt
