/*
-------------------------
BAP - Byte Array Protocol
-------------------------
Version 0.6
Author: Lance Putnam
Date: Oct. 2014

This is an example implementation of a simple serial protocol for transferring
arrays of numbers.

The packet consists of the following unsigned byte fields

	| SOP | SEQ | LEN | data[] | CHK |

where

	SOP		start of packet			always 0xFF
	SEQ		sequence number			can be used for acknowledgement from server
	LEN		array size				number of elements in array (data[])
	data[]	data payload			array of bytes
	CHK		checksum				modulo 256 the sum of all bytes from
									SEQ through the end of the data payload
									(1's complement)
*/

namespace bap{

#ifndef BAP_MAX_PACKET_SIZE
#define BAP_MAX_PACKET_SIZE 259
#endif


/// Create a BAP packet from a given byte array

/// @param[out] buffer	byte buffer in which to store constructed packet
/// @param[ in] data	data payload
/// @param[ in] len		size of data payload, in bytes
/// @param[ in] seq		sequence number
///
/// \returns the number of bytes added to the destination buffer
unsigned createPacket(
	unsigned char * buffer,
	const unsigned char * data, unsigned char len,
	unsigned char seq=0
){

	// Construct the packet according to the specification
	buffer[0] = 0xFF;	// SOP: start of packet
	buffer[1] = seq;	// SEQ: sequence number
	buffer[2] = len;	// LEN: number of array elements

	for(unsigned i=0; i<len; ++i){
		buffer[3+i] = data[i];
	}

	// Compute checksum
	unsigned char chk = 0;

	// Sum all bytes from SEQ to end of data payload
	// Since we use an unsigned char, modulo 256 will happen automatically.
	for(int i=1; i<len+3; ++i){
		chk += buffer[i];
	}

	chk = ~chk;

	// Finally, add checksum to packet
	buffer[3 + len] = chk;

	// Return total packet size, in bytes
	return 3 + len + 1;
}



/// Packet parser

/// This class takes in a single byte at a time (from a serial stream/buffer)
/// and returns whether a packet was parsed. The parsed packet must be dealt
/// with immediately as it will be overwritten when the next byte is input.
class Parser{
public:

	Parser()
	:	mTap(0)
	{}
	
	
	/// Get parsed data payload
	unsigned char * data(){
		return mData;
	}

	/// Get parsed data payload byte
	unsigned char data(int i) const {
		return mData[i];
	}

	/// Get parsed data payload length, in bytes
	unsigned char len() const {
		return mLen;
	}
	
	/// Get parsed packet sequence number
	unsigned char seq() const {
		return mSeq;
	}
	
	
	/// Input byte into parser
	
	/// \returns true if a packet was parsed and false if otherwise.
	///
	bool inputByte(unsigned char byte){

		// Waiting for SOP
		if(mTap == 0 && byte == 0xFF){
			mBegin = 0;
			mBuffer[0] = byte;
			mTap = 1;
		}
		
		// Already found SOP
		else{
			mBuffer[mTap] = byte;
			
			unsigned end = mTap+1;
			
			++mTap;
			if(mTap == BAP_MAX_PACKET_SIZE){
				mTap = 0;
			}
		
			// Can the buffer contain at least one packet?
			if((mBegin+unsigned(3)) < end){
				unsigned char bseq = mBuffer[mBegin+1];
				unsigned char blen = mBuffer[mBegin+2];
				//std::cout << unsigned(bseq) << " " << unsigned(blen) << "\n";

				// Can the buffer contain a packet with size 'len'?
				if((mBegin+unsigned(3)+blen) < end){

					// First, ensure packet checksum is correct
					unsigned char bchk = mBuffer[mBegin+3+blen];
					unsigned char chk = 0;
					for(unsigned j=mBegin+1; j<mBegin+unsigned(3)+blen; ++j){
						//std::cout << "CHK += " << unsigned(buffer[j]) << "\n";
						chk += mBuffer[j];
					}
					chk = ~chk;

					//std::cout << unsigned(chk) << " " << unsigned(bchk) << "\n";
					if(chk == bchk){
						// Success! We have a packet!
						mSeq = bseq;
						mLen = blen;
						mData = mBuffer + mBegin + 3;
						
						mTap = 0;

						return true;
					}
					
					// Bad checksum
					else{
						// Check history for another potential SOP
						for(unsigned j=mBegin+1; j<end; ++j){
							if(mBuffer[j] == 0xFF){
								mBegin = j;
								return false;
							}
						}
						
						// No SOP found, so reset parsing
						mTap = 0;
						return false;
					}
				}
			}
		}
		
		return false;
	}
	

private:
	unsigned char mSeq, mLen; // sequence #, data length
	unsigned char * mData; // data payload
	unsigned char mBuffer[BAP_MAX_PACKET_SIZE];
	unsigned short mTap; // write tap into buffer
	unsigned short mBegin; // location of SOP in buffer
};



// This function is DEPRECATED. Use the bap::Parser class.
/* Returns the index one past the end of the parsed packet. If no packet was
parsed, then the return value is 0.
*/
unsigned parsePacket(
	const unsigned char * buffer, unsigned bufferLen,
	unsigned char * data, unsigned char& len,
	unsigned char& seq
){

	for(unsigned i=0; i<bufferLen; ++i){
		unsigned char b = buffer[i];

		//std::cout << unsigned(buffer[i]) << "\n";

		// Do we have a potential start of packet?
		if(b == 0xFF){
			//std::cout << "Found packet start\n";

			// Can the buffer contain at least one packet?
			if((i+3) < bufferLen){
				unsigned char bseq = buffer[i+1];
				unsigned char blen = buffer[i+2];
				//std::cout << unsigned(bseq) << " " << unsigned(blen) << "\n";

				// Can the buffer contain a packet with size 'len'?
				if((i+3+blen) < bufferLen){

					// First, ensure packet checksum is correct
					unsigned char bchk = buffer[i+3+blen];
					unsigned char chk = 0;
					for(unsigned j=i+1; j<i+3+blen; ++j){
						//std::cout << "CHK += " << unsigned(buffer[j]) << "\n";
						chk += buffer[j];
					}
					chk = ~chk;

					//std::cout << unsigned(chk) << " " << unsigned(bchk) << "\n";
					if(chk == bchk){
						seq = bseq;
						len = blen;

						// Copy data bytes
						for(unsigned j=0; j<len; ++j){
							data[j] = buffer[i+3 + j];
						}

						return i+3+len;
					}
				}
			}
		}
	}

	return 0;
}

} // bap::


