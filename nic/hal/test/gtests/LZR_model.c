//
// LZR cores: C model
//
// (C) 2017 IP Cores, Inc.
//
// Confidential information provided under an NDA clause in the license agreement
//
// Rev. 1.00 - initial release
//      1.01 - fixed boolean constants
//
#include "LZR_model.h"

static int* hashTable;

int Hash(byte a, byte b, byte c)
{
	return (((40543 * ((a << 8) ^ (b << 4) ^ (c))) >> 4) & 0xfff);
}

int Match(byte* data, int len, int pos, int* idx0)
{
	int size = len;
	int posBits = pos & ((1 << addrBits) - 1);
	int posMSB = pos & ~((1 << addrBits) - 1);
	*idx0 = 0;
	if (size < pos + 3) return 0; // Done

	int hash = Hash(data[pos], data[pos + 1], data[pos + 2]);
	*idx0 = hashTable[hash];
	hashTable[hash] = posBits;
	if (*idx0 >= posBits)
	{
		if (posMSB == 0)
			return 0; // Uninitialized
		*idx0 += (posMSB - (1 << addrBits));
	}
	else
		*idx0 += posMSB;
	int matched;
	for (matched = 0;
	pos + matched < size && data[pos + matched] == data[*idx0 + matched] && matched < ((1 << (16 - addrBits)) + 2);
		++matched);
	return matched;
}

int lzrw1a_compress(uint32_t action, uint8_t *hash, uint8_t *data, uint32_t size, uint8_t *result, uint32_t* p_dst_len, uint32_t thresh)
{
	if (action == COMPRESS_ACTION_COMPRESS)
	{
		hashTable = (int *)hash;
		int cf = FALSE; int pos = 0; int rescnt = 0;
		while (pos < size)
		{
			int groupPtr = rescnt;
			int needHeader = TRUE; // Header output postponed for thresholding
			for (int item = 0; item < 16; ++item)
			{
				if (pos >= size)
					break; // No more items
				int cpyIdx;
				int msize = Match(data, size, pos, &cpyIdx);
				int cpyOff = pos - cpyIdx;
				if (msize >= 3)
				{
					// If copy item plus header do not fit, terminate
					if (thresh != 0 && thresh < rescnt + (needHeader ? 2 : 0) + 2)
					{
						cf = TRUE;
						goto finish;
					}

					if (needHeader)
					{
						// Output header template
						result[rescnt++] = 0; // Assume all literals
						result[rescnt++] = 0;
						needHeader = FALSE;
					}

					// Output the copy item
					result[rescnt++] = (byte)(((cpyOff & 0xf00) >> (addrBits - 8)) | (msize - 3));
					result[rescnt++] = (byte)(cpyOff);
					// Update hash for all matches
					++pos;
					int dummy;
					for (int p = 1; p < msize; ++p, ++pos)
						Match(data, size, pos, &dummy);
					// Fix the group descriptor 
					if (item < 8)
						result[groupPtr] |= (byte)(0x01 << item);
					else
						result[groupPtr + 1] |= (byte)(0x01 << (item - 8));
				}
				else
				{
					// If copy item plus header do not fit, terminate
					if (thresh != 0 && thresh < rescnt + (needHeader ? 2 : 0) + 1)
					{
						cf = TRUE;
						goto finish;
					}

					if (needHeader)
					{
						// Output header template
						result[rescnt++] = 0; // Assume all literals
						result[rescnt++] = 0;
						needHeader = FALSE;
					}

					// Literal
					result[rescnt++] = data[pos++];
				}
			}
		}
	finish:
		*p_dst_len = rescnt;
		return TRUE^cf;
	}
	else
	{
		int pos = 0; int rescnt = 0;
		while (pos < size)
		{
			uint32_t group = (uint32_t)((data[pos + 1] << 8) | (data[pos]));
			pos += 2;

			for (int item = 0; item < 16; ++item)
			{
				if (pos >= size) break;
				if ((group & (0x0001 << item)) == 0)
				{
					result[rescnt++] = data[pos++];
				}
				else
				{
					int cpyOff = (data[pos + 1] | ((data[pos] << (addrBits - 8)) & 0xf00));
					if (cpyOff == 0)
						cpyOff += (1 << addrBits);
					int cpyLen = (data[pos] & (0xff >> (addrBits - 8))) + 3;
					int cpyPos = rescnt - cpyOff;
					for (int c = 0; c < cpyLen; ++c)
						result[rescnt++] = result[cpyPos + c];
					pos += 2;
				}
			}
		}
		*p_dst_len = rescnt;
		return 0;
	}
}





