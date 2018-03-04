#include "fuzz.h"
#include "session.h"
#include "fuzz-wrapfd.h"
#include "debug.h"

static void setup_fuzzer(void) {
	fuzz_common_setup();
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	static int once = 0;
	if (!once) {
		setup_fuzzer();
		once = 1;
	}

	if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
		return 0;
	}

	m_malloc_set_epoch(1);

	if (setjmp(fuzz.jmp) == 0) {
		buffer *line = buf_getstringbuf(fuzz.input);
		buffer *keyblob = buf_getstringbuf(fuzz.input);

		unsigned int algolen;
		const char* algoname = buf_getstring(keyblob, &algolen);

		if (have_algo(algo, algolen, sshhostkey) == DROPBEAR_FAILURE) {
			dropbear_exit("fuzzer imagined a bogus algorithm");
		}
		fuzz_checkpubkey_line(line, 5, "/home/me/authorized_keys",
			algoname, algolen,
			keyblob->data, keyblob->len);

		buf_free(line);
		buf_free(keyblob);
		m_malloc_free_epoch(1, 0);
	} else {
		m_malloc_free_epoch(1, 1);
		TRACE(("dropbear_exit longjmped"))
		/* dropbear_exit jumped here */
	}

	return 0;
}
