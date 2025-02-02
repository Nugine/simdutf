std::pair<const char32_t*, char16_t*> sse_convert_utf32_to_utf16(const char32_t* buf, size_t len, char16_t* utf16_output) {

  const char32_t* end = buf + len;

  const __m128i v_0000 = _mm_setzero_si128();
  const __m128i v_ffff0000 = _mm_set1_epi32((int32_t)0xffff0000);
  __m128i forbidden_bytemask = _mm_setzero_si128();

  while (buf + 8 <= end) {
    __m128i in = _mm_loadu_si128((__m128i*)buf);
    __m128i nextin = _mm_loadu_si128((__m128i*)buf+1);
    const __m128i saturation_bytemask = _mm_cmpeq_epi32(_mm_and_si128(_mm_or_si128(in, nextin), v_ffff0000), v_0000);
    const uint32_t saturation_bitmask = static_cast<uint32_t>(_mm_movemask_epi8(saturation_bytemask));

    // Check if no bits set above 16th
    if (saturation_bitmask == 0xffff) {
      // Pack UTF-32 to UTF-16
      const __m128i utf16_packed = _mm_packus_epi32(in, nextin);

      const __m128i v_f800 = _mm_set1_epi16((uint16_t)0xf800);
      const __m128i v_d800 = _mm_set1_epi16((uint16_t)0xd800);
      forbidden_bytemask = _mm_or_si128(forbidden_bytemask, _mm_cmpeq_epi16(_mm_and_si128(utf16_packed, v_f800), v_d800));

      _mm_storeu_si128((__m128i*)utf16_output, utf16_packed);
      utf16_output += 8;
      buf += 8;
    } else {
      size_t forward = 7;
      size_t k = 0;
      if(size_t(end - buf) < forward + 1) { forward = size_t(end - buf - 1);}
      for(; k < forward; k++) {
        uint32_t word = buf[k];
        if((word & 0xFFFF0000)==0) {
          // will not generate a surrogate pair
          if (word >= 0xD800 && word <= 0xDFFF) { return std::make_pair(nullptr, utf16_output); }
          *utf16_output++ = char16_t(word);
        } else {
          // will generate a surrogate pair
          if (word > 0x10FFFF) { return std::make_pair(nullptr, utf16_output); }
          word -= 0x10000;
          *utf16_output++ = char16_t(0xD800 + (word >> 10));
          *utf16_output++ = char16_t(0xDC00 + (word & 0x3FF));
        }
      }
      buf += k;
    }
  }

  // check for invalid input
  if (static_cast<uint32_t>(_mm_movemask_epi8(forbidden_bytemask)) != 0) { return std::make_pair(nullptr, utf16_output); }

  return std::make_pair(buf, utf16_output);
}