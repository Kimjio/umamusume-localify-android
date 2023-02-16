#ifndef UMAMUSUMELOCALIFYANDROID_FNV1A_HASH_HPP
#define UMAMUSUMELOCALIFYANDROID_FNV1A_HASH_HPP

#include <stddef.h>

#define ull unsigned long long

// from MSVC
namespace fnv1a {
    constexpr ull _FNV_offset_basis1 = 14695981039346656037ULL;
    constexpr ull _FNV_prime1 = 1099511628211ULL;

    inline ull _Fnv1a_append_bytes1(ull _Val, const unsigned char *const _First,
                                    const ull _Count) noexcept { // accumulate range [_First, _First + _Count) into partial FNV-1a hash _Val
        for (ull _Idx = 0; _Idx < _Count; ++_Idx) {
            _Val ^= static_cast<ull>(_First[_Idx]);
            _Val *= _FNV_prime1;
        }

        return _Val;
    }

    template<class _Kty>
    ull fnv1a_hash_bytes(
            const _Kty *const _First,
            const ull _Count) noexcept { // bitwise hashes the representation of an array
        return _Fnv1a_append_bytes1(
                _FNV_offset_basis1, reinterpret_cast<const unsigned char *>(_First),
                _Count * sizeof(_Kty));
    }
}


#endif //UMAMUSUMELOCALIFYANDROID_FNV1A_HASH_HPP
