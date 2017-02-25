#ifndef RAS_OSLocking
#define RAS_OSLocking

#if __GNUC__
#define RAS_NOTHROW_NONNULL __attribute__((__nothrow__ __nonnull__))
#else
#define RAS_NOTHROW_NONNULL
#endif

#if __clang__
#define RAS_SWIFT_NAME(n) __attribute__((swift_name(n)))
#else
#define RAS_SWIFT_NAME(n)
#endif

#include <stdbool.h>

/// An unfair lock which requires manual deallocation. It does not guarantee
/// waiting threads to be waken in the lock acquisition order.
RAS_SWIFT_NAME("UnsafeUnfairLock")
typedef struct {
	/// An opaque pointer to the lock.
	// The pointer can represent either `pthread_mutex_t` or `os_unfair_lock`,
	// and the `ras_lock_*` functions differentiates the lock type by inspecting
	// the packed unfairness (bit 0 of the pointer).
	//
	// The bitpacking uses bit 0 which would always be zero due to 32- or 64-bit
	// minimum memory alignment. The tagged pointer should not be passed to any
	// functions other than those defined below.
	//
	// Marking the reference nonnull prevents the Swift importer to synthesize
	// a default initializer.
#if __has_feature(nullability)
	void * _Nonnull tagged;
#else
	void * tagged;
#endif
} ras_lock_t;

RAS_NOTHROW_NONNULL
RAS_SWIFT_NAME("UnsafeUnfairLock.init(_usesUnfairLock:)")
ras_lock_t ras_lock_create(bool usesUnfairLock);

RAS_NOTHROW_NONNULL
RAS_SWIFT_NAME("UnsafeUnfairLock.destroy(self:)")
extern void ras_lock_destroy(const ras_lock_t lock);

RAS_NOTHROW_NONNULL
RAS_SWIFT_NAME("UnsafeUnfairLock.lock(self:)")
extern void ras_lock_lock(const ras_lock_t lock);

RAS_NOTHROW_NONNULL
RAS_SWIFT_NAME("UnsafeUnfairLock.unlock(self:)")
extern void ras_lock_unlock(const ras_lock_t lock);

RAS_NOTHROW_NONNULL
RAS_SWIFT_NAME("UnsafeUnfairLock.try(self:)")
extern bool ras_lock_trylock(const ras_lock_t lock);

#endif /* RAS_OSLocking */
