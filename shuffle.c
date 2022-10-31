#include "shuffle.h"

uint seed = 0, reseed = 0;

static uint reseeder(uint primr) {
	uint _primr = primr;

reseeder_primr:	
	if (! primr) {
		primr += (reseed ^ _primr);
		primr++;
		_primr++;
		goto reseeder_primr;
	}
	else {
reseeder_seed:
		if (! seed) {
			seed ^= reseed;
			seed++;
			goto reseeder_seed;
		}
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof(reseed));

	reseed <<= reseedbits;
	reseed |= ONES(reseedbits);

	return seed ^ reseed;
}

static uint seeder (uint primr) {
	uint _primr = primr;

seeder_primr:	
	if (! primr) {
		primr += (reseed ^ _primr);
		primr++;
		_primr++;
		goto seeder_primr;
	}
	else {
seeder_seed:
		if (! seed) {
			seed ^= reseed;
			seed++;
			goto seeder_seed;
		}
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof(reseed));

	reseed |= seed;
	reseed <<= reseedbits;
	reseed ^= seed;

	return reseeder(seed+reseed+1);
}

uint seeded(uint n) {
  if (!n) {
    return n;
  }

  uint ret = 0;

  seed = seeder(seed+reseed+1);
  ret = (seed+reseed) % n;
  reseeder(ret);

  return ret;
}
