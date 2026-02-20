/*
    Lightweight Json library for Arduino.

    Uses only a caller-supplied buffer for writing JSON to minimize SRAM.
    - 02.12.2025 addon: overloads for F("...") / PROGMEM keys in writer & reader,
    - 15.12.2025 helpers to use F("...") with getItemAs<T>() without extra SRAM.

    Code by: Jari Kaija (FINLAND) 2025
*/

#ifndef JKJSON_H
#define JKJSON_H

#include <Arduino.h>

#define MAX_STRING_INPUT_LENGTH 64

class JKJson
{
public:
    // -----------------------------
    // Stream helpers / sanitizer
    // -----------------------------
    static bool extractFirstJsonObjectInPlace(char* buf);
    static bool captureJsonStream(char c, char* buf, size_t maxLen);

    // Constructor
    JKJson(char* buffer, size_t maxSize);

    // -----------------------------
    // Writer API
    // -----------------------------
    void clear();
    bool beginObject();
    bool endObject();
    bool beginKeyArray(const char* key);
    bool beginArray();
    bool endArray();
    bool addComma();
    // addItem (SRAM key versions – original)
    bool addItem(const char* key, bool value);
    bool addItem(const char* key, long value);
    bool addItem(const char* key, uint32_t value);        
    bool addItem(const char* key, int value);             
    bool addItem(const char* key, unsigned int value);    
    bool addItem(const char* key, float value);
    bool addItem(const char* key, const char* value);
    bool addItem(const char* key, const String& value);
    bool addNull(const char* key);

    // low level
    bool writeText(const char* txt);
    bool writeByte(uint8_t b);
    bool writeBool(bool value);
    void finalize();
    size_t size() const;
    char* data();

    // -----------------------------
    // Sanitizer (copy 1st object)
    // -----------------------------
    static bool extractFirstJsonObject(const char* input,
        char* output,
        size_t maxLen);

    // -----------------------------
    // Reader API (top-level)
    // -----------------------------
    static bool getBool(const char* json, const char* key, bool& out);
    static bool getLong(const char* json, const char* key, long& out);
    static bool getUlong(const char* json, const char* key, unsigned long& out);
    static bool getUint16(const char* json, const char* key, uint16_t& out);
    static bool getByte(const char* json, const char* key, uint8_t& out);
    static bool getFloat(const char* json, const char* key, float& out);
    static bool getString(const char* json, const char* key, char* out, size_t maxLen);

    // -----------------------------
    // Reader API: arrays of objects
    // -----------------------------
    static bool getLongArray(const char* json, const char* key, long* out, size_t maxCount, size_t& outCount);
    static bool getUlongArray(const char* json, const char* key, long* out, size_t maxCount, size_t& outCount);

    static bool getBoolInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, bool& out);
    static bool getLongInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, long& out);
    static bool getUlongInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, unsigned long& out);
    static bool getUint16InArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, uint16_t& out);
    static bool getByteInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, uint8_t& out);
    static bool getFloatInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, float& out);
    static bool getStringInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, char* out, size_t maxLen);

    // -----------------------------
    // Reader helpers
    // -----------------------------
    static const char* skipSpaces(const char* p);
    static const char* findValueForKey(const char* json, const char* key);
    static const char* findArrayElement(const char* json, const char* arrayKey, size_t index);
    static const char* skipValue(const char* p);

    // Pretty-print to any Print
    static void printPrettyJson(const char* json, Print& out, uint8_t indentSpaces = 2);

    // -----------------------------------------------------------
    // ADDON (2025): F("...") / flash key overloads – WRITER
    // -----------------------------------------------------------
    bool beginKeyArray(const __FlashStringHelper* key);
    bool addItem(const __FlashStringHelper* key, bool value);
    bool addItem(const __FlashStringHelper* key, long value);
    bool addItem(const __FlashStringHelper* key, int value);
    bool addItem(const __FlashStringHelper* key, unsigned int value);
    bool addItem(const __FlashStringHelper* key, float value);
    bool addItem(const __FlashStringHelper* key, const char* value);
    bool addItem(const __FlashStringHelper* key, const String& value);
    bool addNull(const __FlashStringHelper* key);

    // -----------------------------------------------------------
    // ADDON (2025): F("...") / flash key overloads – READER (top-level)
    // -----------------------------------------------------------
    static bool getBool(const char* json, const __FlashStringHelper* key, bool& out);
    static bool getLong(const char* json, const __FlashStringHelper* key, long& out);
    static bool getUlong(const char* json, const __FlashStringHelper* key, unsigned long& out);
    static bool getUint16(const char* json, const __FlashStringHelper* key, uint16_t& out);
    static bool getByte(const char* json, const __FlashStringHelper* key, uint8_t& out);
    static bool getFloat(const char* json, const __FlashStringHelper* key, float& out);
    static bool getString(const char* json, const __FlashStringHelper* key, char* out, size_t maxLen);

    // -----------------------------------------------------------
    // ADDON (2025): F("...") / flash key overloads – READER (arrays)
    // -----------------------------------------------------------
    // Each has three overloads: arrayKey in flash, fieldKey in flash, or both in flash.

    // getBoolInArray
    static bool getBoolInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, bool& out);
    static bool getBoolInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, bool& out);
    static bool getBoolInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, bool& out);

    // getLongInArray
    static bool getLongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, long& out);
    static bool getLongInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, long& out);
    static bool getLongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, long& out);

    // getUlongInArray
    static bool getUlongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, unsigned long& out);
    static bool getUlongInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, unsigned long& out);
    static bool getUlongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, unsigned long& out);

    // getUint16InArray
    static bool getUint16InArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, uint16_t& out);
    static bool getUint16InArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint16_t& out);
    static bool getUint16InArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint16_t& out);

    // getByteInArray
    static bool getByteInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, uint8_t& out);
    static bool getByteInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint8_t& out);
    static bool getByteInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint8_t& out);

    // getFloatInArray
    static bool getFloatInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, float& out);
    static bool getFloatInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, float& out);
    static bool getFloatInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, float& out);

    // getStringInArray
    static bool getStringInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, char* out, size_t maxLen);
    static bool getStringInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, char* out, size_t maxLen);
    static bool getStringInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, char* out, size_t maxLen);

    // -----------------------------------------------------------
    // Convenience getters 
    // (declarations kept; implementations & specializations below)
    // -----------------------------------------------------------
    static String getItemAs(const char* json, const char* key);
    static String getItemAs(const char* json, const char* key, size_t maxLen);

    template<typename T>
    static T getItemAs(const char* json, const char* key);

    static String getItemAsInArray(const char* json,
        const char* arrayKey,
        size_t index,
        const char* fieldKey);
    static String getItemAsInArray(const char* json,
        const char* arrayKey,
        size_t index,
        const char* fieldKey,
        size_t maxLen);

    template<typename T>
    static T getItemAsInArray(const char* json,
        const char* arrayKey,
        size_t index,
        const char* fieldKey);

    // --- ADDON: convenience getters with flash keys ---
    template<typename T>
    static T getItemAs(const char* json, const __FlashStringHelper* key);

    template<typename T>
    static T getItemAsInArray(const char* json,
        const __FlashStringHelper* arrayKey,
        size_t index,
        const __FlashStringHelper* fieldKey);

    bool   firstItem;

private:
    char* buf;
    size_t max;
    size_t pos;
    
};

// ===========================================================================
// Template Implementations
// ===========================================================================

// Back-compat default template returns default-constructed T (user specializations follow)
template<typename T>
T JKJson::getItemAs(const char* json, const char* key) { return T{}; }

// String convenience (non-template)
inline String JKJson::getItemAs(const char* json, const char* key) {
    return getItemAs(json, key, MAX_STRING_INPUT_LENGTH);
}
inline String JKJson::getItemAs(const char* json, const char* key, size_t maxLen) {
    if (maxLen == 0) return String();
    char tmp[MAX_STRING_INPUT_LENGTH];
    size_t useLen = (maxLen < sizeof(tmp)) ? maxLen : sizeof(tmp);
    if (getString(json, key, tmp, useLen)) return String(tmp);
    return String();
}

// ---- Type specializations
template<> inline long         JKJson::getItemAs<long        >(const char* json, const char* key) { long v = 0; (void)getLong(json, key, v); return v; }
template<> inline unsigned long JKJson::getItemAs<unsigned long>(const char* json, const char* key) { unsigned long v = 0; (void)getUlong(json, key, v); return v; }
template<> inline int          JKJson::getItemAs<int         >(const char* json, const char* key) { long v = 0; (void)getLong(json, key, v); return (int)v; }
template<> inline uint16_t     JKJson::getItemAs<uint16_t    >(const char* json, const char* key) { uint16_t v = 0; (void)getUint16(json, key, v); return v; }
template<> inline uint8_t      JKJson::getItemAs<uint8_t     >(const char* json, const char* key) { uint8_t v = 0; (void)getByte(json, key, v); return v; }
template<> inline float        JKJson::getItemAs<float       >(const char* json, const char* key) { float v = 0; (void)getFloat(json, key, v); return v; }
template<> inline bool         JKJson::getItemAs<bool        >(const char* json, const char* key) { bool v = false; (void)getBool(json, key, v); return v; }
template<> inline String       JKJson::getItemAs<String      >(const char* json, const char* key) { return getItemAs(json, key); }

// Array variants (original style)
template<typename T>
T JKJson::getItemAsInArray(const char* json,
    const char* arrayKey,
    size_t index,
    const char* fieldKey) {
    return T{};
}

inline String JKJson::getItemAsInArray(const char* json,
    const char* arrayKey,
    size_t index,
    const char* fieldKey) {
    return getItemAsInArray(json, arrayKey, index, fieldKey, MAX_STRING_INPUT_LENGTH);
}
inline String JKJson::getItemAsInArray(const char* json,
    const char* arrayKey,
    size_t index,
    const char* fieldKey,
    size_t maxLen) {
    if (maxLen == 0) return String();
    char tmp[MAX_STRING_INPUT_LENGTH];
    size_t useLen = (maxLen < sizeof(tmp)) ? maxLen : sizeof(tmp);
    if (getStringInArray(json, arrayKey, index, fieldKey, tmp, useLen)) return String(tmp);
    return String();
}

// Array type specializations
template<> inline long         JKJson::getItemAsInArray<long        >(const char* json, const char* ak, size_t idx, const char* fk) { long v = 0; (void)getLongInArray(json, ak, idx, fk, v); return v; }
template<> inline unsigned long JKJson::getItemAsInArray<unsigned long>(const char* json, const char* ak, size_t idx, const char* fk) { unsigned long v = 0; (void)getUlongInArray(json, ak, idx, fk, v); return v; }
template<> inline int          JKJson::getItemAsInArray<int         >(const char* json, const char* ak, size_t idx, const char* fk) { long v = 0; (void)getLongInArray(json, ak, idx, fk, v); return (int)v; }
template<> inline uint16_t     JKJson::getItemAsInArray<uint16_t    >(const char* json, const char* ak, size_t idx, const char* fk) { uint16_t v = 0; (void)getUint16InArray(json, ak, idx, fk, v); return v; }
template<> inline uint8_t      JKJson::getItemAsInArray<uint8_t     >(const char* json, const char* ak, size_t idx, const char* fk) { uint8_t v = 0; (void)getByteInArray(json, ak, idx, fk, v); return v; }
template<> inline float        JKJson::getItemAsInArray<float       >(const char* json, const char* ak, size_t idx, const char* fk) { float v = 0; (void)getFloatInArray(json, ak, idx, fk, v); return v; }
template<> inline bool         JKJson::getItemAsInArray<bool        >(const char* json, const char* ak, size_t idx, const char* fk) { bool v = false; (void)getBoolInArray(json, ak, idx, fk, v); return v; }
template<> inline String       JKJson::getItemAsInArray<String      >(const char* json, const char* ak, size_t idx, const char* fk) { return getItemAsInArray(json, ak, idx, fk); }

// ---------------------------------------------------------------------------
// ADDON: Flash-key convenience templates (no dependency on cpp helpers)
// ---------------------------------------------------------------------------
template<typename T>
T JKJson::getItemAs(const char* json, const __FlashStringHelper* key) {
    char keyBuf[64];
    // Safe copy from PROGMEM
    strncpy_P(keyBuf, (PGM_P)key, sizeof(keyBuf) - 1);
    keyBuf[sizeof(keyBuf) - 1] = '\0';
    return getItemAs<T>(json, keyBuf);
}

template<typename T>
T JKJson::getItemAsInArray(const char* json,
    const __FlashStringHelper* arrayKey,
    size_t index,
    const __FlashStringHelper* fieldKey) {
    char aBuf[64], fBuf[64];
    strncpy_P(aBuf, (PGM_P)arrayKey, sizeof(aBuf) - 1); aBuf[sizeof(aBuf) - 1] = '\0';
    strncpy_P(fBuf, (PGM_P)fieldKey, sizeof(fBuf) - 1); fBuf[sizeof(fBuf) - 1] = '\0';
    return getItemAsInArray<T>(json, aBuf, index, fBuf);
}

#endif // JKJSON_H
