#import "stdint.h"
#import "stdlib.h"
#import "errno.h"
#import "pthread.h"
#import "include/OSLocking.h"

#define PACK_UNFAIRNESS(ptr, b) (void *) ((uintptr_t) ptr | ((uintptr_t) 0x01 & (uintptr_t) b))
#define IS_UNFAIR_LOCK(lock) (bool) ((uintptr_t) lock.tagged & (uintptr_t) 0x01)
#define GET_LOCK_POINTER(lock) (void *) ((uintptr_t) lock.tagged & ~((uintptr_t) 0x01))

#if defined(__MACH__)
	#import "Availability.h"
	#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 100000 || __MAC_OS_X_VERSION_MAX_ALLOWED >= 1012
		#define __RAS_USE_UNFAIR_LOCK
		#import "os/lock.h"
	#endif
#endif

ras_lock_t ras_lock_create(bool usesUnfairLock) {
	#ifndef __RAS_USE_UNFAIR_LOCK
	usesUnfairLock = false;
	#endif

	ras_lock_t lock;

#ifndef __clang_analyzer__
	if (usesUnfairLock) {
#ifndef __RAS_USE_UNFAIR_LOCK
		abort();
#else
		os_unfair_lock_t ref = (os_unfair_lock_t) malloc(sizeof(os_unfair_lock));
		*ref = OS_UNFAIR_LOCK_INIT;
		lock.tagged = PACK_UNFAIRNESS(ref, usesUnfairLock);
#endif
	} else {
		pthread_mutex_t *ref = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

		int code = pthread_mutex_init(ref, NULL);
		if (0 != code) abort();

		lock.tagged = PACK_UNFAIRNESS(ref, usesUnfairLock);
	}
#endif

	return lock;
}

void ras_lock_destroy(const ras_lock_t lock) {
	if (!IS_UNFAIR_LOCK(lock)) {
		int code = pthread_mutex_destroy((pthread_mutex_t *) GET_LOCK_POINTER(lock));
		if (0 != code) abort();

		free(GET_LOCK_POINTER(lock));
		return;
	}

#ifndef __RAS_USE_UNFAIR_LOCK
	abort();
#else
		free(GET_LOCK_POINTER(lock));
#endif
}

void ras_lock_lock(const ras_lock_t lock) {
	if (!IS_UNFAIR_LOCK(lock)) {
		int code = pthread_mutex_lock((pthread_mutex_t *) GET_LOCK_POINTER(lock));
		if (0 != code) abort();

		return;
	}

#ifndef __RAS_USE_UNFAIR_LOCK
	abort();
#else
	os_unfair_lock_lock((os_unfair_lock_t) GET_LOCK_POINTER(lock));
#endif
}

void ras_lock_unlock(const ras_lock_t lock) {
	if (!IS_UNFAIR_LOCK(lock)) {
		int code = pthread_mutex_unlock((pthread_mutex_t *) GET_LOCK_POINTER(lock));
		if (0 != code) abort();

		return;
	}

#ifndef __RAS_USE_UNFAIR_LOCK
	abort();
#else
	os_unfair_lock_unlock((os_unfair_lock_t) GET_LOCK_POINTER(lock));
#endif
}

bool ras_lock_trylock(const ras_lock_t lock) {
	if (!IS_UNFAIR_LOCK(lock)) {
		int code = pthread_mutex_trylock((pthread_mutex_t *) GET_LOCK_POINTER(lock));
		switch (code) {
			case 0: return true;
			case EBUSY: return false;
			default: abort();
		}
	}

#ifndef __RAS_USE_UNFAIR_LOCK
	abort();
#else
	return os_unfair_lock_trylock((os_unfair_lock_t) GET_LOCK_POINTER(lock));
#endif
}
