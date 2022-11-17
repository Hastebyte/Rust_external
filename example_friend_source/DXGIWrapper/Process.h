#ifndef PROCESS_H
#define PROCESS_H

#pragma once

#define SAFE_DELETE_VECTOR(vec)	if (!vec.empty()) { \
									for (auto& slot : vec) { \
										delete slot; \
										slot = nullptr; \
									} \
								}

class Process {
public:
							Process();
							~Process();

	bool					IsValid(uint64_t Ptr);
	bool					Read(uint64_t dwAddress, LPVOID lpBuffer, uint64_t dwSize);
	bool					Write(uint64_t dwAddress, LPCVOID lpBuffer, uint64_t dwSize);

	template<typename T>
	T Read(uint64_t dwAddress, const T& tDefault = T()) {
		T tRet;
		
		if (!Read(dwAddress, &tRet, sizeof(T))) {
			return tDefault;
		}

		return tRet;
	}

	void mymemcpy_s(void* dst, void* src, size_t num) {
		assert((dst != NULL) && (src != NULL));
		int wordnum = num / 4;
		int slice = num % 4;
		const int* pintsrc = static_cast<const int*>(src);
		int* pintdst = static_cast<int *>(dst);
		//copy 4bytes;
		//同上分两种情况
		//first
		if (pintdst > pintsrc&&pintdst < pintsrc + num) {
			//copy one byte
			const char* pcharsrc = static_cast<const char*>(src);
			char* pchardst = static_cast<char *>(dst);
			for (int i = num - 1, j = 0; j < slice; j++, i--) {
				*(pchardst + i) = *(pcharsrc + i);
			}
			//copy four bytes
			for (int i = wordnum - 1; i >= 0; i--) {
				*(pintdst + i) = *(pintsrc + i);
			}
		}
		else {//second
			//copy four bytes
			for (int i = 0; i < wordnum; i++) {
				*(pintdst + i) = *(pintsrc + i);
			}
			//copy one byte
			const char *pcharsrc = (const char*)pintsrc;
			char* pchardst = (char *)(pintdst);
			for (int i = 0; i < slice; i++) {
				*pchardst++ = *pcharsrc++;
			}
		}
	}

	template<typename T>
	T ReadPtr(uint64_t dwAddress, const T& tDefault = T()) {
		T tRet;

		mymemcpy_s(&tRet, (LPVOID)dwAddress, sizeof(T));

		return tRet;
	}

	template<typename T>
	bool Write(uint64_t dwAddress, const T& tValue) {
		return Write(dwAddress, &tValue, sizeof(T));
	}
};

extern Process*	process;

#endif // PROCESS_H