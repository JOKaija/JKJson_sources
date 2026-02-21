#include "JKJson.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// -------------------------------------------------------
// INTERNAL HELPERS
// -------------------------------------------------------
static void trimFloatString(char* s) {
    char* p = s;
    while (*p && *p != '.') p++;
    if (*p == 0) return;
    char* end = s + strlen(s) - 1;
    while (end > p && *end == '0') *end-- = 0;
    if (end == p) *end = 0;
}

static bool isNullLiteral(const char* v) {
    return (strncmp(v, "null", 4) == 0);
}

static void copyFlashString(const __FlashStringHelper* src, char* dest, size_t maxLen) {
    if (!src || !dest || maxLen == 0) return;
    strncpy_P(dest, (PGM_P)src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

// -------------------------------------------------------
// CONSTRUCTOR
// -------------------------------------------------------
JKJson::JKJson(char* buffer, size_t maxSize) {
    buf = buffer;
    max = maxSize;
    pos = 0;
    firstItem = true;
}

void JKJson::clear() {
    pos = 0;
    firstItem = true;
}

// -------------------------------------------------------
// WRITER RAW
// -------------------------------------------------------
bool JKJson::writeText(const char* txt) {
    size_t len = strlen(txt);
    if (pos + len >= max) return false;
    memcpy(&buf[pos], txt, len);
    pos += len;
    return true;
}

bool JKJson::writeByte(uint8_t b) {
    if (pos >= max) return false;
    buf[pos++] = b;
    return true;
}

bool JKJson::writeBool(bool value) {
    return writeText(value ? "true" : "false");
}

void JKJson::finalize() {
    if (max == 0) return;
    buf[(pos < max) ? pos : (max - 1)] = '\0';
}

size_t JKJson::size() const { return pos; }
char* JKJson::data() { return buf; }

// -------------------------------------------------------
// WRITER OBJECT / ARRAY
// -------------------------------------------------------
bool JKJson::beginObject() { return writeByte('{'); }
bool JKJson::endObject() { return writeByte('}'); }

bool JKJson::beginKeyArray(const char* key) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"');
    writeByte(':');
    return writeByte('[');
}

bool JKJson::beginArray() { return writeByte('['); }
bool JKJson::endArray() { return writeByte(']'); }
bool JKJson::addComma() { return writeByte(','); }

// -------------------------------------------------------
// WRITER: ADD ITEMS (SRAM key versions)
// -------------------------------------------------------
bool JKJson::addItem(const char* key, bool value) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    return writeBool(value);
}

bool JKJson::addItem(const char* key, long value) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    char tmp[16];
    ltoa(value, tmp, 10);
    return writeText(tmp);
}

bool JKJson::addItem(const char* key, uint32_t value) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    char tmp[16];
    ultoa(value, tmp, 10);
    return writeText(tmp);
}

bool JKJson::addItem(const char* key, int value) {
    return addItem(key, (long)value);
}

bool JKJson::addItem(const char* key, unsigned int value) {
    return addItem(key, (long)value);
}

bool JKJson::addItem(const char* key, float value) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    char tmp[32];
    dtostrf(value, 0, 6, tmp);
    trimFloatString(tmp);
    return writeText(tmp);
}

bool JKJson::addItem(const char* key, const char* value) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    writeByte('"'); writeText(value); writeByte('"');
    return true;
}

bool JKJson::addItem(const char* key, const String& value) {
    return addItem(key, value.c_str());
}

bool JKJson::addNull(const char* key) {
    if (!firstItem) writeByte(',');
    firstItem = false;
    writeByte('"'); writeText(key); writeByte('"'); writeByte(':');
    return writeText("null");
}

// -------------------------------------------------------
// ADDON: Flash String (F()) support – writer
// -------------------------------------------------------
bool JKJson::addItem(const __FlashStringHelper* key, bool value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, long value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, int value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, unsigned int value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, float value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, const char* value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addItem(const __FlashStringHelper* key, const String& value) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addItem(tmp, value);
}
bool JKJson::addNull(const __FlashStringHelper* key) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return addNull(tmp);
}
bool JKJson::beginKeyArray(const __FlashStringHelper* key) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return beginKeyArray(tmp);
}

// -------------------------------------------------------
// JSON Stream capture
// -------------------------------------------------------
bool JKJson::captureJsonStream(char c, char* buf, size_t maxLen) {
    static bool jsonStarted = false;
    static bool inString = false;
    static bool escape = false;
    static int  depth = 0;
    static size_t pos = 0;

    if (!jsonStarted) {
        if (c == '{') {
            jsonStarted = true;
            depth = 1;
            pos = 0;
            buf[pos++] = c;
        }
        return false;
    }

    if (inString) {
        if (escape) escape = false;
        else if (c == '\\') escape = true;
        else if (c == '"') inString = false;
    }
    else {
        if (c == '"') inString = true;
        else if (c == '{') depth++;
        else if (c == '}') depth--;
    }

    if (pos < maxLen - 1) buf[pos++] = c;

    if (jsonStarted && depth == 0 && c == '}') {
        buf[pos] = '\0';
        jsonStarted = false;
        inString = false;
        escape = false;
        depth = 0;
        pos = 0;
        return true;
    }
    return false;
}

// -------------------------------------------------------
// Sanitizer
// -------------------------------------------------------
bool JKJson::extractFirstJsonObject(const char* input, char* output, size_t maxLen) {
    if (!input || !output || maxLen < 3) return false;
    const char* p = input;
    while (*p && *p != '{') p++;
    if (!*p) return false;

    bool inString = false, escape = false;
    int depth = 0;
    size_t w = 0;

    for (; *p && w + 1 < maxLen; ++p) {
        char c = *p;
        if (inString) {
            if (escape) escape = false;
            else if (c == '\\') escape = true;
            else if (c == '"') inString = false;
        }
        else {
            if (c == '"') inString = true;
            else if (c == '{') depth++;
            else if (c == '}') depth--;
        }
        output[w++] = c;
        if (!inString && depth == 0 && c == '}') break;
    }
    output[w] = '\0';
    return (depth == 0);
}
bool JKJson::extractFirstJsonObjectInPlace(char* buf) {
    if (!buf || !*buf) return false;
    char* readPtr = buf;
    char* writePtr = buf;
    while (*readPtr && *readPtr != '{') readPtr++;
    if (!*readPtr) return false;

    bool inString = false, escape = false;
    int depth = 0;

    while (*readPtr) {
        char c = *readPtr++;
        if (inString) {
            if (escape) escape = false;
            else if (c == '\\') escape = true;
            else if (c == '"') inString = false;
        }
        else {
            if (c == '"') inString = true;
            else if (c == '{') depth++;
            else if (c == '}') depth--;
        }
        *writePtr++ = c;
        if (!inString && depth == 0 && c == '}') break;
    }
    *writePtr = '\0';
    return (depth == 0);
}

// -------------------------------------------------------
// READER HELPERS
// -------------------------------------------------------
const char* JKJson::skipSpaces(const char* p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

const char* JKJson::findValueForKey(const char* json, const char* key) {
    const char* p = json;
    size_t keyLen = strlen(key);
    while (*p) {
        while (*p && *p != '"') p++;
        if (!*p) break;
        const char* start = p + 1;
        const char* end = strchr(start, '"');
        if (!end) return NULL;
        size_t len = end - start;
        if (len == keyLen && strncmp(start, key, keyLen) == 0) {
            p = end + 1;
            p = skipSpaces(p);
            if (*p != ':') { p++; continue; }
            p++;
            return skipSpaces(p);
        }
        p = end + 1;
    }
    return NULL;
}

// -------------------------------------------------------
// READER CORE
// -------------------------------------------------------
bool JKJson::getBool(const char* json, const char* key, bool& out) {
    const char* v = findValueForKey(json, key);
    if (!v) return false;
    if (strncmp(v, "true", 4) == 0) { out = true;  return true; }
    if (strncmp(v, "false", 5) == 0) { out = false; return true; }
    return false;
}

bool JKJson::getLong(const char* json, const char* key, long& out) {
    const char* v = findValueForKey(json, key);
    if (!v) return false;
    char* end;
    out = strtol(v, &end, 10);
    return end != v;
}

bool JKJson::getUlong(const char* json, const char* key, unsigned long& out) {
    const char* v = findValueForKey(json, key);
    if (!v) return false;
    char* end;
    out = strtoul(v, &end, 10);
    return end != v;
}

bool JKJson::getUint16(const char* json, const char* key, uint16_t& out) {
    unsigned long tmp = 0;
    if (!getUlong(json, key, tmp)) return false;
    out = (uint16_t)tmp;
    return true;
}

bool JKJson::getByte(const char* json, const char* key, uint8_t& out) {
    unsigned long tmp = 0;
    if (!getUlong(json, key, tmp)) return false;
    out = (uint8_t)tmp;
    return true;
}

bool JKJson::getFloat(const char* json, const char* key, float& out) {
    const char* v = findValueForKey(json, key);
    if (!v) return false;
    char* end;
    out = (float)strtod(v, &end);
    return end != v;
}

bool JKJson::getString(const char* json, const char* key, char* out, size_t maxLen) {
    const char* v = findValueForKey(json, key);
    if (!v || *v != '"') return false;
    v++;
    size_t i = 0;
    while (*v && *v != '"' && i + 1 < maxLen) out[i++] = *v++;
    out[i] = '\0';
    return *v == '"';
}

// -------------------------------------------------------
// READER: Arrays of objects
// -------------------------------------------------------
bool JKJson::getBoolInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, bool& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getBool(elem, fieldKey, out);
}
bool JKJson::getLongInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, long& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getLong(elem, fieldKey, out);
}
bool JKJson::getUlongInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, unsigned long& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getUlong(elem, fieldKey, out);
}
bool JKJson::getUint16InArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, uint16_t& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getUint16(elem, fieldKey, out);
}
bool JKJson::getByteInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, uint8_t& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getByte(elem, fieldKey, out);
}
bool JKJson::getFloatInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, float& out) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getFloat(elem, fieldKey, out);
}
bool JKJson::getStringInArray(const char* json, const char* arrayKey, size_t index, const char* fieldKey, char* out, size_t maxLen) {
    const char* elem = findArrayElement(json, arrayKey, index);
    if (!elem)return false;
    return getString(elem, fieldKey, out, maxLen);
}

// -------------------------------------------------------
// Flash-key Reader Overloads (top-level)
// -------------------------------------------------------
bool JKJson::getBool(const char* json, const __FlashStringHelper* key, bool& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getBool(json, tmp, out);
}
bool JKJson::getLong(const char* json, const __FlashStringHelper* key, long& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getLong(json, tmp, out);
}
bool JKJson::getUlong(const char* json, const __FlashStringHelper* key, unsigned long& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getUlong(json, tmp, out);
}
bool JKJson::getUint16(const char* json, const __FlashStringHelper* key, uint16_t& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getUint16(json, tmp, out);
}
bool JKJson::getByte(const char* json, const __FlashStringHelper* key, uint8_t& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getByte(json, tmp, out);
}
bool JKJson::getFloat(const char* json, const __FlashStringHelper* key, float& out) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getFloat(json, tmp, out);
}
bool JKJson::getString(const char* json, const __FlashStringHelper* key, char* out, size_t maxLen) {
    char tmp[64]; copyFlashString(key, tmp, sizeof(tmp)); return getString(json, tmp, out, maxLen);
}

// -------------------------------------------------------
// Flash-key Reader Overloads (arrays)  ? ADDED
// -------------------------------------------------------
#define JKJ_COPY_FSTR(_src_, _dst_) do { copyFlashString((_src_), (_dst_), sizeof(_dst_)); } while(0)

// bool
bool JKJson::getBoolInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, bool& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getBoolInArray(json, a, index, fieldKey, out);
}
bool JKJson::getBoolInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, bool& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getBoolInArray(json, arrayKey, index, f, out);
}
bool JKJson::getBoolInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, bool& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getBoolInArray(json, a, index, f, out);
}

// long
bool JKJson::getLongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, long& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getLongInArray(json, a, index, fieldKey, out);
}
bool JKJson::getLongInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, long& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getLongInArray(json, arrayKey, index, f, out);
}
bool JKJson::getLongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, long& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getLongInArray(json, a, index, f, out);
}

// unsigned long
bool JKJson::getUlongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, unsigned long& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getUlongInArray(json, a, index, fieldKey, out);
}
bool JKJson::getUlongInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, unsigned long& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getUlongInArray(json, arrayKey, index, f, out);
}
bool JKJson::getUlongInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, unsigned long& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getUlongInArray(json, a, index, f, out);
}

// uint16_t
bool JKJson::getUint16InArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, uint16_t& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getUint16InArray(json, a, index, fieldKey, out);
}
bool JKJson::getUint16InArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint16_t& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getUint16InArray(json, arrayKey, index, f, out);
}
bool JKJson::getUint16InArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint16_t& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getUint16InArray(json, a, index, f, out);
}

// uint8_t
bool JKJson::getByteInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, uint8_t& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getByteInArray(json, a, index, fieldKey, out);
}
bool JKJson::getByteInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint8_t& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getByteInArray(json, arrayKey, index, f, out);
}
bool JKJson::getByteInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, uint8_t& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getByteInArray(json, a, index, f, out);
}

// float
bool JKJson::getFloatInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, float& out) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getFloatInArray(json, a, index, fieldKey, out);
}
bool JKJson::getFloatInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, float& out) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getFloatInArray(json, arrayKey, index, f, out);
}
bool JKJson::getFloatInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, float& out) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getFloatInArray(json, a, index, f, out);
}

// string
bool JKJson::getStringInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const char* fieldKey, char* out, size_t maxLen) {
    char a[64]; JKJ_COPY_FSTR(arrayKey, a); return getStringInArray(json, a, index, fieldKey, out, maxLen);
}
bool JKJson::getStringInArray(const char* json, const char* arrayKey, size_t index, const __FlashStringHelper* fieldKey, char* out, size_t maxLen) {
    char f[64]; JKJ_COPY_FSTR(fieldKey, f); return getStringInArray(json, arrayKey, index, f, out, maxLen);
}
bool JKJson::getStringInArray(const char* json, const __FlashStringHelper* arrayKey, size_t index, const __FlashStringHelper* fieldKey, char* out, size_t maxLen) {
    char a[64], f[64]; JKJ_COPY_FSTR(arrayKey, a); JKJ_COPY_FSTR(fieldKey, f); return getStringInArray(json, a, index, f, out, maxLen);
}

#undef JKJ_COPY_FSTR

// -------------------------------------------------------
// Pretty Printer
// -------------------------------------------------------
void JKJson::printPrettyJson(const char* json, Print& out, uint8_t indentSpaces) {
    if (!json) return;
    const char* p = json;
    bool inString = false, escape = false;
    int depth = 0;
    uint8_t step = (indentSpaces == 0) ? 0 : indentSpaces;

    auto printIndent = [&](int d) {
        for (int i = 0; i < d * step; ++i) out.print(' ');
    };

    auto nextNonSpace = [&](const char* q) -> char {
        while (*q && (*q == ' ' || *q == '\t' || *q == '\n' || *q == '\r')) ++q;
        return *q;
    };

    while (*p) {
        char c = *p++;
        if (inString) {
            out.print(c);
            if (escape) escape = false;
            else if (c == '\\') escape = true;
            else if (c == '"') inString = false;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        switch (c) {
        case '"': inString = true; out.print(c); break;
        case '{': case '[': {
            out.print(c);
            char nn = nextNonSpace(p);
            if (!((c == '{' && nn == '}') || (c == '[' && nn == ']'))) {
                depth++; out.print('\n'); printIndent(depth);
            }
            break;
        }
        case '}': case ']':
            depth = (depth > 0) ? depth - 1 : 0;
            out.print('\n'); printIndent(depth); out.print(c);
            break;
        case ',': out.print(','); out.print('\n'); printIndent(depth); break;
        case ':': out.print(':'); out.print(' '); break;
        default: out.print(c); break;
        }
    }
    

}

// -------------------------------------------------------
// Helper: skipValue()
// -------------------------------------------------------
const char* JKJson::skipValue(const char* p) {
    p = skipSpaces(p);

    if (*p == '{') {
        int depth = 0;
        do {
            if (*p == '{') depth++;
            else if (*p == '}') depth--;
            p++;
        } while (*p && depth > 0);
        return p;
    }
    else if (*p == '[') {
        int depth = 0;
        do {
            if (*p == '[') depth++;
            else if (*p == ']') depth--;
            p++;
        } while (*p && depth > 0);
        return p;
    }
    else if (*p == '"') {
        p++;
        while (*p && *p != '"') p++;
        if (*p == '"') p++;
        return p;
    }
    else {
        while (*p && *p != ',' && *p != ']' && *p != '}' && !isspace((unsigned char)*p))
            p++;
        return p;
    }
}

// -------------------------------------------------------
// Helper: findArrayElement()
// -------------------------------------------------------
const char* JKJson::findArrayElement(const char* json,
    const char* arrayKey,
    size_t index)
{
    const char* v = findValueForKey(json, arrayKey);
    if (!v) return NULL;

    v = skipSpaces(v);
    if (*v != '[') return NULL;

    const char* p = v + 1;
    size_t count = 0;

    while (*p) {
        p = skipSpaces(p);
        if (*p == ']') break;

        const char* elemStart = p;

        if (count == index)
            return elemStart;

        const char* next = skipValue(elemStart);
        p = skipSpaces(next);

        if (*p == ',') { p++; count++; continue; }
        if (*p == ']') break;
        return NULL;
    }
    return NULL;
}
