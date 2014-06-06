#include <vector>
#include <stdexcept>

namespace mntone { namespace rtmp { namespace utility {

	class bit_iterator final
	{
	public:
		using bit_array_data = uint8;
		using bit_array_pointer = bit_array_data*;
		using bit_array_length = int32;
		const bit_array_length bit_length = 8 * sizeof( bit_array_data );

	public:
		bit_iterator( const bit_array_pointer ptr, const bit_array_length length )
			: ptr_( ptr )
			, length_( length )
			, byte_offset_( 0 )
			, bit_offset_( 0 )
		{ }

		bit_iterator( const bit_array_pointer ptr, const bit_array_length length, bit_array_length byte_offset, bit_array_length bit_offset )
			: ptr_( ptr )
			, length_( length )
			, byte_offset_( byte_offset )
			, bit_offset_( bit_offset )
		{ }

		bit_iterator& operator++()
		{
			++bit_offset_;
			if( bit_offset_ % bit_length == 0 )
			{
				++byte_offset_;
				bit_offset_ = 0;
			}
			if( byte_offset_ == length_ )
			{
				throw std::out_of_range( "Out of range." );
			}
		}
		bit_iterator operator++( int ) { bit_iterator ret( *this ); return ( ++ret ); }

		bit_iterator& operator--()
		{
			--bit_offset_;
			if( bit_offset_ == -1 )
			{
				--byte_offset_;
				bit_offset_ = bit_length - 1;
			}
			if( byte_offset_ == -1 )
			{
				throw std::out_of_range( "Out of range." );
			}
		}
		bit_iterator operator--( int ) { bit_iterator ret( *this ); return ( --ret ); }

		bit_iterator& operator+=( bit_array_length offset )
		{
			bit_offset_ += offset;
			if( bit_offset_ > 0 )
			{
				byte_offset_ += bit_offset_ / bit_length;
				bit_offset_ %= bit_length;
			}
			else
			{
				byte_offset_ += ( bit_offset_ - ( bit_length - 1 ) ) / bit_length;
				bit_offset_ = bit_length + bit_offset_ % bit_length;
			}
			if( byte_offset_ < 0 && byte_offset_ >= length_ )
			{
				throw std::out_of_range( "Out of range." );
			}
		}
		bit_iterator operator+( bit_array_length offset ) const { bit_iterator ret( *this ); return ( ret += offset ); }
		bit_iterator& operator-=( bit_array_length offset ) { *this += -offset; }
		bit_iterator operator-( bit_array_length offset ) const { bit_iterator ret( *this ); return ( ret -= offset ); }

		template<typename T>
		T get_and_move_length( bit_array_length length )
		{
			const bit_array_length left_bit = bit_length - bit_offset_;
			if( length >= left_bit )
			{
				T ret = ptr_[byte_offset_++] & get_mask<T>( left_bit );
				bit_offset_ = 0;
				length -= left_bit;

				bit_array_length left_byte = length / bit_length;
				length -= bit_length * left_byte;
				while( left_byte-- != 0 )
				{
					ret = ( ret << bit_length ) | ptr_[byte_offset_++];
				}

				if( length != 0 )
				{
					const auto shift_bit = bit_length - length;
					bit_offset_ += length;
					ret = ( ret << length ) | ( ptr_[byte_offset_] >> shift_bit ) & get_mask<T>( length );
				}
				return ret;
			}

			const auto shift_bit = left_bit - length;
			bit_offset_ += length;
			return ( ptr_[byte_offset_] >> shift_bit ) & get_mask<T>( length );
		}

	private:
		template<typename T>
		T get_mask( bit_array_length length )
		{
			return ( 1 << length ) - 1;
		}

		bit_iterator& operator=( const bit_iterator& );

	public:
		const bit_array_length byte_offset() const noexcept { return byte_offset_; }
		const bit_array_length bit_offset() const noexcept { return bit_offset_; }

	private:
		const bit_array_pointer ptr_;
		const bit_array_length length_;
		bit_array_length byte_offset_, bit_offset_;
	};

} } }