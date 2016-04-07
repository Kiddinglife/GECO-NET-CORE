#ifndef __INCLUDE_GECO_BITS_STREAM_H
#define __INCLUDE_GECO_BITS_STREAM_H

#if defined(_WIN32)
#include "geco-wins-includes.h"
#endif

#include <cstdio>
#include <stdlib.h>
#include <memory>
#include <cfloat>
#include <cstring>
#include <cmath>
#include <cassert>

#include "geco-export.h"
#include "geco-namesapces.h"
#include "geco-malloc-interface.h"
#include "geco-sock-includes.h"
#include "geco-net-config.h"
#include "geco-basic-type.h"
#include "geco-net-type.h"
//#include "JackieString.h"

// MSWin uses _copysign, others use copysign...
#ifndef _WIN32
#define _copysign copysign
#endif

#define unsigned_integral true
#define signed_integral false
#define geco_debug debug

using namespace geco::ultils;

GECO_NET_BEGIN_NSPACE

class UInt24;
class GecoString;
class GecoWString;

//! This class allows you to write and read native types as a string of bits.  
//! the value of @mWritePosBits always reprsents
//! the position where a bit is going to be written(not been written yet)
//! the value of @curr_readable_pos always reprsents 
//! the position where a bit is going to be read(not been read yet)
//! both of them will start to cout at index of 0, 
//! so mWritePosBits = 2 means the first 2 bits (index 0 and 1) has been written,
//! the third bit (index 2) is being written (not been written yet)
//! so curr_readable_pos = 2 means the first 2 bits (index 0 and 1) has been read,
//! the third bit (index 2) is being read (not been read yet)
//! |                  8 bits               |                  8 bits                |
//!+++++++++++++++++++++++++++++++++++
//! | 0 |  1 | 2 | 3 |  4 | 5 |  6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |   bit index
//!+++++++++++++++++++++++++++++++++++
//! | 0 |  0 | 0 | 1 |  0 | 0 |  0 | 0 | 1 | 0 | 0  | 0 | 0  |  0 |  0 | 0  |  bit in memory
//!+++++++++++++++++++++++++++++++++++
//!
//! Assume given @mWritePosBits = 12, @curr_readable_pos = 2,
//! base on draw above,
//! all the unwritten bits are 4 bits at index 12 to 15,
//! all the written bits are 12 bits at index 0 - 11
//! all the unread bits are 10 bits (0100001000, index 2 to 11),
//!
//! Based on above, we can calculate:
//! index of byte that @curr_readable_pos points to is:
//! 0 = @curr_readable_pos >> 3 = 2/8 = index 0, data[0]
//! index of byte that @mWritePosBits points to is:
//! 1 = @mWritePosBits >> 3 = 12/8 = index 1, data[1]
//!
//! offset of byte boundary behind @curr_readable_pos is:
//! curr_readable_pos mods 8 =  curr_readable_pos & 7 = 2 &7 = 2 bits (00 at index 0,1)
//! offset of byte boundary behind mWritePosBits is:
//! mWritePosBits mod 8 = mWritePosBits & 7 = 12 &7 = 4 bits (1000 at index 8,9,10,11)
//!
//! BITS_TO_BYTES for mWritePosBits (bit at index 12 is exclusive) is:
//! (12+7) >> 3 = 19/8 = 2 ( also is the number of written bytes)
//! BITS_TO_BYTES(8[bit at index 8 is exclusive ])  = 
//! (8+7)>>3 = 15/8 = 1 ( also is the number of written bytes)
class GECO_EXPORT GecoBitStream
{
    private:
    typedef UInt32 BitSize;
    typedef UInt32 ByteSize;

    private:
    BitSize allocated_bits_size_;
    BitSize writable_bit_pos_;
    BitSize readable_bit_pos_;
    UInt8 *data_;

    //! true if @data is pointint to heap-memory pointer, 
    //! false if it is stack-memory  pointer
    bool can_free_;

    //! true if writting not allowed in which case all write functions will not work
    //! false if writting is allowed mainly used for reading receive_params
    bool is_read_only_;
    UInt8 statck_buffer_[GECO_STREAM_STACK_ALLOC_BYTES];

    public:
    GECO_STATIC_FACTORY_DELC(GecoBitStream);

    //! @param [in] [ BitSize initialBytesAllocate]:
    //! the number of bytes to pre-allocate.
    //! @Remarks:
    //! Create the JackieBits, with some number of bytes to immediately
    //! allocate. There is no benefit to calling this, unless you know exactly
    //! how many bytes you need and it is greater than 256.
    GecoBitStream(const BitSize initialBytesAllocate);

    //! @brief  Initialize by setting the @data to a predefined pointer.
    //! @access  public  
    //! @param [in] [UInt8 * src]  
    //! @param [in] [const  ByteSize len]  unit of byte
    //! @param [in] [bool copy]  
    //! true to make an deep copy of the @src . 
    //! false to just save a pointer to the @src.
    //! @remarks
    //! 99% of the time you will use this function to read Packet:;data, 
    //! in which case you should write something as follows:
    //! JackieStream js(packet->data, packet->length, false);
    GecoBitStream(UInt8* src, const ByteSize len, bool copy = false);

    //! DEFAULT CTOR
    GecoBitStream();

    //! realloc and free are more efficient than delete and new  
    //! because it will not call ctor and dtor
    ~GecoBitStream();

    //! Getters and Setters
    inline BitSize writable_bit_position() const
    {
        return writable_bit_pos_;
    }
    inline BitSize writable_byte_position() const
    {
        return BITS_TO_BYTES(writable_bit_pos_);
    }
    inline BitSize readable_bit_position() const
    {
        return readable_bit_pos_;
    }
    inline UInt8* UInt8Data() const
    {
        return data_;
    }
    inline Int8* Int8Data() const
    {
        return (Int8*)data_;
    }
    inline void Int8Data(Int8* val)
    {
        data_ = (UInt8*)val;
        is_read_only_ = true;
    }
    inline void UInt8Data(UInt8* val)
    {
        data_ = val;
        is_read_only_ = true;
    }
    inline void writable_byte_position(BitSize val)
    {
        writable_bit_pos_ = val;
    }
    inline void readable_bit_position(BitSize val)
    {
        readable_bit_pos_ = val;
    }
    inline void allocated_bits_size(BitSize val)
    {
        allocated_bits_size_ = val;
    }

    //! @brief  Resets stream for reuse.
    //! @Access  public  
    //! @Notice
    //! Do NOT reallocate memory because JackieStream is used
    //! to serialize/deserialize a buffer. Reallocation is a dangerous 
    //! operation (may result in leaks).
    inline void Reset(void)
    {
        writable_bit_pos_ = readable_bit_pos_ = 0;
    }

    //!@brief Sets the read pointer back to the beginning of your data.
    //! @access public
    //! @author mengdi[Jackie]
    inline void ResetReadableBitPosition(void)
    {
        readable_bit_pos_ = 0;
    }

    //! @brief Sets the write pointer back to the beginning of your data.
    //! @access public
    inline void ResetWritableBitPosition(void)
    {
        writable_bit_pos_ = 0;
    }

    //! @brief this is good to call when you are done with reading the stream to make
    //! sure you didn't leave any data left over void
    //! should hit if reads didn't match writes
    //! @access public
    inline void AssertStreamEmpty(void)
    {
        assert(readable_bit_pos_ == writable_bit_pos_);
    }

    //! @brief payload are actually the remaining readable bits.
    //! @access public 
    inline BitSize GetPayLoadBits(void) const
    {
        return writable_bit_pos_ - readable_bit_pos_;
    }

    //!@brief the number of bytes needed to hold all the written bits.
    //! @access public 
    //!@notice
    //! particial byte is also accounted and the bit at index of mWritePosBits is exclusive.
    //! if mWritingPosBits =12, will need 2 bytes to hold 12 written bits (6 bits unused),
    //! if mWritingPosBits = 8, will need 1 byte to hold 8 written bits (0 bits unused).
    inline ByteSize GetWrittenBytesSize(void) const
    {
        return BITS_TO_BYTES(writable_bit_pos_);
    }

    //! @brief get the number of written bits
    //! will return same value to that of WritePosBits()
    //! @access public
    //! @author mengdi[Jackie]
    inline BitSize GetWrittenBitsSize(void) const
    {
        return writable_bit_pos_;
    }

    //! @access public 
    //! @returns void
    //! @param [in] void
    //! @brief align the next read to a byte boundary.  
    //! @notice
    //! this can be used to 'waste' bits to byte align for efficiency reasons It
    //! can also be used to force coalesced bitstreams to start on byte
    //! boundaries so so WriteAlignedBits and ReadAlignedBits both
    //! calculate the same offset when aligning.
    inline void AlignReadableBitPosition(void)
    {
        // 8 - ( (8-1)&7 +1 ) = 0 no change which is right
        // 8 - ( (7-1)&7 +1 ) = 1, then 7+1 = 8 which is also right
        readable_bit_pos_ += 8 - (((readable_bit_pos_ - 1) & 7) + 1);
    }

    //! @method Read
    //! @access public 
    //! @returns void
    //! @param [in] Int8 * output 
    //! The result byte array. It should be larger than @em numberOfBytes.
    //! @param [in] const unsigned int numberOfBytes  The number of byte to read
    //! @brief Read an array of raw data or casted stream of byte.
    //! @notice The array is raw data. 
    //! @warnning no automatic endian conversion with this function AND read bit pos is not aligned
    void ReadUChars(UInt8* output, const unsigned int numberOfBytes)
    {
        ReadBits(output, BYTES_TO_BITS(numberOfBytes));
    }
    void ReadChars(Int8* output, const unsigned int numberOfBytes)
    {
        ReadBits((UInt8*)output, BYTES_TO_BITS(numberOfBytes));
    }

    //! @func   ReadBits
    //! @brief   Read numbers of bit into dest array
    //! @access public
    //! @param [out] [unsigned UInt8 * dest]  The destination array
    //! @param [in] [BitSize bitsRead] The number of bits to read
    //! @param [in] [const bool alignRight]  If true bits will be right aligned
    //! @returns void
    //! @remarks
    //! 1.jackie stream internal data are aligned to the left side of byte boundary.
    //! 2.user data are aligned to the right side of byte boundary.
    //! @notice
    //! 1.use True to read to user data 
    //! 2.use False to read this stream to another stream 
    //! @author mengdi[Jackie]
    void ReadBits(UInt8 *dest, BitSize bitsRead, bool alignRight = true);


    //! @brief Read any integral type from a bitstream.  
    //! Define DO_NOT_SWAP_ENDIAN if you DO NOT need endian swapping.
    inline void ReadUInt8(UInt8 &dest)
    {
        ReadBits(&dest, BYTES_TO_BITS(sizeof(UInt8)), true);
    }
    inline void ReadInt8(Int8 &dest)
    {
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int8)), true);
    }
    inline void ReadUInt16(UInt16 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(UInt16)];
            ReadBits(output, BYTES_TO_BITS(sizeof(UInt16)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(UInt16));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt16)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt16)), true);
#endif
    }
    inline void ReadInt16(Int16 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(Int16)];
            ReadBits(output, BYTES_TO_BITS(sizeof(Int16)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(Int16));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int16)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int16)), true);
#endif
    }
    inline void ReadUInt32(UInt32 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(UInt32)];
            ReadBits(output, BYTES_TO_BITS(sizeof(UInt32)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(UInt32));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt32)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt32)), true);
#endif
    }
    inline void ReadInt32(Int32 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(Int32)];
            ReadBits(output, BYTES_TO_BITS(sizeof(Int32)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(Int32));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int32)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int32)), true);
#endif
    }
    inline void ReadUInt64(UInt64 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(UInt64)];
            ReadBits(output, BYTES_TO_BITS(sizeof(UInt64)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(UInt64));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt64)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt64)), true);
#endif
    }
    inline void ReadInt32(Int64 &dest)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            UInt8 output[sizeof(Int64)];
            ReadBits(output, BYTES_TO_BITS(sizeof(Int64)), true);
            ReverseBytes(output, (UInt8*)&dest, sizeof(Int64));
        }
        else
        {
            ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int64)), true);
        }
#else
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int64)), true);
#endif
    }
    inline void ReadBoolean(bool &dest)
    {
        assert(GetPayLoadBits() >= 1);
        dest = (data_[readable_bit_pos_ >> 3] & (0x80 >> (readable_bit_pos_ & 7))) != 0;
        readable_bit_pos_++;
    }
    inline void ReadInetAddress(InetAddress &dest)
    {
        UInt8 ipVersion;
        ReadUInt8(ipVersion);
        if (ipVersion == 4)
        {
            dest.address.addr4.sin_family = AF_INET;
            // Don't endian swap the address or port
            UInt32 binaryAddress;
            ReadBits((UInt8*)& binaryAddress, BYTES_TO_BITS(sizeof(binaryAddress)), true);
            // Unhide the IP address, done to prevent routers from changing it
            dest.address.addr4.sin_addr.s_addr = ~binaryAddress;
            ReadBits((UInt8*)& dest.address.addr4.sin_port,
                BYTES_TO_BITS(sizeof(dest.address.addr4.sin_port)), true);
            dest.debugPort = ntohs(dest.address.addr4.sin_port);
        }
        else
        {
#if NET_SUPPORT_IPV6==1
            ReadBits((UInt8*)&dest.address.addr6, BYTES_TO_BITS(sizeof(dest.address.addr6)), true);
            dest.debugPort = ntohs(dest.address.addr6.sin6_port);
#endif
        }
    }
    //! @notice will align @mReadPosBIts to byte-boundary internally
    //! @see  AlignReadPosBitsByteBoundary()
    inline void ReadUInt24(UInt24 &dest)
    {
        assert(GetPayLoadBits() >= 24);
        if (!IsBigEndian())
        {
            ReadBits((UInt8 *)&dest.val, 24, false);
            ((UInt8 *)&dest.val)[3] = 0;
        }
        else
        {
            ReadBits((UInt8 *)&dest.val, 24, false);
            ReverseBytes((UInt8 *)&dest.val, 3);
            ((UInt8 *)&dest.val)[0] = 0;
        }
    }
    inline void ReadGUID(JackieGUID &dest)
    {
        return ReadUInt64(dest.g);
    }
    inline void ReadGecoString(GecoString &outTemplateVar)
    {
        //outTemplateVar.Read(this);
    }
    inline void ReadGecoWString(GecoWString &outTemplateVar)
    {
        //return outTemplateVar.Deserialize(this);
    }
    inline void ReadCharStr(char *&varString)
    {
        // GecoString::Read(varString, this);
    }
    inline bool ReadWCharStr(wchar_t *&varString)
    {
        //return GecoWString::Deserialize(varString, this);
    }

    //! @brief Read any integral type from a bitstream.  
    //! @details If the written value differed from the value 
    //! compared against in the write function,
    //! var will be updated.  Otherwise it will retain the current value.
    //! ReadDelta is only valid from a previous call to WriteDelta
    //! @param[in] outTemplateVar The value to read
    template <class IntegralType>
    inline void ReadChangedValue(IntegralType &dest)
    {
        bool dataWritten;
        Read(dataWritten);
        if (dataWritten) Read(dest);
    }

    //! @brief Read a bool from a bitstream.
    //! @param[in] outTemplateVar The value to read
    inline void ReadChangedValue(bool &dest)
    {
        ReadBoolean(dest);
    }

    //! @Brief Assume the input source points to a compressed native type. 
    //! Decompress and read it.
    void ReadIntegerBits(UInt8* dest, const BitSize bits2Read, bool isUnsigned);

    //! @method ReadMini
    //! @access public 
    //! @returns void
    //! @param [in] IntegralType & dest
    //! @brief Read any integral type from a bitstream, 
    //! endian swapping counters internally. default is unsigned (isUnsigned = true)
    //! @notice
    //! For floating point, this is lossy, using 2 bytes for a float and 4 for
    //! a double.  The range must be between -1 and +1.
    //! For non-floating point, this is lossless, but only has benefit if you 
    //! use less than half the bits of the type

    inline void ReadMiniUInt8(UInt8 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt8)), true);
    }
    inline void ReadMiniInt8(Int8 &dest)
    {
        ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int8)), false);
    }
    inline void ReadMiniUInt16(UInt16 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt16)), true);
    }
    inline void ReadMiniInt16(Int16 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(Int16)), false);
    }
    inline void ReadMiniUInt32(UInt32 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt32)), true);
    }
    inline void ReadMiniInt32(Int32 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt32)), false);
    }
    inline void ReadMiniUInt64(UInt64 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt64)), true);
    }
    inline void ReadMiniInt64(Int64 &dest)
    {
        ReadIntegerBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(UInt64)), false);
    }
    inline void ReadMiniBoolean(bool &dest)
    {
        ReadBoolean(dest);
    }

    inline void ReadMiniInetAddress(InetAddress &dest)
    {
        UInt8 ipVersion;
        ReadMiniUInt8(ipVersion);
        if (ipVersion == 4)
        {
            dest.address.addr4.sin_family = AF_INET;
            // Read(var.binaryAddress);
            // Don't endian swap the address or port
            UInt32 binaryAddress;
            ReadMiniUInt32(binaryAddress);
            // Unhide the IP address, done to prevent routers from changing it
            dest.address.addr4.sin_addr.s_addr = ~binaryAddress;
            ReadMiniUInt16(dest.address.addr4.sin_port);
            dest.debugPort = ntohs(dest.address.addr4.sin_port);
        }
        else
        {
#if NET_SUPPORT_IPV6==1
            ReadMiniUInt16(dest.address.addr6);
            dest.debugPort = ntohs(dest.address.addr6.sin6_port);
#endif
        }
    }
    inline void ReadMiniUInt24(UInt24 &dest)
    {
        ReadMiniUInt32(dest.val);
    }
    inline void ReadMiniGUID(JackieGUID &dest)
    {
        ReadMiniUInt64(dest.g);
    }
    inline void ReadMiniBoolean(bool &dest)
    {
        ReadBoolean(dest);
    }
    //! For values between -1 and 1
    inline void ReadMiniFloat(float &dest)
    {
        UInt16 compressedFloat;
        ReadMiniUInt16(compressedFloat);
        dest = ((float)compressedFloat / 32767.5f - 1.0f);
    }
    //! For values between -1 and 1
    inline void ReadMiniDouble(double &dest)
    {
        UInt32 compressedFloat;
        ReadMiniUInt32(compressedFloat);
        dest = ((double)compressedFloat / 2147483648.0 - 1.0);
    }

    //! For strings
    inline void ReadMiniGecoString(GecoString &outTemplateVar)
    {
        // outTemplateVar.ReadMini(this, false);
    }
    inline void ReadMiniGecoWString(GecoWString &outTemplateVar)
    {
        //outTemplateVar.ReadMini(this);
    }
    inline void ReadMiniCharStr(char *outTemplateVar)
    {
        bool use_haffman_coding;
        ReadMiniBoolean(use_haffman_coding);
        if (!use_haffman_coding)
        {
            UInt24 len;
            ReadMiniUInt24(len);

            UInt8 charr;
            for (int i = 0; i < len; ++i)
            {
                ReadMiniUInt8(*((UInt8*)outTemplateVar + i));
            }
        }
        else
        {
            // GecoString::ReadMini(outTemplateVar, this, false);
        }

    }
    inline void ReadMiniWCharStr(wchar_t *&outtemplatevar)
    {
        // GecoWString::deserialize(outtemplatevar, this);
    }
    inline void ReadMiniUChars(unsigned char *outTemplateVar)
    {
        bool use_haffman_coding;
        ReadMiniBoolean(use_haffman_coding);
        if (!use_haffman_coding)
        {
            UInt24 len;
            ReadMiniUInt24(len);

            UInt8 charr;
            for (int i = 0; i < len; ++i)
            {
                ReadMiniUInt8(*(outTemplateVar + i));

            }
        }
        else
        {
            // GecoString::ReadMini(outTemplateVar, this, false);
        }
    }

    inline void ReadMiniChars(char *outTemplateVar)
    {
        bool use_haffman_coding;
        ReadMiniBoolean(use_haffman_coding);
        if (!use_haffman_coding)
        {
            UInt24 len;
            ReadMiniUInt24(len);

            UInt8 charr;
            for (int i = 0; i < len; ++i)
            {
                ReadMiniInt8(*(outTemplateVar + i));

            }
        }
        else
        {
            // GecoString::ReadMini(outTemplateVar, this, false);
        }
    }


    //! @method ReadMiniChanged
    //! @access public 
    //! @returns void
    //! @param [in] templateType & dest
    //! @brief Read any integral type from a bitstream.  
    //! If the written value differed from the value compared against in 
    //! the write function, var will be updated.  Otherwise it will retain the
    //! current value. the current value will be updated.
    //! @notice
    //! For floating point, this is lossy, using 2 bytes for a float and 4 for a 
    //! double.  The range must be between -1 and +1. For non-floating point, 
    //! this is lossless, but only has benefit if you use less than half the bits of the
    //! type.  If you are not using DO_NOT_SWAP_ENDIAN the opposite is true for
    //! types larger than 1 byte
    //! @see
    inline void ReadMiniUInt8Changed(UInt8 &dest)
    {
        bool dataWritten;
        ReadMiniBoolean(dataWritten);
        if (dataWritten)
            ReadMiniUInt8(dest);
    }

    //! @brief Read a bool from a bitstream.
    //! @param[in] outTemplateVar The value to read
    inline void ReadMiniBooleanChanged(bool &dest)
    {
        ReadMiniBoolean(dest);
    }


    void ReadUInt8Range(
        UInt8 &value,
        const UInt8 minimum,
        const UInt8 maximum,
        bool allowOutsideRange = false)
    {
        //! get the high byte bits size
        UInt8 diff = maximum - minimum;
        int requiredBits = BYTES_TO_BITS(sizeof(UInt8)) - GetLeadingZeroSize(diff);
        //ReadIntegerRange(value, minimum, maximum, requiredBits, allowOutsideRange);

        assert(maximum >= minimum);
        if (allowOutsideRange)
        {
            bool isOutsideRange;
            ReadMiniBoolean(isOutsideRange);
            if (isOutsideRange)
            {
                ReadMiniUInt8(value);
                return;
            }
        }

        value = 0;
        ReadBits((UInt8*)&value, requiredBits, true);
        if (IsBigEndian())
        {
            ReverseBytes((UInt8*)&value, sizeof(value));
        }
        value += minimum;
    }

    void ReadInt8Range(
        Int8 &value,
        const Int8 minimum,
        const Int8 maximum,
        bool allowOutsideRange = false)
    {
        //! get the high byte bits size
        Int8 diff = maximum - minimum;
        int requiredBits = BYTES_TO_BITS(sizeof(Int8)) - GetLeadingZeroSize(diff);
        //ReadIntegerRange(value, minimum, maximum, requiredBits, allowOutsideRange);

        assert(maximum >= minimum);
        if (allowOutsideRange)
        {
            bool isOutsideRange;
            ReadMiniBoolean(isOutsideRange);
            if (isOutsideRange)
            {
                ReadMiniInt8(value);
                return;
            }
        }

        value = 0;
        ReadBits((UInt8*)&value, requiredBits, true);
        if (IsBigEndian())
        {
            ReverseBytes((UInt8*)&value, sizeof(value));
        }
        value += minimum;
    }

    void ReadUInt16Range(
        UInt16 &value,
        const UInt16 minimum,
        const UInt16 maximum,
        bool allowOutsideRange = false)
    {
        //! get the high byte bits size
        UInt16 diff = maximum - minimum;
        int requiredBits = BYTES_TO_BITS(sizeof(UInt16)) - GetLeadingZeroSize(diff);
        //ReadIntegerRange(value, minimum, maximum, requiredBits, allowOutsideRange);

        assert(maximum >= minimum);
        if (allowOutsideRange)
        {
            bool isOutsideRange;
            ReadMiniBoolean(isOutsideRange);
            if (isOutsideRange)
            {
                ReadMiniUInt16(value);
                return;
            }
        }

        value = 0;
        ReadBits((UInt8*)&value, requiredBits, true);
        if (IsBigEndian())
        {
            ReverseBytes((UInt8*)&value, sizeof(value));
        }
        value += minimum;
    }
    void ReadUInt16Range(
        Int16 &value,
        const Int16 minimum,
        const Int16 maximum,
        bool allowOutsideRange = false)
    {
        //! get the high byte bits size
        Int16 diff = maximum - minimum;
        int requiredBits = BYTES_TO_BITS(sizeof(Int16)) - GetLeadingZeroSize(diff);
        //ReadIntegerRange(value, minimum, maximum, requiredBits, allowOutsideRange);

        assert(maximum >= minimum);
        if (allowOutsideRange)
        {
            bool isOutsideRange;
            ReadMiniBoolean(isOutsideRange);
            if (isOutsideRange)
            {
                ReadMiniInt16(value);
                return;
            }
        }

        value = 0;
        ReadBits((UInt8*)&value, requiredBits, true);
        if (IsBigEndian())
        {
            ReverseBytes((UInt8*)&value, sizeof(value));
        }
        value += minimum;
    }

    //! @method ReadBitsIntegerRange
    //! @access public 
    //! @returns void
    //! @param [in] templateType & value
    //! @param [in] const templateType minimum
    //! @param [in] const templateType maximum
    //! @param [in] const int requiredBits the value of bits to read
    //! @param [in] bool allowOutsideRange 
    //! if true, we directly read it
    //! @brief
    //! @notice
    //! This is how we write value
    //! Assume@valueBeyondMini's value is 0x000012
    //!------------------> Memory Address
    //!+++++++++++
    //!| 00 | 00 | 00 | 12 |  Big Endian
    //!+++++++++++
    //!+++++++++++
    //!| 12 | 00 | 00 | 00 |  Little Endian 
    //!+++++++++++
    //! so for big endian, we need to reverse byte so that
    //! the high byte of 0x00 that was put in low address can be written correctly
    //! for little endian, we do nothing.
    //! After reverse bytes:
    //!+++++++++++
    //!| 12 | 00 | 00 | 00 |  Big Endian 
    //!+++++++++++
    //!+++++++++++
    //!| 12 | 00 | 00 | 00 |  Little Endian 
    //!+++++++++++
    //! When reading it, we have to reverse it back fro big endian
    //! we do nothing for little endian.
    //! @see
    void ReadIntegerRange(
        UInt8 &value,
        const UInt8 minimum,
        const UInt8 maximum,
        const int requiredBits,
        bool allowOutsideRange)
    {
        assert(maximum >= minimum);

        if (allowOutsideRange)
        {
            bool isOutsideRange;
            ReadMiniBoolean(isOutsideRange);
            if (isOutsideRange)
            {
                ReadMiniUInt8(value);
                return;
            }
        }

        value = 0;
        ReadBits((UInt8*)&value, requiredBits, true);
        if (IsBigEndian())
        {
            ReverseBytes((UInt8*)&value, sizeof(value));
        }
        value += minimum;
    }

    //! @method Read
    //! @access public 
    //! @returns void
    //! @param [in] float & outFloat The float to read
    //! @param [in] float floatMin  Predetermined minimum value of f
    //! @param [in] float floatMax Predetermined maximum value of f
    //! @brief
    //! Read a float into 2 bytes, spanning the range between
    //! @param floatMin and @param floatMax
    //! @notice
    //! @see
    void ReadFloatRange(float &outFloat, float floatMin, float floatMax);

    //! @brief Read bytes, starting at the next aligned byte. 
    //! @details Note that the modulus 8 starting offset of the sequence
    //! must be the same as was used with WriteBits. This will be a problem
    //! with packet coalescence unless you byte align the coalesced packets.
    //! @param[in] dest The byte array larger than @em numberOfBytesRead
    //! @param[in] bytes2Read The number of byte to read from the internal state
    //! @return true if there is enough byte.
    void ReadAlignedBytes(UInt8 *dest, const ByteSize bytes2Read);

    //! @brief Reads what was written by WriteAlignedBytes.
    //! @param[in] inOutByteArray The data
    //! @param[in] maxBytesRead Maximum number of bytes to read
    //! @return true on success, false on failure.
    void ReadAlignedBytes(Int8 *dest, ByteSize &bytes2Read,
        const ByteSize maxBytes2Read);

    //! @method ReadAlignedBytesAlloc
    //! @access public 
    //! @returns void
    //! @param [in] Int8 * * dest  will be deleted if it is not a pointer to 0
    //! @param [in] ByteSize & bytes2Read
    //! @param [in] const ByteSize maxBytes2Read
    //! @brief  Same as ReadAlignedBytesSafe() but allocates the memory
    //! for you using new, rather than assuming it is safe to write to
    void ReadAlignedBytesAlloc(Int8 **dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read);

    // @brief return 1 if the next data read is a 1, 0 if it is a 0
    //!@access public 
    inline UInt32 ReadBit(void)
    {
        UInt32 result = ((data_[readable_bit_pos_ >> 3] & (0x80 >> (readable_bit_pos_ & 7))) != 0) ? 1 : 0;
        readable_bit_pos_++;
        return result;
    }

    //! @access public
    //! @brief Read a normalized 3D vector, using (at most) 4 bytes 
    //! + 3 bits instead of 12-24 bytes.  
    //! @details Will further compress y or z axis aligned vectors.
    //! Accurate to 1/32767.5.
    //! @param[in] x x
    //! @param[in] y y
    //! @param[in] z z
    //! @return void
    //! @notice templateType for this function must be a float or double
    template <class FloatingType>
    void ReadNormVector(FloatingType &x, FloatingType &y, FloatingType &z)
    {
        ReadFloatRange(x, -1.0f, 1.0f);
        ReadFloatRange(y, -1.0f, 1.0f);
        ReadFloatRange(z, -1.0f, 1.0f);
    }

    //! @brief Read 3 floats or doubles, using 10 bytes, 
    //! where those float or doubles comprise a vector.
    //! @details Loses accuracy to about 3/10ths and only saves 2 bytes, 
    //! so only use if accuracy is not important.
    //! @param[in] x x
    //! @param[in] y y
    //! @param[in] z z
    //! @return void
    //! @notice FloatingType for this function must be a float or double
    template <class FloatingType>
    void ReadVector(FloatingType &x, FloatingType &y, FloatingType &z)
    {
        float magnitude;
        Read(magnitude);

        if (magnitude > 0.00001f)
        {
            ReadMini(x);
            ReadMini(y);
            ReadMini(z);
            x *= magnitude;
            y *= magnitude;
            z *= magnitude;
        }
        else
        {
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
    }

    //! @brief Read a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.
    //! @param[in] w w
    //! @param[in] x x
    //! @param[in] y y
    //! @param[in] z z
    //! @return void
    //! @notice FloatingType for this function must be a float or double
    template <class FloatingType>
    bool ReadNormQuat(FloatingType &w, FloatingType &x, FloatingType &y, FloatingType &z)
    {
        bool cwNeg = false, cxNeg = false, cyNeg = false, czNeg = false;
        Read(cwNeg);
        Read(cxNeg);
        Read(cyNeg);
        Read(czNeg);

        UInt16 cx, cy, cz;
        ReadMini(cx);
        ReadMini(cy);
        ReadMini(cz);

        // Calculate w from x,y,z
        x = (FloatingType)(cx / 65535.0);
        y = (FloatingType)(cy / 65535.0);
        z = (FloatingType)(cz / 65535.0);

        if (cxNeg) x = -x;
        if (cyNeg) y = -y;
        if (czNeg) z = -z;

        float difference = 1.0f - x*x - y*y - z*z;
        if (difference < 0.0f) difference = 0.0f;

        w = (FloatingType)(sqrt(difference));
        if (cwNeg) w = -w;
    }

    //! @brief Read an orthogonal matrix from a quaternion, 
    //! reading 3 components of the quaternion in 2 bytes each and
    //! extrapolatig the 4th.
    //! @details Use 6 bytes instead of 36
    //! Lossy, although the result is renormalized
    //! @return true on success, false on failure.
    //!@notice FloatingType for this function must be a float or double
    template <class FloatingType>
    void ReadOrthMatrix(
        FloatingType &m00, FloatingType &m01, FloatingType &m02,
        FloatingType &m10, FloatingType &m11, FloatingType &m12,
        FloatingType &m20, FloatingType &m21, FloatingType &m22)
    {
        float qw, qx, qy, qz;
        ReadNormQuat(qw, qx, qy, qz);

        // Quat to orthogonal rotation matrix
        // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionMatrix/index.htm
        double sqw = (double)qw*(double)qw;
        double sqx = (double)qx*(double)qx;
        double sqy = (double)qy*(double)qy;
        double sqz = (double)qz*(double)qz;
        m00 = (FloatingType)(sqx - sqy - sqz + sqw);// since sqw + sqx + sqy + sqz =1
        m11 = (FloatingType)(-sqx + sqy - sqz + sqw);
        m22 = (FloatingType)(-sqx - sqy + sqz + sqw);

        double tmp1 = (double)qx*(double)qy;
        double tmp2 = (double)qz*(double)qw;
        m10 = (FloatingType)(2.0 * (tmp1 + tmp2));
        m01 = (FloatingType)(2.0 * (tmp1 - tmp2));

        tmp1 = (double)qx*(double)qz;
        tmp2 = (double)qy*(double)qw;
        m20 = (FloatingType)(2.0 * (tmp1 - tmp2));
        m02 = (FloatingType)(2.0 * (tmp1 + tmp2));
        tmp1 = (double)qy*(double)qz;
        tmp2 = (double)qx*(double)qw;
        m21 = (FloatingType)(2.0 * (tmp1 + tmp2));
        m12 = (FloatingType)(2.0 * (tmp1 - tmp2));
    }

    //! @func AppendBitsCouldRealloc 
    //! @brief 
    //! reallocates (if necessary) in preparation of writing @bits2Append
    //! all internal status will not be changed like @mWritePosBits and so on
    //! @access  public  
    //! @notice  
    //! It is caller's reponsibility to ensure 
    //! @param bits2Append > 0 and @param mReadOnly is false
    //! @author mengdi[Jackie]
    void AppendBitsCouldRealloc(const BitSize bits2Append);

    //! @func  WriteBits 
    //! @brief  write @bitsCount number of bits into @input
    //! @access      public  
    //! @param [in] [const UInt8 * src] source array
    //! @param [in] [BitSize bits2Write] the number of bits to write
    //! @param [in] [bool rightAligned] if true particial bits will be right aligned
    //! @returns void
    //! @remarks
    //! 1.jackie stream internal data are aligned to the left side of byte boundary.
    //! 2.user data are aligned to the right side of byte boundary.
    //! @notice
    //! 1.Use true to write user data to jackie stream 
    //! 2.Use False to write this jackie stream internal data to another stream
    //! @Author mengdi[Jackie]
    void WriteBits(const UInt8* src, BitSize bits2Write, bool rightAligned = true);

    //! @func Write
    //! @access  public  
    //! @brief write an array or raw data in bytes.
    //! NOT do endian swapp.
    //! default is right aligned[true]
    //! @author mengdi[Jackie]
    inline void Write(const Int8* src, const ByteSize bytes2Write)
    {
        WriteBits((UInt8*)src, BYTES_TO_BITS(bytes2Write), true);
    }
    inline void Write(const UInt8* src, const ByteSize bytes2Write)
    {
        WriteBits(src, BYTES_TO_BITS(bytes2Write), true);
    }

    //! @brief Write one JackieBits to another.
    //! @param[in] [bits2Write] bits to write
    //! @param[in] [JackieBits] the JackieBits to copy from
    void Write(GecoBitStream *jackieBits, BitSize bits2Write);
    inline void Write(GecoBitStream &jackieBits, BitSize bits2Write)
    {
        Write(&jackieBits, bits2Write);
    }
    inline void Write(GecoBitStream *jackieBits)
    {
        Write(jackieBits, jackieBits->GetPayLoadBits());
    }
    inline void Write(GecoBitStream &jackieBits)
    {
        Write(&jackieBits);
    }

    //! @method WritePtr
    //! @access public 
    //! @returns void
    //! @param [in] IntergralType * src pointing to the value to write
    //! @brief  
    //! write the dereferenced pointer to any integral type to a bitstream.  
    //! Undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
    template <class IntergralType>
    void WritePtr(IntergralType *src)
    {
        if (sizeof(IntergralType) == 1)
            WriteBits((UInt8*)src, BYTES_TO_BITS(sizeof(IntergralType)), true);
        else
        {
#ifndef DO_NOT_SWAP_ENDIAN
            if (DoEndianSwap())
            {
                UInt8 output[sizeof(IntergralType)];
                ReverseBytes((UInt8*)src, output, sizeof(IntergralType));
                WriteBits((UInt8*)output, BYTES_TO_BITS(sizeof(IntergralType)), true);
            }
            else
#endif
                WriteBits((UInt8*)src, BYTES_TO_BITS(sizeof(IntergralType)), true);
        }
    }

    //! @func WriteBitZero 
    //! @access  public  
    //! @notice @mReadOnly must be false
    //! @author mengdi[Jackie]
    inline void WriteBitZero(void)
    {
        assert(is_read_only_ == false);

        //AppendBitsCouldRealloc(1);
        //BitSize shit = 8 - (mWritingPosBits & 7);
        //data[mWritingPosBits >> 3] = ((data[mWritingPosBits >> 3] >> shit) << shit);
        //mWritingPosBits++;

        AppendBitsCouldRealloc(1);
        //! New bytes need to be zeroed
        if ((writable_bit_pos_ & 7) == 0) data_[writable_bit_pos_ >> 3] = 0;
        writable_bit_pos_++;
    }

    //! @func WriteBitOne 
    //! @access  public  
    //! @notice @mReadOnly must be false
    //! @author mengdi[Jackie]
    inline void WriteBitOne(void)
    {
        assert(is_read_only_ == false);
        AppendBitsCouldRealloc(1);

        // Write bit 1
        BitSize shift = writable_bit_pos_ & 7;
        shift == 0 ? data_[writable_bit_pos_ >> 3] = 0x80 :
            data_[writable_bit_pos_ >> 3] |= 0x80 >> shift;
        writable_bit_pos_++;
    }

    //! @func AlignWritePosBits2ByteBoundary 
    //! @brief align @mWritePosBits to a byte boundary.
    //! @access  public  
    //! @notice
    //! this can be used to 'waste' bits to byte align for efficiency reasons It
    //! can also be used to force coalesced bitstreams to start on byte
    //! boundaries so so WriteAlignedBits and ReadAlignedBits both
    //! calculate the same offset when aligning.
    //! @author mengdi[Jackie]
    inline void AlignWritePosBits2ByteBoundary(void)
    {
        writable_bit_pos_ += 8 - (((writable_bit_pos_ - 1) & 7) + 1);
    }

    //! @func WriteAlignedBytes 
    //! @brief  align the bitstream to the byte boundary and 
    //! then write the specified number of bytes.  
    //! @access  public  
    //! @param [in] [const UInt8 * src]  
    //! @param [in] [const ByteSize numberOfBytesWrite]  
    //! @returns [void]
    //! @notice this is faster than WriteBits() but
    //! wastes the bits to do the alignment for @mWritePosBits and
    //! requires you to call ReadAlignedBits() at the corresponding 
    //! read position.
    //! @author mengdi[Jackie]
    void WriteAlignedBytes(const UInt8 *src, const ByteSize numberOfBytesWrite);

    //! @brief Aligns the bitstream, writes inputLength, and writes input. 
    //! @access  public  
    //! @param[in] inByteArray The data
    //! @param[in] inputLength the size of input.
    //! @param[in] maxBytesWrite max bytes to write
    //! @notice Won't write beyond maxBytesWrite
    void WriteAlignedBytes(const UInt8 *src, const ByteSize bytes2Write,
        const ByteSize maxBytes2Write);

    //! @func Write 
    //! @brief write a float into 2 bytes, spanning the range, 
    //! between @param[floatMin] and @param[floatMax]
    //! @access  public  
    //! @param [in] [float src]  value to write into stream
    //! @param [in] [float floatMin] Predetermined mini value of f
    //! @param [in] [float floatMax] Predetermined max value of f
    //! @return bool
    //! @notice 
    //! @author mengdi[Jackie]
    void WriteFloatRange(float src, float floatMin, float floatMax);

    //! @func Write 
    //! @brief write any integral type to a bitstream.  
    //! @access  public  
    //! @param [in] [const templateType & src] 
    //! it is user data that is right aligned in default
    //! @return void
    //! @notice will swap endian internally 
    //! if DO_NOT_SWAP_ENDIAN not defined
    //! @author mengdi[Jackie]
    template <class IntergralType>
    void Write(const IntergralType &src)
    {
        if (sizeof(IntergralType) == 1)
        {
            WriteBits((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
        }
        else
        {
#ifndef DO_NOT_SWAP_ENDIAN
            if (DoEndianSwap())
            {
                UInt8 output[sizeof(IntergralType)];
                ReverseBytes((UInt8*)&src, output, sizeof(IntergralType));
                WriteBits(output, BYTES_TO_BITS(sizeof(IntergralType)), true);
            }
            else
#endif
                WriteBits((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
        }
    }

    //! @func Write 
    //! @access  public  
    //! @brief Write a bool to a bitstream.
    //! @param [in] [const bool & src] The value to write
    //! @return [bool] true succeed, false failed
    //! @author mengdi[Jackie]
    template <>
    inline void Write(const bool &src)
    {
        if (src == true)
            WriteBitOne();
        else
            WriteBitZero();
    }

    //! @func Write 
    //! @brief write a InetAddress to stream
    //! @access  public  
    //! @param [in] [const InetAddress & src]  
    //! @return [bool]  true succeed, false failed
    //! @remark
    //! @notice  will not endian swap the address or port
    //! @author mengdi[Jackie]
    template <>
    inline void Write(const InetAddress &src)
    {
        UInt8 version = src.GetIPVersion();
        Write(version);

        if (version == 4)
        {
            //! Hide the address so routers don't modify it
            InetAddress addr = src;
            UInt32 binaryAddress = ~src.address.addr4.sin_addr.s_addr;
            UInt16 p = addr.GetPortNetworkOrder();
            // Don't endian swap the address or port
            WriteBits((UInt8*)&binaryAddress,
                BYTES_TO_BITS(sizeof(binaryAddress)), true);
            WriteBits((UInt8*)&p, BYTES_TO_BITS(sizeof(p)), true);
        }
        else
        {
#if NET_SUPPORT_IPV6 == 1
            // Don't endian swap
            WriteBits((UInt8*)&src.address.addr6,
                BYTES_TO_BITS(sizeof(src.address.addr6)), true);
#endif
        }
    }

    //! @func Write 
    //! @brief write three bytes into stream
    //! @access  public  
    //! @param [in] [const UInt24 & inTemplateVar]  
    //! @return [void]  true succeed, false failed
    //! @remark
    //! @notice will align write-position to byte-boundary internally
    //! @see  AlignWritePosBits2ByteBoundary()
    //! @author mengdi[Jackie]
    template <>
    inline void Write(const UInt24 &inTemplateVar)
    {
        //AlignWritePosBits2ByteBoundary();
        //AppendBitsCouldRealloc(24);

        if (!IsBigEndian())
        {
            WriteBits((UInt8 *)&inTemplateVar.val, 24, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[0], 8, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[1], 8, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[2], 8, false);
            //data[(mWritingPosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[0];
            //data[(mWritingPosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[1];
            //data[(mWritingPosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[2];
        }
        else
        {
            ReverseBytes((UInt8 *)&inTemplateVar.val, 3);
            WriteBits((UInt8 *)&inTemplateVar.val, 24, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[3], 8, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[2], 8, false);
            //WriteBits(&((UInt8 *)&inTemplateVar.val)[1], 8, false);
            //data[(mWritingPosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[3];
            //data[(mWritingPosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[2];
            //data[(mWritingPosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[1];
        }

        //mWritingPosBits += 24;
    }

    //! @func Write 
    //! @access  public  
    //! @param [in] [const JackieGUID & inTemplateVar]  
    //! @return void
    //! @author mengdi[Jackie]
    template <>
    inline void Write(const JackieGUID &inTemplateVar)
    {
        Write(inTemplateVar.g);
    }

    template <>
    inline void Write(const GecoString &src)
    {
        //src.Write(this);
    }
    //template <>
    //inline void WriteFrom(const RakWString &src)
    //{
    //	src.Serialize(this);
    //}
    template <>
    inline void Write(const char * const &inStringVar)
    {
        // JackieString::Write(inStringVar, this);
    }
    template <>
    inline void Write(const wchar_t * const &inStringVar)
    {
        //JackieWString::Serialize(inStringVar, this);
    }
    template <>
    inline void Write(const UInt8 * const &src)
    {
        Write((const char*)src);
    }
    template <>
    inline void Write(char * const &src)
    {
        Write((const char*)src);
    }
    template <>
    inline void Write(UInt8 * const &src)
    {
        Write((const char*)src);
    }

    //! @func WriteChanged 
    //! @brief write any changed integral type to a bitstream.
    //! @access  public  
    //! @param [in] const templateType & latestVal 
    //! @param [in] const templateType & lastVal  
    //! @return void 
    //! @notice 
    //! If the current value is different from the last value
    //! the current value will be written.  Otherwise, a single bit will be written
    //! @author mengdi[Jackie]
    template <class templateType>
    inline void WriteChangedValue(const templateType &latestVal,
        const templateType &lastVal)
    {
        if (latestVal == lastVal)
        {
            Write(false);
        }
        else
        {
            Write(true);
            Write(latestVal);
        }
    }

    //! @func WriteChanged 
    //! @brief write a bool delta. Same thing as just calling Write
    //! @access  public  
    //! @param [in] const bool & currentValue  
    //! @param [in] const bool & lastValue  
    //! @return void 
    //! @author mengdi[Jackie]
    template <>
    inline void WriteChangedValue(const bool &currentValue,
        const bool &lastValue)
    {
        (void)lastValue;
        Write(currentValue);
    }

    //! @brief WriteDelta when you don't know what the last value is, or there is no last value.
    //! @param[in] currentValue The current value to write
    //! @func WriteChanged 
    //! @brief 
    //! writeDelta when you don't know what the last value is, or there is no last value.
    //! @access  public  
    //! @param [in] const templateType & currentValue  
    //! @return void  
    //! @author mengdi[Jackie]
    template <class templateType>
    inline void WriteChangedValue(const templateType &currentValue)
    {
        Write(true);
        Write(currentValue);
    }

    //! @func WriteMiniChanged 
    //! @brief write any integral type to a bitstream.  
    //! @access  public  
    //! @param [in] const templateType & currVal 
    //! The current value to write 
    //! @param [in] const templateType & lastValue  
    //!  The last value to compare against
    //! @return void 
    //! @notice
    //! If the current value is different from the last value. the current
    //! value will be written.  Otherwise, a single bit will be written
    //! For floating point, this is lossy, using 2 bytes for a float and 
    //! 4 for a double. The range must be between -1 and +1.
    //! For non-floating point, this is lossless, but only has benefit
    //! if you use less than half the bits of the type
    //! If you are not using DO_NOT_SWAP_ENDIAN the opposite is
    //! true for types larger than 1 byte
    //! @author mengdi[Jackie]
    template <class templateType>
    inline void WriteMiniChanged(const templateType&currVal,
        const templateType &lastValue)
    {
        if (currVal == lastValue)
        {
            Write(false);
        }
        else
        {
            Write(true);
            WriteMini(currVal);
        }
    }

    //! @brief Write a bool delta.  Same thing as just calling Write
    //! @param[in] currentValue The current value to write
    //! @param[in] lastValue The last value to compare against
    template <>
    inline void WriteMiniChanged(const bool &currentValue, const bool&
        lastValue)
    {
        (void)lastValue;
        Write(currentValue);
    }

    //! @brief Same as WriteMiniChanged() 
    //! when we have an unknown second parameter
    template <class templateType>
    inline void WriteMiniChanged(const templateType &currentValue)
    {
        Write(true);
        WriteMini(currentValue);
    }

    //! @func WriteMini 
    //! @access  public  
    //! @param [in] const UInt8 * src  
    //! @param [in] const BitSize bits2Write  write size in bits
    //! @param [in] const bool isUnsigned  
    //! @return void 
    //! @notice 
    //! this function assumes that @src points to a native type,
    //! compress and write it.
    //! @Remarks
    //! assume we have src with value of FourOnes-FourOnes-FourOnes-11110001
    //!++++++++++++++> High Memory Address (hma)
    //!++++++++++++++++++++++++++++++
    //! | FourOnes | FourOnes | FourOnes | 11110001 |  Big Endian 
    //!++++++++++++++++++++++++++++++
    //!++++++++++++++++++++++++++++++
    //! |11110001 | FourOnes | FourOnes | FourOnes |  Little Endian 
    //!++++++++++++++++++++++++++++++
    //! for little endian, the high bytes are located in hma and so @currByte should 
    //! increment from value of highest index ((bits2Write >> 3) - 1)
    //! for big endian, the high bytes are located in lma and so @currByte should 
    //! increment from value of lowest index (0)
    //! 在字节内部，一个字节的二进制排序，不存在大小端问题。
    //! 就和平常书写的一样，先写高位，即低地址存储高位。
    //! 如char a=0x12.存储从低位到高位就为0001 0010
    //! @author mengdi[Jackie]
    void WriteMini(const UInt8* src, const BitSize bits2Write, const bool isUnsigned);

    //! @func WriteMini 
    //! @brief Write any integral type to a bitstream,
    //! endian swapping counters internally. default is unsigned (isUnsigned = true)
    //! @access  public  
    //! @param [in] const IntergralType & src  
    //! @return void 
    //! @notice
    //! For floating point, this is lossy, using 2 bytes for a float and 4 for 
    //! a double.  The range must be between -1 and +1.
    //! For non-floating point, this is lossless, but only has benefit 
    //! if you use less than half the bits of the type
    //! we write low bits and reassenble the value in receiver endpoint
    //! based on its endian, so no need to do endian swap here
    //! @author mengdi[Jackie]
    template <bool isUnsigned = unsigned_integral, class IntergralType>
    inline void WriteMini(const IntergralType &src)
    {
        WriteMini((UInt8*)&src, sizeof(IntergralType) << 3, isUnsigned);
    }

    template <> inline void WriteMini(const InetAddress &src)
    {
        //Write(src);
        UInt8 version = src.GetIPVersion();
        WriteMini<unsigned_integral>(version);

        if (version == 4)
        {
            //! Hide the address so routers don't modify it
            InetAddress addr = src;
            UInt32 binaryAddress = ~src.address.addr4.sin_addr.s_addr;
            UInt16 p = addr.GetPortNetworkOrder();
            WriteMini<unsigned_integral>(binaryAddress);
            WriteMini<unsigned_integral>(p);
        }
        else
        {
#if NET_SUPPORT_IPV6 == 1
            UInt32 binaryAddress = src.address.addr6;
            WriteMini<unsigned_integral>(binaryAddress);
#endif
        }
    }
    template <> inline void WriteMini(const JackieGUID &src)
    {
        WriteMini<unsigned_integral>(src.g);
    }
    template <> inline void WriteMini(const UInt24 &var)
    {
        WriteMini<unsigned_integral>(var.val);
        //Write(var);
    }
    template <> inline void WriteMini(const bool &src)
    {
        Write(src);
    }

    inline void WriteMini(const float &src)
    {
        assert(src > -1.01f && src < 1.01f);
        float varCopy = src;
        if (varCopy < -1.0f) varCopy = -1.0f;
        if (varCopy > 1.0f) varCopy = 1.0f;
        WriteMini<unsigned_integral>((UInt16)((varCopy + 1.0f)*32767.5f));
    }
    template <> //!@notice For values between -1 and 1
    inline void WriteMini(const double &src)
    {
        assert(src > -1.01f && src < 1.01f);
        double varCopy = src;
        if (varCopy < -1.0f) varCopy = -1.0f;
        if (varCopy > 1.0f) varCopy = 1.0f;
        WriteMini<unsigned_integral>((UInt32)((varCopy + 1.0)*2147483648.0));
    }

    //! Compress the string
    template <>
    inline void WriteMini(const GecoString &src)
    {
        // src.WriteMini(this, 0, false);
    }
    //template <>
    //inline void WriteMini(const JackieWString &src)
    //{
    //	src.Serialize(this);
    //}
    template <>
    inline void WriteMini(const char * const &inStringVar)
    {
        // JackieString::WriteMini(inStringVar, this);
    }
    template <>
    inline void WriteMini(const wchar_t * const &inStringVar)
    {
        //JackieWString::Serialize(inStringVar, this);
    }
    template <>
    inline void WriteMini(const UInt8 * const &src)
    {
        WriteMini((const char*)src);
    }
    template <>
    inline void WriteMini(char * const &src)
    {
        WriteMini((const char*)src);
    }
    template <>
    inline void WriteMini(UInt8 * const &src)
    {
        WriteMini((const char*)src);
    }

    //! @access public 
    //! @returns void
    template <class destType, class srcType >
    void WriteCasted(const srcType &value)
    {
        destType val = (destType)value;
        Write(val);
    }

    //! @method WriteBitsIntegerRange
    //! @access public 
    //! @returns void
    //! @param [in] const templateType value 
    //! value Integer value to write, which should be
    //! between @param mini and @param max
    //! which should be between @paramminimum and @param maximum
    //! @param [in] const templateType minimum best to use global const
    //! @param [in] const templateType maximum best to use global const
    //! @param [in] bool allowOutsideRange
    //! If true, all sends will take an extra bit, however value can deviate
    //! from outside @param minimum and @param maximum.
    //! If false, will assert if the value deviates
    //! @brief Given the minimum and maximum values for an integer type,
    //! figure out the minimum number of bits to represent the range
    //! Then serialize only those bits, smaller the difference is, less bits to use,
    //! no matter how big the max or mini is, best to send and recv huge numbers,
    //! like 666666666, SerializeMini() will not work well in such case,
    //! @notice
    //! A static is used so that the required number of bits for
    //! (maximum - minimum) is only calculated once.This does require that
    //! @param minimum and @param maximum are fixed values for a
    //! given line of code  for the life of the program
    //! @use
    //! const uint64 MAX_VALUE = 1000000000;
    //! const uint64 Mini_VALUE = 999999900;
    //! uint64 currVal = 999999966;
    //! SerializeIntegerRange(true, currVal, MAX_VALUE, Mini_VALUE);
    //! uint64 Val;
    //! SerializeIntegerRange(fals, Val, MAX_VALUE, Mini_VALUE);
    //! the sample above will use 7 bits (128) instead of 8 bytes
    //! if you use SerializeMini(), will also use 8 bytes because there are  no zero bits to compress
    template <class IntegerType>
    void WriteIntegerRange(
        const IntegerType value,
        const IntegerType mini,
        const IntegerType max,
        bool allowOutsideRange = false)
    {
        IntegerType diff = max - mini;
        int requiredBits = BYTES_TO_BITS(sizeof(IntegerType)) - GetLeadingZeroSize(diff);
        WriteIntegerRange(value, mini, max, requiredBits, allowOutsideRange);
    }

    //! @brief 
    //! only work for positive integers but you can transfer nagative integers as postive
    //! integers and transform it back to negative at receipt endpoint.
    //! the smaller difference between min and max, the less bits used to transmit
    //! eg. given a number of 105 in 100 - 120 is more efficiently compressed 
    //! than that in 0 - 120, you actually is sending number of 105-100=5
    //! it is even more efficient than using WriteMini()
    //! @Remarks
    //! Assume@valueBeyondMini's value is 00000000 - 00101100 
    //! Memory Address ------------------> 
    //! 00000000   00101100   Big Endian
    //! 00101100   00000000   Little Endian 
    //! so for big endian, we need to reverse byte so that
    //! the high byte of 00101100 that was put in low address can be written correctly
    //! for little endian, we do nothing.
    template <class IntegerType>
    void WriteIntegerRange(
        const IntegerType value,
        const IntegerType mini,
        const IntegerType max,
        const int requiredBits,
        bool allowOutsideRange = false)
    {
        assert(max >= mini);
        assert(allowOutsideRange == true || (value >= mini && value <= max));

        if (allowOutsideRange)
        {
            if (value <mini || value>max)  //!< out of range
            {
                Write(true);
                WriteMini(value);
                return;
            }
            Write(false); //!< inside range
        }

        IntegerType valueBeyondMini = value - mini;
        if (IsBigEndian())
        {
            UInt8 output[sizeof(IntegerType)];
            ReverseBytes((UInt8*)&valueBeyondMini, output, sizeof(IntegerType));
            WriteBits(output, requiredBits, true);
        }
        else
        {
            WriteBits((UInt8*)&valueBeyondMini, requiredBits, true);
        }
    }

    //! @method WriteNormVector
    //! @access public 
    //! @returns void
    //! @param [in] templateType x
    //! @param [in] templateType y
    //! @param [in] templateType z
    //! @brief
    //! Write a normalized 3D vector, using (at most) 4 bytes + 3 bits
    //! instead of 12 - 24 bytes. Accurate to 1/32767.5.
    //! @notice
    //! Will further compress y or z axis aligned vectors.
    //! templateType for this function must be a float or double
    //! @see
    template <class templateType> void WriteNormVector(
        templateType x,
        templateType y,
        templateType z)
    {
        assert(x <= 1.01 &&y <= 1.01 &&z <= 1.01 &&x >= -1.01 &&y >= -1.01 &&z >= -1.01);
        WriteFloatRange((float)x, -1.0f, 1.0f);
        WriteFloatRange((float)y, -1.0f, 1.0f);
        WriteFloatRange((float)z, -1.0f, 1.0f);
    }

    //! @method WriteVector
    //! @access public 
    //! @returns void
    //! @brief Write a vector, using 10 bytes instead of 12.
    //! @notice
    //! Loses accuracy to about 3/10ths and only saves 2 bytes, 
    //! so only use if accuracy is not important
    //! templateType for this function must be a float or double
    //! @see
    template <class FloatingType> void WriteVector(
        FloatingType x,
        FloatingType y,
        FloatingType z)
    {
        FloatingType magnitude = sqrt(x * x + y * y + z * z);
        Write((float)magnitude);
        if (magnitude > 0.00001f)
        {
            WriteMini((float)(x / magnitude));
            WriteMini((float)(y / magnitude));
            WriteMini((float)(z / magnitude));
        }
    }

    //! @method WriteNormQuat
    //! @access public 
    //! @returns void
    //! @brief 
    //! Write a normalized quaternion in (18 bits[best case] to 6 bytes[worest case]) + 4 bits instead of 16 bytes.  
    //! Slightly lossy.
    //! @notice
    //! FloatingType for this function must be a float or double
    //! @see
    template <class FloatingType> void WriteNormQuat(
        FloatingType w,
        FloatingType x,
        FloatingType y,
        FloatingType z)
    {
        Write((bool)(w < 0.0));
        Write((bool)(x < 0.0));
        Write((bool)(y < 0.0));
        Write((bool)(z < 0.0));
        WriteMini((UInt16)(fabs(x)*65535.0));
        WriteMini((UInt16)(fabs(y)*65535.0));
        WriteMini((UInt16)(fabs(z)*65535.0));
        // Leave out w and calculate it on the target
    }

    //! @method WriteOrthMatrix
    //! @access public 
    //! @returns void
    //! @brief
    //! Write an orthogonal matrix by creating a quaternion, 
    //! and writing 3 components of the quaternion in 2 bytes each.
    //! @notice
    //! Lossy, although the result is renormalized
    //! Use (18 bits to 6 bytes) +4 bits instead of 36
    //! FloatingType for this function must be a float or double
    //! @see WriteNormQuat()
    template <class FloatingType> void WriteOrthMatrix(
        FloatingType m00, FloatingType m01, FloatingType m02,
        FloatingType m10, FloatingType m11, FloatingType m12,
        FloatingType m20, FloatingType m21, FloatingType m22)
    {
        double qw;
        double qx;
        double qy;
        double qz;

        // Convert matrix to quat
        // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixQuaternion/
        float sum;
        sum = 1 + m00 + m11 + m22;
        if (sum < 0.0f) sum = 0.0f;
        qw = sqrt(sum) / 2;
        sum = 1 + m00 - m11 - m22;
        if (sum < 0.0f) sum = 0.0f;
        qx = sqrt(sum) / 2;
        sum = 1 - m00 + m11 - m22;
        if (sum < 0.0f) sum = 0.0f;
        qy = sqrt(sum) / 2;
        sum = 1 - m00 - m11 + m22;
        if (sum < 0.0f) sum = 0.0f;
        qz = sqrt(sum) / 2;
        if (qw < 0.0) qw = 0.0;
        if (qx < 0.0) qx = 0.0;
        if (qy < 0.0) qy = 0.0;
        if (qz < 0.0) qz = 0.0;
        qx = _copysign((double)qx, (double)(m21 - m12));
        qy = _copysign((double)qy, (double)(m02 - m20));
        qz = _copysign((double)qz, (double)(m10 - m01));

        WriteNormQuat(qw, qx, qy, qz);
    }

    //! @brief  Write zeros until the bitstream is filled up to @param bytes
    //! @notice will internally align write pos and then reallocate if necessary
    //!  the @mWritePosBits will be byte aligned
    void PadZero2LengthOf(UInt32 bytes);

    //! @brief swao bytes starting from @data with offset given
    inline void EndianSwapBytes(UInt32 byteOffset, UInt32 length)
    {
        if (DoEndianSwap()) ReverseBytes(data_ + byteOffset, length);
    }

    //! @brief Makes a copy of the internal data for you @param _data 
    //! will point to the stream. Partial bytes are left aligned
    //! @param[out] _data The allocated copy of GetData()
    //! @return The length in bits of the stream.
    //! @notice
    //! all bytes are copied besides the bytes in GetPayLoadBits()
    BitSize Copy(UInt8*& _data) const
    {
        assert(writable_bit_pos_ > 0);

        _data = (UInt8*)gMallocEx(BITS_TO_BYTES(writable_bit_pos_),
            TRACKE_MALLOC);
        memcpy(_data, data_, sizeof(UInt8) * BITS_TO_BYTES(writable_bit_pos_));
        return writable_bit_pos_;
    }

    //!@brief Ignore data we don't intend to read
    void ReadSkipBits(const BitSize numberOfBits)
    {
        readable_bit_pos_ += numberOfBits;
    }
    void ReadSkipBytes(const ByteSize numberOfBytes)
    {
        ReadSkipBits(BYTES_TO_BITS(numberOfBytes));
    }

    void WriteOneAlignedBytes(const char *inByteArray)
    {
        assert((writable_bit_pos_ & 7) == 0);
        AppendBitsCouldRealloc(8);
        data_[writable_bit_pos_ >> 3] = inByteArray[0];
        writable_bit_pos_ += 8;
    }
    void ReadOneAlignedBytes(char *inOutByteArray)
    {
        assert((readable_bit_pos_ & 7) == 0);
        assert(GetPayLoadBits() >= 8);
        // if (curr_readable_pos + 1 * 8 > mWritePosBits) return;

        inOutByteArray[0] = data_[(readable_bit_pos_ >> 3)];
        readable_bit_pos_ += 8;
    }

    void WriteTwoAlignedBytes(const char *inByteArray)
    {
        assert((writable_bit_pos_ & 7) == 0);
        AppendBitsCouldRealloc(16);
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            data_[(writable_bit_pos_ >> 3) + 0] = inByteArray[1];
            data_[(writable_bit_pos_ >> 3) + 1] = inByteArray[0];
        }
        else
#endif
        {
            data_[(writable_bit_pos_ >> 3) + 0] = inByteArray[0];
            data_[(writable_bit_pos_ >> 3) + 1] = inByteArray[1];
        }

        writable_bit_pos_ += 16;
    }
    void ReadTwoAlignedBytes(char *inOutByteArray)
    {
        assert((readable_bit_pos_ & 7) == 0);
        assert(GetPayLoadBits() >= 16);
        //if (curr_readable_pos + 16 > mWritePosBits) return ;
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            inOutByteArray[0] = data_[(readable_bit_pos_ >> 3) + 1];
            inOutByteArray[1] = data_[(readable_bit_pos_ >> 3) + 0];
        }
        else
#endif
        {
            inOutByteArray[0] = data_[(readable_bit_pos_ >> 3) + 0];
            inOutByteArray[1] = data_[(readable_bit_pos_ >> 3) + 1];
        }

        readable_bit_pos_ += 16;
    }

    void WriteFourAlignedBytes(const char *inByteArray)
    {
        assert((writable_bit_pos_ & 7) == 0);
        AppendBitsCouldRealloc(32);
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            data_[(writable_bit_pos_ >> 3) + 0] = inByteArray[3];
            data_[(writable_bit_pos_ >> 3) + 1] = inByteArray[2];
            data_[(writable_bit_pos_ >> 3) + 2] = inByteArray[1];
            data_[(writable_bit_pos_ >> 3) + 3] = inByteArray[0];
        }
        else
#endif
        {
            data_[(writable_bit_pos_ >> 3) + 0] = inByteArray[0];
            data_[(writable_bit_pos_ >> 3) + 1] = inByteArray[1];
            data_[(writable_bit_pos_ >> 3) + 2] = inByteArray[2];
            data_[(writable_bit_pos_ >> 3) + 3] = inByteArray[3];
        }

        writable_bit_pos_ += 32;
    }
    void ReadFourAlignedBytes(char *inOutByteArray)
    {
        assert((readable_bit_pos_ & 7) == 0);
        assert(GetPayLoadBits() >= 32);
        //if (curr_readable_pos + 4 * 8 > mWritePosBits) return;
#ifndef DO_NOT_SWAP_ENDIAN
        if (DoEndianSwap())
        {
            inOutByteArray[0] = data_[(readable_bit_pos_ >> 3) + 3];
            inOutByteArray[1] = data_[(readable_bit_pos_ >> 3) + 2];
            inOutByteArray[2] = data_[(readable_bit_pos_ >> 3) + 1];
            inOutByteArray[3] = data_[(readable_bit_pos_ >> 3) + 0];
        }
        else
#endif
        {
            inOutByteArray[0] = data_[(readable_bit_pos_ >> 3) + 0];
            inOutByteArray[1] = data_[(readable_bit_pos_ >> 3) + 1];
            inOutByteArray[2] = data_[(readable_bit_pos_ >> 3) + 2];
            inOutByteArray[3] = data_[(readable_bit_pos_ >> 3) + 3];
        }

        readable_bit_pos_ += 32;
    }

    //!@brief text-print bits starting from @data to @mWritingPosBits
    void Bitify(void);
    void Hexlify(void);

    //! @briefAssume we have value of 00101100   00000000   Little Endian
    //! the required bits are 8(0000000)+2(first 2 bits from left to right in 00101100)
    //! = 10 buts in total
    inline static int GetLeadingZeroSize(Int8 x)
    {
        return GetLeadingZeroSize((UInt8)x);
    }
    inline static int GetLeadingZeroSize(UInt8 x)
    {
        UInt8 y;
        int n;

        n = 8;
        y = x >> 4; if (y != 0)
        {
            n = n - 4; x = y;
        }
        y = x >> 2; if (y != 0)
        {
            n = n - 2; x = y;
        }
        y = x >> 1; if (y != 0) return n - 2;
        return (int)(n - x);
    }
    inline static int GetLeadingZeroSize(Int16 x)
    {
        return GetLeadingZeroSize((UInt16)x);
    }
    inline static int GetLeadingZeroSize(UInt16 x)
    {
        UInt16 y;
        int n;

        n = 16;
        y = x >> 8; if (y != 0)
        {
            n = n - 8; x = y;
        }
        y = x >> 4; if (y != 0)
        {
            n = n - 4; x = y;
        }
        y = x >> 2; if (y != 0)
        {
            n = n - 2; x = y;
        }
        y = x >> 1; if (y != 0) return n - 2;
        return (int)(n - x);
    }
    inline static int GetLeadingZeroSize(Int32 x)
    {
        return GetLeadingZeroSize((UInt32)x);
    }
    inline static int GetLeadingZeroSize(UInt32 x)
    {
        UInt32 y;
        int n;

        n = 32;
        y = x >> 16; if (y != 0)
        {
            n = n - 16; x = y;
        }
        y = x >> 8; if (y != 0)
        {
            n = n - 8; x = y;
        }
        y = x >> 4; if (y != 0)
        {
            n = n - 4; x = y;
        }
        y = x >> 2; if (y != 0)
        {
            n = n - 2; x = y;
        }
        y = x >> 1; if (y != 0) return n - 2;
        return (int)(n - x);
    }
    inline static int GetLeadingZeroSize(Int64 x)
    {
        return GetLeadingZeroSize((UInt64)x);
    }
    inline static int GetLeadingZeroSize(UInt64 x)
    {
        UInt64 y;
        int n;

        n = 64;
        y = x >> 32; if (y != 0)
        {
            n = n - 32; x = y;
        }
        y = x >> 16; if (y != 0)
        {
            n = n - 16; x = y;
        }
        y = x >> 8; if (y != 0)
        {
            n = n - 8; x = y;
        }
        y = x >> 4; if (y != 0)
        {
            n = n - 4; x = y;
        }
        y = x >> 2; if (y != 0)
        {
            n = n - 2; x = y;
        }
        y = x >> 1; if (y != 0) return n - 2;
        return (int)(n - x);
    }

    inline static bool DoEndianSwap(void)
    {
#ifndef DO_NOT_SWAP_ENDIAN
        return IsNetworkOrder() == false;
#else
        return false;
#endif
    }
    inline static bool IsNetworkOrder(void)
    {
        static int a = 0x01;
        static bool isNetworkOrder = *((char*)& a) != 1;
        return isNetworkOrder;
    }
    inline static bool IsBigEndian(void)
    {
        return IsNetworkOrder();
    }

    //! @Brief faster than ReverseBytes() if you want to reverse byte
    //! for a variable teself internnaly like uint64 will loop 12 times 
    //! compared to 8 times using ReverseBytes()
    inline static void ReverseBytes(UInt8 *src, const UInt32 length)
    {
        UInt8 temp;
        for (UInt32 i = 0; i < (length >> 1); i++)
        {
            temp = src[i];
            src[i] = src[length - i - 1];
            src[length - i - 1] = temp;
        }
    }
    inline static void ReverseBytes(UInt8 *src, UInt8 *dest, const UInt32 length)
    {
        for (UInt32 i = 0; i < length; i++)
        {
            dest[i] = src[length - i - 1];
        }
    }

    //! Can only print 4096 size of UInt8 no materr is is bit or byte
    //! mainly used for dump binary data

    //! @brief Return the bit representation of the binary data.
    static void Bitify(char* out_bitstr, BitSize bitsPrint, UInt8* src);
    //! @brief Return the binary data represented by the bit string bitstr.
    static void unBitify(char* outstr, BitSize bitsPrint, UInt8* src);

    //! @brief Return the hexadecimal representation of the binary data.
    static void Hexlify(char* out_hexstr, BitSize bitsPrint, UInt8* src);
    //! @brief Return the binary data represented by the hexadecimal string hexstr.
    static void unHexlify(char* outstr, BitSize bitsPrint, UInt8* src);
};

GECO_NET_END_NSPACE
#endif  //__BITSTREAM_H__
