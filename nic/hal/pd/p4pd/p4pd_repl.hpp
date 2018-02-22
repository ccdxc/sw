// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __P4PD_REPL_HPP__
#define __P4PD_REPL_HPP__

#define P4PD_REPL_DATA_SIZE            (60)
#define P4PD_REPL_ENTRY_WIDTH          (64)
#define P4PD_REPL_TABLE_DEPTH          (64*1024)

class p4pd_repl_table_entry {
	struct __attribute__((__packed__)) __p4pd_repl_entry_s {
		uint8_t  token_bytes[P4PD_REPL_DATA_SIZE];
		uint32_t last_entry:1;
		uint32_t num_tokens:4;
		uint32_t next_ptr  :27;
	} repl_entry_;

public:
	p4pd_repl_table_entry()  {memset(&repl_entry_, 0, sizeof(repl_entry_));}
	uint32_t get_last_entry() {return repl_entry_.last_entry;}
	uint32_t get_num_tokens() {return repl_entry_.num_tokens;}
	uint32_t get_next_ptr()   {return repl_entry_.next_ptr;}
	void get_token(void *token, uint32_t index, uint32_t size){memcpy(token, &repl_entry_.token_bytes[index*size], size);}

	void set_last_entry(uint32_t las) {repl_entry_.last_entry = las;}
	void set_num_tokens(uint32_t num) {repl_entry_.num_tokens = num;}
	void set_next_ptr(uint32_t ptr) {repl_entry_.next_ptr = ptr;}
	void set_token(void *token, uint32_t index, uint32_t size){
		for (uint32_t i=0; i<size; i++) {
			repl_entry_.token_bytes[(index * size) + i] = ((uint8_t*)token)[i];
		}
	}
};

hal_ret_t p4pd_repl_entry_read(uint32_t index, p4pd_repl_table_entry* p4pd_repl_entry);
hal_ret_t p4pd_repl_entry_write(uint32_t index, p4pd_repl_table_entry* p4pd_repl_entry);

#endif    // __P4PD_REPL_HPP__
