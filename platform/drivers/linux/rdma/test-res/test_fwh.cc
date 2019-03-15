#include <random>
#include <gtest/gtest.h>
#include "wrap_fw.h"

TEST(Fwh, SendWqeMinSize) {
	// speculation disabled

	// up to two sge require 64B
	EXPECT_EQ(ionic_v1_send_wqe_min_size(1, 0, 0), 64);
	EXPECT_EQ(ionic_v1_send_wqe_min_size(2, 0, 0), 64);
	// up to six sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_send_wqe_min_size(3, 0, 0), 64);
	EXPECT_GT(ionic_v1_send_wqe_min_size(6, 0, 0), 64);
	EXPECT_LE(ionic_v1_send_wqe_min_size(3, 0, 0), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(6, 0, 0), 128);
	// up to 14 sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_send_wqe_min_size(7, 0, 0), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(14, 0, 0), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(7, 0, 0), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(14, 0, 0), 256);
	// up to 30 sge require gt 256B but le 512B
	EXPECT_GT(ionic_v1_send_wqe_min_size(15, 0, 0), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(30, 0, 0), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(15, 0, 0), 512);
	EXPECT_LE(ionic_v1_send_wqe_min_size(30, 0, 0), 512);
	// more than 30 sge require gt 512B
	EXPECT_GT(ionic_v1_send_wqe_min_size(31, 0, 0), 512);
	// more inline data will increase wqe size
	EXPECT_LE(ionic_v1_send_wqe_min_size(2, 32, 2), 64);
	EXPECT_GT(ionic_v1_send_wqe_min_size(2, 33, 2), 64);
	EXPECT_LE(ionic_v1_send_wqe_min_size(6, 96, 2), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(6, 97, 2), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(14, 224, 2), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(14, 225, 2), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(30, 480, 2), 512);
	EXPECT_GT(ionic_v1_send_wqe_min_size(30, 481, 2), 512);

	// spec == two (special case with no lengths)

	// up to two sge require 64B
	EXPECT_EQ(ionic_v1_send_wqe_min_size(1, 0, 2), 64);
	EXPECT_EQ(ionic_v1_send_wqe_min_size(2, 0, 2), 64);
	// more inline data will increase wqe size
	EXPECT_LE(ionic_v1_send_wqe_min_size(2, 32, 2), 64);
	EXPECT_GT(ionic_v1_send_wqe_min_size(2, 33, 2), 64);
	EXPECT_LE(ionic_v1_send_wqe_min_size(2, 96, 2), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(2, 97, 2), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(2, 224, 2), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(2, 225, 2), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(2, 480, 2), 512);
	EXPECT_GT(ionic_v1_send_wqe_min_size(2, 481, 2), 512);
	// invalid to call this with sge > spec

	// spec == eight (reserves space for lengths)

	// up to four sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_send_wqe_min_size(1, 0, 8), 64);
	EXPECT_GT(ionic_v1_send_wqe_min_size(4, 0, 8), 64);
	EXPECT_LE(ionic_v1_send_wqe_min_size(1, 0, 8), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(4, 0, 8), 128);
	// up to eight sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_send_wqe_min_size(5, 0, 8), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(8, 0, 8), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(5, 0, 8), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(8, 0, 8), 256);
	// more inline data will increase wqe size
	EXPECT_LE(ionic_v1_send_wqe_min_size(4, 96, 8), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(4, 97, 8), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(8, 224, 8), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(8, 225, 8), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(8, 480, 8), 512);
	EXPECT_GT(ionic_v1_send_wqe_min_size(8, 481, 8), 512);
	// invalid to call this with sge > spec

	// spec == 16 (reserves space for lengths)

	// up to four sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_send_wqe_min_size(1, 0, 16), 64);
	EXPECT_GT(ionic_v1_send_wqe_min_size(4, 0, 16), 64);
	EXPECT_LE(ionic_v1_send_wqe_min_size(1, 0, 16), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(4, 0, 16), 128);
	// up to 12 sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_send_wqe_min_size(5, 0, 16), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(12, 0, 16), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(5, 0, 16), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(12, 0, 16), 256);
	// up to 16 sge require gt 256B but le 512B
	EXPECT_GT(ionic_v1_send_wqe_min_size(13, 0, 16), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(16, 0, 16), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(13, 0, 16), 512);
	EXPECT_LE(ionic_v1_send_wqe_min_size(16, 0, 16), 512);
	// more inline data will increase wqe size
	EXPECT_LE(ionic_v1_send_wqe_min_size(4, 96, 8), 128);
	EXPECT_GT(ionic_v1_send_wqe_min_size(4, 97, 8), 128);
	EXPECT_LE(ionic_v1_send_wqe_min_size(8, 224, 8), 256);
	EXPECT_GT(ionic_v1_send_wqe_min_size(8, 225, 8), 256);
	EXPECT_LE(ionic_v1_send_wqe_min_size(16, 480, 8), 512);
	EXPECT_GT(ionic_v1_send_wqe_min_size(16, 481, 8), 512);
	// invalid to call this with sge > spec
}

TEST(Fwh, SendWqeMaxSge) {
	// speculation disabled
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(6, 0), 2);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(7, 0), 6);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(8, 0), 14);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(9, 0), 30);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(10, 0), 62);

	// spec == two (special case with no lengths)
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(6, 2), 2);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(7, 2), 2);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(8, 2), 2);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(9, 2), 2);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(10, 2), 2);

	// spec == eight (reserves space for lengths)
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(7, 8), 4);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(8, 8), 8);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(9, 8), 8);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(10, 8), 8);

	// spec == 16 (reserves space for lengths)
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(7, 16), 4);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(8, 16), 12);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(9, 16), 16);
	EXPECT_EQ(ionic_v1_send_wqe_max_sge(10, 16), 16);
}

TEST(Fwh, SendWqeMaxData) {
	EXPECT_EQ(ionic_v1_send_wqe_max_data(6), 32);
	EXPECT_EQ(ionic_v1_send_wqe_max_data(7), 96);
	EXPECT_EQ(ionic_v1_send_wqe_max_data(8), 224);
	EXPECT_EQ(ionic_v1_send_wqe_max_data(9), 480);
}

TEST(Fwh, RecvWqeMinSize) {
	// speculation disabled

	// up to two sge require 64B
	EXPECT_EQ(ionic_v1_recv_wqe_min_size(1, 0), 64);
	EXPECT_EQ(ionic_v1_recv_wqe_min_size(2, 0), 64);
	// up to six sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(3, 0), 64);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(6, 0), 64);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(3, 0), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(6, 0), 128);
	// up to 14 sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(7, 0), 128);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(14, 0), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(7, 0), 256);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(14, 0), 256);
	// up to 30 sge require gt 256B but le 512B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(15, 0), 256);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(30, 0), 256);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(15, 0), 512);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(30, 0), 512);
	// more than 30 sge require gt 512B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(31, 0), 512);

	// spec == two (special case with no lengths)

	// up to two sge require 64B
	EXPECT_EQ(ionic_v1_recv_wqe_min_size(1, 2), 64);
	EXPECT_EQ(ionic_v1_recv_wqe_min_size(2, 2), 64);
	// invalid to call this with sge > spec

	// spec == eight (reserves space for lengths)

	// up to four sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(1, 8), 64);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(4, 8), 64);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(1, 8), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(4, 8), 128);
	// up to eight sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(5, 8), 128);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(8, 8), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(5, 8), 256);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(8, 8), 256);
	// invalid to call this with sge > spec

	// spec == 16 (reserves space for lengths)

	// up to four sge require gt 64B but le 128B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(1, 16), 64);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(4, 16), 64);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(1, 16), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(4, 16), 128);
	// up to 12 sge require gt 128B but le 256B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(5, 16), 128);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(12, 16), 128);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(5, 16), 256);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(12, 16), 256);
	// up to 16 sge require gt 256B but le 512B
	EXPECT_GT(ionic_v1_recv_wqe_min_size(13, 16), 256);
	EXPECT_GT(ionic_v1_recv_wqe_min_size(16, 16), 256);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(13, 16), 512);
	EXPECT_LE(ionic_v1_recv_wqe_min_size(16, 16), 512);
	// invalid to call this with sge > spec
}

TEST(Fwh, RecvWqeMaxSge) {
	// speculation disabled
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(6, 0), 2);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(7, 0), 6);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(8, 0), 14);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(9, 0), 30);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(10, 0), 62);

	// spec == two (special case with no lengths)
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(6, 2), 2);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(7, 2), 2);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(8, 2), 2);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(9, 2), 2);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(10, 2), 2);

	// spec == eight (reserves space for lengths)
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(7, 8), 4);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(8, 8), 8);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(9, 8), 8);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(10, 8), 8);

	// spec == 16 (reserves space for lengths)
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(7, 16), 4);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(8, 16), 12);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(9, 16), 16);
	EXPECT_EQ(ionic_v1_recv_wqe_max_sge(10, 16), 16);
}

TEST(Fwh, UseSpecSge) {
	// config spec == zero disables speculation for any num sges

	EXPECT_EQ(ionic_v1_use_spec_sge(1, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(2, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(3, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(4, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(5, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(7, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(8, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(9, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(15, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(16, 0), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(17, 0), 0);

	// config spec == 8 allows up to eight sge with 4B len format
	//   (this config will ship by default)

	// 1 or 2 sge special case: no space reserved for lengths
	EXPECT_EQ(ionic_v1_use_spec_sge(1, 8), 2);
	EXPECT_EQ(ionic_v1_use_spec_sge(2, 8), 2);
	// up to 8 sge in spec format
	EXPECT_EQ(ionic_v1_use_spec_sge(3, 8), 8);
	EXPECT_EQ(ionic_v1_use_spec_sge(4, 8), 8);
	EXPECT_EQ(ionic_v1_use_spec_sge(5, 8), 8);
	EXPECT_EQ(ionic_v1_use_spec_sge(7, 8), 8);
	EXPECT_EQ(ionic_v1_use_spec_sge(8, 8), 8);
	// disable for more than 8 sge
	EXPECT_EQ(ionic_v1_use_spec_sge(9, 8), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(15, 8), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(16, 8), 0);
	EXPECT_EQ(ionic_v1_use_spec_sge(17, 8), 0);

	// config spec == 16 allows up to sixteen sge with 2B len format

	// 1 or 2 sge special case: no space reserved for lengths
	EXPECT_EQ(ionic_v1_use_spec_sge(1, 16), 2);
	EXPECT_EQ(ionic_v1_use_spec_sge(2, 16), 2);
	// up to 16 sge in spec format
	EXPECT_EQ(ionic_v1_use_spec_sge(3, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(4, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(5, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(7, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(8, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(9, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(15, 16), 16);
	EXPECT_EQ(ionic_v1_use_spec_sge(16, 16), 16);
	// disable for more than 16 sge
	EXPECT_EQ(ionic_v1_use_spec_sge(17, 16), 0);
}
